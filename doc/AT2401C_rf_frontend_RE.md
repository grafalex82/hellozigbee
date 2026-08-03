# QBKG11LM / QBKG12LM — AT2401C RF front-end (PA/LNA) reverse-engineering

Reverse-engineering of the **2.4 GHz RF front-end module (FEM)** on the Aqara
**QBKG11LM / QBKG12LM** with-neutral wall switches (board `LM15-LNS-PA-A-T0`, NXP
**JN5169** MCU), and the one-line firmware change that enables it in the
**hellozigbee** firmware.

This document covers the chip, its wiring to the JN5169, how to switch it on, and
the measured effect.

## Summary — the problem this solves

On stock hellozigbee the switch **transmits far weaker than the original Aqara firmware**:
neighbouring routers hear it poorly (the coordinator often can't hear it directly at all),
while the device itself hears everyone fine. That good-RX / deaf-TX **asymmetry** is the
tell-tale of a transmit-path power deficit, and it survived every *core-radio* power tweak
(`eAppApiPlmeSet` max PHY power, `vAppApiSetComplianceLimits`) — because the deficit isn't in
the core radio.

**Root cause:** the board carries an external **AT2401C** front-end module (PA + LNA + T/R
switch — the "PA" in `LM15-LNS-`**`PA`**`-A-T0`). Its power amplifier only engages when its
`TXEN` pin is driven high. hellozigbee never drives it, so the device transmits on the **bare
JN5169 radio (~0 dBm)**, whereas stock Aqara firmware drives the module's **+22 dBm PA**.

**Fix:** one call at radio init — `vAHI_HighPowerModuleEnable(TRUE, TRUE)` — which makes the
radio auto-drive its RFTX/RFRX control outputs (DIO3/DIO2), wired to the module's TXEN/RXEN.

---

## 1. The chip — AT2401C

- 2.4 GHz FEM: **PA + LNA + T/R switch**, QFN-16 (3×3 mm), VDD **2.0–3.6 V**, **+22 dBm**
  saturated output, ~90 mA at +20 dBm TX. (Skyworks **RFX2401C** is a pin/function twin.)
- Sample marking: `2401C  4447.1  UH1826` (`2401C` = part; the rest is lot/date).

### Pinout (top view, pin-1 dot at top-left, counter-clockwise)

```
                     T O P   edge  (read L→R: 16,15,14,13)
                    16     15     14     13
                    VDD    GND    VDD    DNC
              ●      │      │      │      │
             ┌───────┴──────┴──────┴──────┴───────┐
    1  GND ──┤                                    ├── 12  GND
    2  GND ──┤                                    ├── 11  GND
    3  GND ──┤             AT2401C                ├── 10  ANT
             │            (TOP VIEW)              │
    4  TXRX ─┤        EP (centre pad) = GND       ├──  9  GND
             └───────┬──────┬──────┬──────┬───────┘
                     │      │      │      │
                     5      6      7      8
                    TXEN   RXEN   GND    GND
                     B O T T O M  edge  (read L→R: 5,6,7,8)
```

| Pin | Name | Function |
|---|---|---|
| 4 | **TXRX** | RF signal to/from the transceiver (JN5169 side); DC-shorted to GND |
| 5 | **TXEN** | TX enable — CMOS control input |
| 6 | **RXEN** | RX enable — CMOS control input |
| 10 | **ANT** | PA output / LNA input (antenna side); DC-shorted to GND |
| 16 | **VDD** | supply input (pin 14 is a second VDD, internally tied to 16) |
| 13 | DNC | do not connect |
| 1,2,3,7,8,9,11,12,15 + EP | GND | ground |

### Control logic

| TXEN | RXEN | State |
|---|---|---|
| 1 | X | **TX path active (PA on)** |
| 0 | 1 | RX path active (LNA on) |
| 0 | 0 | shutdown / sleep |

Both controls are **active-high** (logic "1" ≥ 1.2 V). T/R and shutdown switching ~800 ns.

---

## 2. Reverse-engineered wiring — AT2401C ↔ JN5169

Traced by continuity on a dead board. The two control lines each run through a **1 kΩ series
resistor** (the datasheet's recommended control-drive) to the JN5169's dedicated radio-control
outputs; the RF port and supply route as expected.

```
   AT2401C                                   JN5169 (QFN40)
   ┌──────────────┐
   │ 5  TXEN ───────[ 1kΩ ]──────────────► 19  DIO3 / RFTX    (radio TX-enable output)
   │ 6  RXEN ───────[ 1kΩ ]──────────────► 18  DIO2 / RFRX    (radio RX-enable output)
   │ 4  TXRX ───────[ match net ]────────► 13  RF_IO          (single-ended 50 Ω)
   │ 10 ANT  ────────────────────────────► PCB antenna
   │ 16 VDD  ────────────────────────────► 30  VDDD           (3.3 V digital supply rail)
   └──────────────┘
```

| AT2401C | via | JN5169 pin | JN5169 function |
|---|---|---|---|
| TXEN (5) | 1 kΩ | **19** | **DIO3 / RFTX** — radio transmitter control output |
| RXEN (6) | 1 kΩ | **18** | **DIO2 / RFRX** — radio receiver control output |
| TXRX (4) | RF match | **13** | **RF_IO** — RF antenna port |
| ANT (10) | — | — | PCB antenna |
| VDD (16) | — | **30** | **VDDD** — digital supply input (the 2.2–3.6 V rail, ~3.3 V) |

Notes:
- `VDDD` (pin 30) is the JN5169's **supply input**, not one of the internal ~1.8 V regulator
  (`VB_*`) pins — so the FEM is fed from the same rail as the MCU, within its 2.0–3.6 V window.
- The `TXRX ↔ RF_IO` link is through the RF matching network (series DC-block + π-match), so it
  is not a DC short — trace it along the RF track, not with a continuity beep.

---

## 3. How the JN5169 drives it

`DIO2/RFRX` (pin 18) and `DIO3/RFTX` (pin 19) are the JN516x radio's **dedicated RX/TX control
outputs**. When high-power-module control is enabled, the MAC/PHY **asserts them automatically**:
RFTX high during transmit, RFRX high during receive. Their **active-high** polarity matches the
AT2401C's TXEN/RXEN directly (no inverter needed), so:

- **transmit** → RFTX high → TXEN high → **PA engaged**
- **receive**  → RFRX high → RXEN high → **LNA engaged**

DIO2/DIO3 are otherwise unused on this board (relay/buttons/LEDs live elsewhere),
so nothing else contends for them.

---

## 4. The firmware enable

In `ZigbeeDevice::ZigbeeDevice()`, **before** `ZPS_eAplAfInit()` brings up the MAC/PHY:

```c
// AppHardwareApi.h
vAHI_HighPowerModuleEnable(TRUE, TRUE);   // TX and RX control must be enabled together
```

- Requires `#include "AppHardwareApi.h"`.
- This is a **one-time init** — it configures the radio to auto-toggle DIO2/DIO3 in hardware on
  every RX/TX; it does not need re-applying per packet. (If a stack radio re-init on rejoin ever
  dropped it, re-assert it in the rejoin path — but a non-sleeping mains router holds it.)
- Keep the existing `vAppApiSetComplianceLimits(...)` ETSI ceiling; with a real PA in circuit it
  finally does something useful (it bounds the amplified output).

**Channel-26 caveat:** NXP documents that `vAHI_HighPowerModuleEnable` **breaches emission limits
on channel 26** at full PA (band-edge spectral mask). If the network runs on channel 26, use
`vAppApiSetHighPowerMode()` (802.15.4 stack API) instead, which reduces power there. Off channel
26 this does not apply.

### What did NOT work (and why)

- `eAppApiPlmeSet(PHY_PIB_ATTR_TX_POWER, 10)` — no measurable effect (ineffective in the ZPS
  runtime context, and irrelevant while the external PA is off).
- `vAppApiSetComplianceLimits(...)` alone — no effect: it's a *ceiling* the bare core already sits
  below, and the missing gain was the un-powered PA, not the core level.

The transmit deficit was the external PA the whole time, not the JN5169's own output power.

---

## 5. Validation (measured)

Zigbee networkmap **inbound LQI** (how neighbouring routers hear the device). Same switch model,
**same wall socket**, three firmware states. Neighbour names anonymised (R1…R8 + Coordinator).

| neighbour hears it | PA off (before) | **PA on (this fix)** | **stock Aqara FW** |
|---|---|---|---|
| Coordinator (direct) | — (unreachable) | 104 | 103 |
| R1 | 58 | 186 | 185 |
| R2 | 38 | 177 | 175 |
| R3 | 0 | 147 | 152 |
| R4 | 0 | 126 | 123 |
| R5 | — | 100 | 88 |
| R6 | — | 85 | 86 |
| R7 | — | 85 | 80 |
| R8 | — | 75 | 63 |
| **median / max** | **0 / 58** | **104 / 186** | **103 / 185** |

- **PA off:** the device was effectively invisible to the coordinator (routed via a neighbour),
  heard by only a couple of routers, the rest at LQI 0.
- **PA on:** the coordinator hears it **directly**, all routers hear it strongly, and the RX/TX
  asymmetry disappears (it hears its strongest neighbour at ~194, that neighbour now hears it at
  ~186 — reciprocal).
- **Versus stock Aqara firmware in the identical socket:** matched to within a couple of LQI
  points on **every** link. The fix reproduces the factory operating point exactly — it enables
  the PA the same way stock does, **without over-driving** (nothing is pushed to LQI 255; peaks
  sit at/under the stock device's).

---

## References

- **AT2401C** datasheet — Hangzhou Zhongkewei (DS-AT2401C).
- **RFX2401C** datasheet — Skyworks (pin/function twin).
- **JN5169** datasheet (NXP) — pin table: DIO2/RFRX pin 18, DIO3/RFTX pin 19, RF_IO pin 13,
  VDDD pin 30.
- NXP **JN-UG-3087** (JN516x Integrated Peripherals API) — `vAHI_HighPowerModuleEnable`.
