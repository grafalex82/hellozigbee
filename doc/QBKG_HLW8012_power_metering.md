# QBKG11LM — HLW8012 power metering driver

Power/energy metering support for the Aqara **QBKG11LM** with-neutral wall switch
(board `LM15-LNS-PA-A-T0`, NXP **JN5169**) in the **hellozigbee** firmware, driving the
on-board **HLW8012** energy-metering IC. The device exposes **active power (W)**,
**RMS voltage (V)**, **RMS current (A)** and **cumulative energy (kWh)** over standard
ZCL clusters, with attribute reporting, so zigbee2mqtt renders them natively.

The hardware reverse-engineering (signal tracing, board constants, safety notes) is
documented separately in `QBKG_HLW8012_power_metering_RE.md`. This document covers the
driver design, the calibration method, and the coordinator-side integration, including
several non-obvious SDK and zigbee2mqtt pitfalls.

Advantage over stock Aqara firmware: stock only refreshes the power value when the relay
switches (zigbee2mqtt#1744); this driver measures and reports continuously.

## Acquisition — hardware pulse counters, hardware timebase

The HLW8012 outputs frequencies, not registers:

| HLW8012 pin | meaning | JN5169 | peripheral |
|---|---|---|---|
| CF | active power (freq ∝ W); each pulse = fixed energy | DIO8 | Pulse Counter 1 |
| CF1 | RMS current *or* voltage, selected by SEL | DIO1 | Pulse Counter 0 |
| SEL | CF1 mode select (inverted by a 2N7002 on this board) | DIO9 | GPIO out |

Lucky break: PC0/PC1 default to exactly these DIOs — no remapping. Both counters run
**free-running** (rising edge, debounce off, not combined, no interrupts) and are sampled
once per second by `EnergyMeterTask`; uint16 wrap-around deltas absorb counter overflow
and the documented spurious +1 of `bAHI_StartPulseCounter()`.

**Do not assume the sampling window is 1 s.** The ZTIMER callback jitters whenever the
main loop is busy (observed ±25 % under radio storms — a brushed-motor load next to the
antenna is an effective jammer), and since both channels scale by the same wrong factor
the corrupted readings look internally consistent. Timer 0 free-runs as a timebase
(16 MHz / 2^14 = 976.5625 Hz; DIO takeover disabled — its pins overlap CF/SEL/button)
and every window computes `freq = pulses × 78125 / (ticks × 8)`, immune to callback
jitter. Frequencies are kept in 0.1 Hz units ("dHz").

**SEL multiplexing:** the polarity had to be resolved empirically — with DIO9 driven low
CF1 outputs *voltage* pulses (the 2N7002 inverts on the way to the chip). SEL alternates
every 5 windows; the window straddling a toggle is discarded (mode change + HLW8012
settle) and each mode keeps its last valid frequency, so voltage stays fresh while
current is measured.

## Calibration — integrate counts, then anchor to references

Datasheet-nominal multipliers (from V_REF 2.43 V, F_OSC 3.579 MHz, divider ratio 1881,
shunt 2 mΩ — the xoseperez/hlw8012 formulas) were off by several percent on the tested
unit: component tolerances are real (the shunt measured ~9 % under its marking).

Method that works over the air:

1. The firmware exposes **cumulative CF/CF1 pulse counts** in two manufacturer-specific
   uint32 attributes of the Electrical Measurement cluster (0xFF00/0xFF01, manufacturer
   code 0x1037). Reading counts twice a few minutes apart under a steady load gives the
   average frequency immune to any per-window noise and radio dropouts.
2. **Power:** anchor CF frequency to a plug-through power meter reading taken in the same
   interval (`K_P = W_meter / f_CF`). One tested unit: 4.5004 W/Hz vs 4.138 nominal.
3. **Voltage:** anchor CF1 (voltage mode) to a multimeter reading at the load terminals,
   taken under load so the sag is included.
4. **Current:** no reference instrument needed — all three HLW8012 channels share V_REF
   and the shunt, so the gains obey `K_P = K_V × K_I` and `K_I = K_P / K_V` follows
   algebraically (power-factor independent). Cross-check: the implied PF for a
   heater+motor load should land just below 1 (0.966 measured).

The constants live in the board section of `zcl_options.h` (`METERING_W_PER_DHZ_E5`
etc., scaled by 1e5 for integer math). They are **specimen-specific** — recalibrate per
unit, or expect a few percent error from copying another unit's values.

## ZCL exposure

Both clusters sit on the basic (common) endpoint, next to DeviceTemperature:

- **Electrical Measurement (0x0B04):** `activePower` (W, multiplier 1 / divisor 1),
  `rmsVoltage` (0.1 V, divisor 10), `rmsCurrent` (mA, divisor 1000), plus the cumulative
  count attributes above.
- **Simple Metering (0x0702):** `currentSummationDelivered` in Wh (multiplier 1 /
  divisor 1000 → kWh), derived from the lifetime CF count — each pulse is a fixed energy
  quantum (`K_P` joules), so the count *is* the energy register.

The energy register is persisted via PDM with a wear-aware policy: saved every
~0.1 kWh of accumulation, or daily if any unsaved energy exists. Power-cut loss is
bounded by ~0.1 kWh; EEPROM endurance stays comfortable for decades.

### SDK pitfall #1 — two independent "reportable" flags

Making these attributes reportable requires **both**:

- `E_ZCL_AF_RP` in the attribute definition table — the reporting engine and
  `eZCL_ReportAttribute()` silently skip attributes without it. The SDK tables ship
  read-only, so patched copies of `ElectricalMeasurement.c` / `SimpleMetering.c` are
  vendored into `src/` (the delta is only this flag).
- `E_ZCL_ACF_RP` in the per-instance attribute *control bits* — the configure-reporting
  command handler checks this one and answers `UNREPORTABLE_ATTRIBUTE` otherwise. Set at
  runtime with `eZCL_SetReportableFlag()` after cluster registration.

### SDK pitfall #2 — the reporting engine samples structs directly

The engine reads the cluster storage structs without invoking any application hook, so
updating attributes lazily in the read handler would report stale values. The meter task
pushes fresh values into the structs every sampling window instead.

(Also: the SDK gates the Simple Metering source on `CLD_SIMPLE_METERING` but its header
structs on `CLD_SM`/`SM_SERVER` — define all three.)

## zigbee2mqtt integration

The external converter adds `m.electricityMeter()` to the QBKG11LM definition — power,
voltage, current and energy render as native entities and reporting is configured by the
standard z2m configure flow.

### zigbee-herdsman pitfall — OTA never applies with an unsynced clock

zigbee-herdsman (z2m ≥ 2.x) sends the OTA `upgradeEndResponse` with
`currentTime = <real ZCL-UTC>` and `upgradeTime = currentTime + 1`. The NXP OTA client
only schedules *relative* to receipt when `currentTime == 0`; otherwise it arms an
absolute compare against its **own UTC clock**, which on a device with no Time cluster
counts from 0 at boot — the upgrade gets scheduled ~26 years out and the device never
reboots (transfer and CRC succeed; the client then sits in COUNT_DOWN state ignoring all
further OTA frames). The code even carries a TODO suspecting this
(`controller/model/device.js`, "could this tiny offset be a problem for some stacks?").

Workarounds until fixed upstream: patch `device.js` to send `currentTime: 0,
upgradeTime: 1`, or simply **power-cycle the stuck device** — this firmware's
`restoreOTAAttributes()` clamps any pending OTA schedule to "retry in 10 s" on boot,
which applies the already-downloaded image.

## Known limitations

- The current channel is derived (see calibration) and carries a ~30 mA noise floor at
  zero load (HLW8012 low-end SNR; the stock firmware's low-load readings suffer the same
  way). Could be zero-clamped when the relay is off.
- QBKG12LM very likely carries the same metering circuit, but its HLW8012→DIO wiring has
  not been verified, so the driver is enabled for QBKG11LM only
  (`SUPPORTS_POWER_METERING` in `zcl_options.h`).
- Low loads produce sub-Hz CF (0.5 W ≈ 0.12 Hz): 1 s windows legitimately read 0 W
  between pulses. The energy register integrates correctly regardless.
