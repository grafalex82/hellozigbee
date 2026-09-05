# QBKG11LM / QBKG12LM — HLW8012 power‑metering reverse engineering

Reverse‑engineering of the power‑measurement path on the Aqara **QBKG11LM (1‑gang)** /
**QBKG12LM (2‑gang)** with‑neutral wall switches, for adding energy metering to the
**hellozigbee** firmware (grafalex82/hellozigbee) running on the on‑board **NXP JN5169**.

- **CPU board:** `LM15-LNS-PA-A-T0` (LUMI) — JN5169 MCU, buttons, LEDs, antenna + PA.
- **Metering chip:** **HLW8012** single‑phase energy‑metering IC, on the **power board**.
- grafalex identified the HLW8012 in doc part26 but did **not** reverse‑engineer its wiring —
  this document fills that gap.
- Goal: expose **active power (W)**, **RMS current (A)** / **RMS voltage (V)**, and
  **cumulative energy (kWh)** over Zigbee.

> ⚠️ **NON‑ISOLATED BOARD.** The common/GND rail is tied to mains **Live** (confirmed: HLW8012
> GND pin 5, the 2N7002 source, and TP11 are all the same net = L). The *entire* logic domain —
> JN5169 included — floats at mains potential. Continuity tracing on a **dead** board is safe;
> **never probe it powered.** Do calibration through the **isolated Zigbee link** (read values in
> z2m against a reference meter on the load), or use an isolation transformer + differential probe.

---

## 1. HLW8012 (SOP‑8) — chip pinout

```
                       ┌────∪────┐
   VDD  ── 1  ●────────┤         ├────────  8 ── SEL   (mode select, input; internal pull-down)
   V1P  ── 2  ─────────┤         ├────────  7 ── CF1   (current/voltage pulse out)
   V1N  ── 3  ─────────┤ HLW8012 ├────────  6 ── CF    (power/energy pulse out)
   V2P  ── 4  ─────────┤         ├────────  5 ── GND
                       └─────────┘
```
| Pin | Name | Function |
|---|---|---|
| 1 | VDD | supply — **5 V** on this board |
| 2 | V1P | current sense + (across shunt) — mains side |
| 3 | V1N | current sense − (across shunt) — mains side |
| 4 | V2P | voltage sense (from divider off L) — mains side |
| 5 | GND | ground — **tied to mains L** (non‑isolated) |
| 6 | **CF** | pulse freq ∝ **active power (W)**; pulse **count = energy (kWh)** |
| 7 | **CF1** | pulse freq ∝ **RMS current** (SEL=0) or **RMS voltage** (SEL=1) |
| 8 | **SEL** | selects CF1 output type |

---

## 2. Signal chains — HLW8012 → JN5169

The chip runs at **5 V**, the JN5169 at **3.3 V**, so every MCU‑facing line is level‑shifted.

### CF (power / energy) — HLW8012 pin 6
```
CF (pin6, 5V) ─ TP8 ─ 10kΩ (103) ─┬─ TP9 (tap ≈3.0V) ─ JP2 pin 8 ─ JN5169 pin 31 = DIO8 (PC1)
                                   └─ 15kΩ (18C) ─ TP11 (GND/L)
```
- Resistive divider: 5 V × 15/(10+15) = **3.0 V** at the tap (TP9).
- Lands on **DIO8 = hardware Pulse Counter 1 (PC1)**.

### CF1 (current / voltage) — HLW8012 pin 7
```
CF1 (pin7, 5V) ─ TP6 ─ 10kΩ (103) ─┬─ TP7 (tap ≈3.0V) ─ JP1 pin 5 ─ JN5169 pin 17 = DIO1 (PC0)
                                    └─ 15kΩ (18C) ─ TP11 (GND/L)
```
- Same 10k/15k divider → 3.0 V tap (TP7).
- Lands on **DIO1 = hardware Pulse Counter 0 (PC0)**.

