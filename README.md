# SandFan

My project for Half-Life by Hack Club!

![Render or photo of the project](docs/hero.png)

Meant for climbers, a two-in-one portable device:
 - Variable speed enclosed fan for drying out wet hands on one side.
 - Variable speed skin-sander for grinding down overgrown calluses.

## How it works
A Seeed XIAO RP2040 is the controller. It sits on a 1S LiPo battery through a slide switch.

- **Power:** The battery feeds the motors directly (about 3.0 to 4.2 V). An AP2112K low-dropout regulator makes 3.3 V for the logic. The XIAO's own USB-C port charges the battery through an MCP73831 charger (about 200 mA), so there is no second USB connector. The battery can be charged while the switch is off.
- **Motor drive:** Each motor has its own AO3400A logic-level N-MOSFET on the low side, driven by a 20 kHz PWM pin from the RP2040. A 100 ohm gate resistor and a 100k pulldown keep the motors off while the MCU boots. An SS14 flyback diode and a 100 nF cap sit across each motor.
- **Stall protection:** The sander's MOSFET source goes to ground through a 0.1 ohm shunt. The voltage across it is filtered (1k and 100 nF) and read by an ADC pin. If the current stays above the limit for 150 ms, the firmware stops the sander and holds it off for 2 s, blinking its LED.
- **Buttons:** Five tactile buttons: fan up, fan down, sander up, sander down, and stop (everything off). Speed has 5 steps per motor. Three LEDs show fan, sander and charge state.
- **Low battery:** A 1:2 divider lets the MCU read the battery voltage. Motors are cut off below 3.3 V.

![Block diagram](docs/block-diagram.png)

## Schematic and board
![Schematic](docs/schematic.png)
![Board layout](docs/layout.png)
![3D render](docs/render.png)

The board is 78 x 46 mm, two layers, with a ground pour on both sides. The XIAO's USB-C port sits at the board edge. ERC and DRC both pass with 0 violations in KiCad 10.

## Wiring
Everything on the board is soldered. The only hand-wired parts are the three things that plug in with JST-PH 2-pin connectors:

| Connector | Pin 1 | Pin 2 |
|-----------|-------|-------|
| J1 Battery | + | - |
| J2 Fan motor | VSYS (battery +) | motor return (switched by MOSFET) |
| J3 Sander motor | VSYS (battery +) | motor return (switched by MOSFET) |

LiPo packs are not standardized. Check your pack's polarity before plugging it in. Use a protected cell. Motor polarity only sets the spin direction.

## Bill of materials
| Part | Qty | Price | Link |
|------|-----|-------|------|
| Seeed XIAO RP2040 (plus 2x 1x7 2.54 mm headers) | 1 | TBD | TBD |
| MCP73831T-2ACI/OT charger | 1 | TBD | TBD |
| AP2112K-3.3 regulator | 1 | TBD | TBD |
| AO3400A N-MOSFET | 2 | TBD | TBD |
| SS14 Schottky diode (SMA) | 2 | TBD | TBD |
| 0.1 ohm 1% 0.5 W resistor (1206) | 1 | TBD | TBD |
| B3U-1000P tactile switch | 5 | TBD | TBD |
| OS102011MS2QN1 slide switch | 1 | TBD | TBD |
| JST-PH B2B-PH-K-S connector | 3 | TBD | TBD |
| LEDs 0603 (red, green, yellow) | 3 | TBD | TBD |
| Resistors 0603 (100, 470, 1k, 5.1k, 100k) | 12 | TBD | TBD |
| Capacitors (100 nF, 4.7 uF, 10 uF, 22 uF) | 9 | TBD | TBD |
| 1S LiPo battery, protected | 1 | TBD | TBD |
| Fan motor and sander motor | 2 | TBD | TBD |

Prices and links get filled in when parts are ordered. The full line-by-line list with designators is in [bom.csv](bom.csv).

## Repo layout
- hardware/ : KiCad files, Gerbers zip
- firmware/ : source code
- docs/ : images
- bom.csv

## Status and next steps
**Done:** schematic, PCB (fully routed), Gerbers and drill files (`hardware/sandfan_gerbers.zip`), BOM, firmware first draft.

**Checked:** ERC and DRC pass in KiCad 10.0.6. The schematic netlist was cross-checked against the intended connections, and the board has 0 unconnected items.

**Not yet tested:**
- The firmware has not been compiled or run. It targets the arduino-pico core on a XIAO RP2040 and its pin map matches the schematic.
- No board has been built. The XIAO footprint (2x7, 2.54 mm pitch, 15.24 mm between rows) is from memory of the module, so measure the real module against it before ordering.
- The motors are not chosen yet. The stall current limit (1.5 A) and the 0.1 ohm shunt need to be sized to the real sander motor, and the shunt should be rated for the stall current.
- When USB is plugged in, the XIAO's own 3.3 V regulator and the AP2112K both drive the 3V3 rail. This is expected to be fine, but it is not verified on hardware.

**Build week:** order the board and parts, build it, compile and flash the firmware, tune the stall threshold with the real motor, then pick an enclosure and fit the fan and sander.
