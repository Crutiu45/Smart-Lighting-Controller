# Smart Lighting Controller

An ESP32-based automatic room lighting system using PIR motion sensing, relay-controlled lighting, manual override, and a discrete RGB LED for system state indication.

> **Simulation:** A complete Wokwi virtual circuit is included in `diagram.json`. Open it at [wokwi.com](https://wokwi.com) or via the PlatformIO + Wokwi VS Code extension.

---

## Features

- **Motion-activated lighting** — PIR sensor (GPIO33) triggers a relay (GPIO23) to turn on the load LED when occupancy is detected
- **Dual state machine** — Separate *Room* and *Lighting* state machines run concurrently for clean, deterministic control logic
- **Auto-off timeout** — Light turns off automatically after 5 seconds of no motion (OCCUPIED → IDLE → EMPTY)
- **Manual override** — Pushbutton (GPIO14, falling-edge interrupt) toggles the relay on/off, immediately pre-empting automatic control
- **Override timeout** — System returns to automatic mode 5 seconds after the last button press
- **RGB status LED** — Discrete common-cathode RGB LED (GPIOs 18/19/22) indicates system state:
  - 🔴 **Red** — Manual override active
  - 🟢 **Green** — Motion detected, relay on (auto mode)
  - 🔵 **Blue** — Idle / no motion / auto mode inactive
- **Serial debug console** — Commands over 115200 baud for hardware-free testing:

  | Key | Action |
  |-----|--------|
  | `m` | Simulate motion detected |
  | `t` | Force motion timeout |
  | `o` | Toggle override mode |
  | `s` | Print full system status |

---

## Hardware

### Components

| Component | Part | Pin(s) | Notes |
|-----------|------|--------|-------|
| Microcontroller | ESP32 DevKit C V4 | — | 240 MHz dual-core Xtensa LX6 |
| PIR motion sensor | HC-SR501 (or similar) | GPIO33 | Input, HIGH = motion detected |
| Relay module | Single-channel 5V relay | GPIO23 | Output — switches LED load |
| Push button | Momentary, normally open | GPIO14 | Internal pull-up; FALLING edge interrupt |
| RGB LED | Common-cathode discrete LED | R→GPIO19, G→GPIO18, B→GPIO22 | Current-limited via 220Ω resistors |
| Load LED | Red LED | Via relay NO contact | Simulates room lighting load |
| Resistors | 220Ω × 4 | RGB + load LED | Current limiting |
| Resistor | 10kΩ × 1 | Button | Pull-down (if not using internal pull-up) |

### Wiring Summary

```
ESP32 GPIO23  →  Relay IN
ESP32 GPIO33  →  PIR OUT         (PIR VCC → 3.3V/5V, PIR GND → GND)
ESP32 GPIO14  →  Button leg 1    (Button leg 2 → GND)
ESP32 GPIO18  →  220Ω → RGB G
ESP32 GPIO19  →  220Ω → RGB R
ESP32 GPIO22  →  220Ω → RGB B
                 RGB COM → GND

Relay COM  →  VCC
Relay NO   →  220Ω → Load LED anode
             Load LED cathode → GND
```

> **Note:** The Wokwi simulation uses `wokwi-rgb-led` (common cathode, GPIOs 18/19/22) and `wokwi-relay-module`. The firmware drives the RGB pins directly — there is no NeoPixel/WS2812 in the real or simulated circuit.

---

## State Machines

### Room State Machine

| From | Event | To | Action |
|------|-------|----|--------|
| `EMPTY` | motion_detected | `OCCUPIED` | Start occupancy timer |
| `OCCUPIED` | no motion for T_idle (5s) | `IDLE` | Start idle countdown |
| `IDLE` | motion_detected | `OCCUPIED` | Reset timers |
| `IDLE` | idle timer expired | `EMPTY` | Signal lights OFF |

### Lighting Control State Machine

| From | Event | To | Action |
|------|-------|----|--------|
| `LIGHT_OFF` | room_occupied | `LIGHT_ON_AUTO` | Relay ON |
| `LIGHT_ON_AUTO` | room_empty | `LIGHT_OFF` | Relay OFF |
| `ANY` | manual_override_on | `LIGHT_ON_MANUAL` | Force relay state |
| `LIGHT_ON_MANUAL` | override timeout (5s) | `LIGHT_ON_AUTO` or `LIGHT_OFF` | Resume auto |

---

## Project Structure

```
Smart Lighting Controller/
├── src/
│   └── main.cpp          — Firmware: state machines, ISR, serial debug
├── platformio.ini         — PlatformIO build config (esp32doit-devkit-v1, Arduino framework)
├── diagram.json           — Wokwi circuit schematic
├── wokwi.toml             — Wokwi simulator config
├── .vscode/               — VS Code / PlatformIO IDE settings
└── .gitignore
```

---

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) (CLI or VS Code extension)
- ESP32 board connected via USB
- Wokwi VS Code extension *(optional, for simulation)*

### Build & Flash

```bash
# Build the firmware
pio run

# Build and upload to connected ESP32
pio run --target upload

# Open serial monitor (115200 baud)
pio device monitor
```

### Wokwi Simulation

Open the project folder in VS Code with the Wokwi extension installed, then press **F1 → Wokwi: Start Simulator**. The `diagram.json` and `wokwi.toml` files configure the full virtual circuit automatically.

---

## Dependencies

Managed automatically by PlatformIO via `platformio.ini`:

| Library | Version | Purpose |
|---------|---------|---------|
| `adafruit/Adafruit NeoPixel` | ^1.12.0 | NeoPixel driver (included; not used on discrete RGB circuit) |

> The Adafruit NeoPixel library is present in `platformio.ini` from an earlier iteration. It is not used by the discrete RGB LED wiring but does no harm if left in.

---

## License

MIT