### SEL (mode select) — HLW8012 pin 8
```
JN5169 pin 32 = DIO9 ─ JP1 pin 7 ─ TP2 ─ 2N7002 (N-MOSFET, marked "702") ─ SEL (pin8)
                                          gate=control, drain=SEL, source=common/L
```
- Level‑shifted **up** (3.3 V MCU → 5 V SEL) through a **2N7002** — **inverting**.
- Lands on **DIO9** (plain GPIO output).
- SEL logic polarity is inverted by the FET → **determine empirically** (drive DIO9, watch whether
  CF1 reports current vs voltage).

---

## 3. Final pin map

| HLW8012 | signal | power‑board TPs | level shift | JP connector | JN5169 pin | JN5169 DIO | peripheral |
|---|---|---|---|---|---|---|---|
| pin 6 | **CF** (power/energy) | TP8 → tap **TP9** | 10k/15k divider → 3.0 V | **JP2 pin 8** | **31** | **DIO8** | **PC1** (pulse counter 1) |
| pin 7 | **CF1** (current/voltage) | TP6 → tap **TP7** | 10k/15k divider → 3.0 V | **JP1 pin 5** | **17** | **DIO1** | **PC0** (pulse counter 0) |
| pin 8 | **SEL** (mode select) | **TP2** | 2N7002, inverting up‑shift | **JP1 pin 7** | **32** | **DIO9** | GPIO out (inverted) |

**Directions (JN5169):** DIO8 = input (PC1), DIO1 = input (PC0), DIO9 = output.

---

## 3a. Analog front‑end — current shunt & voltage divider (measured)

These are the two board constants that set the pulse‑frequency → physical‑unit scaling.

### Current shunt (V1P/V1N, pins 2/3)
- **R002 = 0.002 Ω (2 mΩ)** — in series **between L and L1**; all load current flows through it.
- SMD low‑value code (`R` = decimal point). Sense voltage: 10 A → 20 mV, 16 A → 32 mV — within the HLW8012 ±43.75 mV V1 range; I²R at 10 A ≈ 0.2 W.

### Voltage divider (V2P, pin 4 = **TP16**)
```
N ──[ 22 Ω fusible ]──┬──[ 4 × 470 kΩ (4703) = 1.88 MΩ ]── V2P / TP16 (pin 4) ──[ 1 kΩ (018) ]── L (=GND)
                      └──► SMPS input / rest of circuit
        Rup (high side)                                       Rdown (low side)
```
- **Rup = 4 × 470 kΩ = 1.88 MΩ** in series with a **22 Ω fusible** on the N side → **Rup_total ≈ 1,880,022 Ω** — SMD `4703` (1%, 4‑digit code `470×10³`).
- **Rdown = 1 kΩ** — marked `018`, measured 1.00 kΩ (ohm mode, 2 kΩ range).
- **Divider ratio = (Rup_total + Rdown)/Rdown ≈ 1881** (the 22 Ω is 0.001 % of Rup — negligible).
- V2P at 230 V ≈ 230 × 1000/1,881,022 ≈ **122 mV RMS** — within the HLW8012 V2 range.

> The **~22 Ω through‑hole flameproof resistor** (bands red‑red‑black‑gold, measured 21.2 Ω) is a **shared mains inrush / fusible safety resistor**: it sits between the true **N terminal** and the board's internal post‑fuse node, from which **both the SMPS input and this voltage divider** are fed. So it **is** in series in the divider's return to N — but at 22 Ω vs 1.88 MΩ it does not affect calibration.

### Initial multipliers (xoseperez/hlw8012 formula — V_REF = 2.43, F_OSC = 3.579 MHz)
Driver setup equivalent: `setResistors(current = 0.002, v_upstream = 1.88e6, v_downstream = 1000)`.
- current_multiplier ≈ **7.2 × 10³**
- voltage_multiplier ≈ **3.3 × 10⁵**
- power_multiplier   ≈ **4.1 × 10⁶**

These are **theoretical**; shunt / V_REF / F_OSC tolerances shift them a few %. **Trim against a reference meter on a known resistive load** (per §6).

---

## 4. Connectors (board‑to‑board)

Two 8‑pin (4×2) headers, dot = pin 1, **zig‑zag** numbering (`1 2 / 3 4 / 5 6 / 7 8`).
Metering uses **JP1** and **JP2** (JP1 carries most of it).

**JP1 (partially mapped):**
| pin | net |
|---|---|
| 1 | VDD (5 V) → HLW8012 pin 1 |
| 5 | CF1 tap (TP7) → DIO1 |
| 7 | SEL (TP2, via 2N7002) → DIO9 |
| 8 | GND (= TP11, tied to L) |
| 2,3,4,6 | not mapped (VDD/GND/relay control/other) |

**JP2 (partially mapped):**
| pin | net |
|---|---|
| 8 | CF tap (TP9) → DIO8 |
| others | not mapped |

---

## 5. Power‑board test points (as used here)

| TP | net |
|---|---|
| TP2 | SEL control (2N7002 gate side) → JP1 pin 7 |
| TP6 | HLW8012 CF1 (pin 7, raw 5 V) |
| TP7 | CF1 divider **tap** (3.0 V) → JP1 pin 5 |
| TP8 | HLW8012 CF (pin 6, raw 5 V) |
| TP9 | CF divider **tap** (3.0 V) → JP2 pin 8 |
| TP11 | **GND / common — tied to mains L** (reference for both dividers, 2N7002 source, HLW8012 GND) |
| TP16 | HLW8012 **V2P** (pin 4) — voltage‑sense divider tap (Rup 1.88 MΩ ↔ Rdown 1 kΩ) |

---

## 6. Driver implementation notes (JN5169 / hellozigbee)

- **Use the hardware pulse counters** (not GPIO edge interrupts): CF → **PC1** (DIO8), CF1 → **PC0**
  (DIO1). The JN516x SDK provides pulse‑counter APIs (`vAHI_PulseCounterConfigure`, etc.). This is
  robust at high CF frequencies (high power) where interrupt‑driven counting could miss edges.
  - **Energy (kWh):** accumulate the **CF** pulse count over time.
  - **Power (W):** CF **frequency** — read PC1 over a fixed window, or use the period between pulses.
  - **Current/Voltage:** CF1 frequency (PC0), with **SEL** toggled to select which.
  - Note DIO8 also has `TIM0CK_GT` and DIO9 has `TIM0CAP` — timer capture is an alternative for
    precise period measurement if needed.
- **SEL (DIO9):** GPIO output, **inverted** through the 2N7002. Drive it, alternate current/voltage
  reads, settle time per the HLW8012 datasheet before reading CF1. Confirm polarity empirically.
- **Level shift:** CF/CF1 arrive already divided to ~3.0 V (safe for 3.3 V DIO). The divider changes
  amplitude only — **pulse frequency is unchanged**, so it does **not** affect counting/calibration.
- **Calibration (TBD):** HLW8012 output frequencies depend on the **current shunt** (V1P/V1N) and the
  **voltage divider** (V2P) on the power board. Read those component values off the board, or
  calibrate against a **known resistive load + reference meter**. Port the maths/constants from an
  existing driver:
  - `xoseperez/hlw8012` (Arduino), ESPHome `hlw8012`, or Tasmota.
- **Zigbee exposure:** add standard clusters so z2m renders it automatically:
  - `haElectricalMeasurement` (0x0B04): `activePower` (W), `rmsCurrent` (A), `rmsVoltage` (V).
  - `seMetering` (0x0702): `instantaneousDemand` (W), `currentSummationDelivered` (kWh).
  - Slots into hellozigbee's existing cluster/endpoint + attribute‑reporting infrastructure.

---

## 7. TODO

- [x] Read/measure shunt + voltage‑divider values → calibration constants. **(shunt 2 mΩ; divider Rup 1.88 MΩ / Rdown 1 kΩ, ratio 1881 — see §3a)**
- [ ] Determine SEL (DIO9) active polarity empirically (2N7002 inverts).
- [ ] Implement PC0/PC1 pulse‑counter driver + SEL toggling.
- [ ] Add `haElectricalMeasurement` / `seMetering` clusters + reporting.
- [ ] Coordinate/PR upstream (grafalex82/hellozigbee — it's an open "current and power sensor" TODO).

## References
- HLW8012 datasheet (Hiliwei Tech).
- JN5169 datasheet Rev 1.4 (NXP) — pin config Fig 3 / Table 2.
- hellozigbee doc part26 (QBKG12LM support) — identifies the HLW8012, notes it was not RE'd.
