# Smart Lighting Controller

An ESP32-WROVER-based automatic room lighting system using PIR motion sensing, relay-controlled lighting, manual override, and a NeoPixel RGB ring for system state indication.

---

## Features

- **Motion-activated lighting** — PIR sensor (GPIO33) triggers a relay (GPIO23) to turn on the load LED when occupancy is detected
- **Dual state machine** — Separate *Room* and *Lighting* state machines run concurrently for clean, deterministic control logic
- **Auto-off timeout** — Light turns off automatically after 5 seconds of no motion (OCCUPIED → IDLE → EMPTY)
- **Manual override** — Pushbutton (GPIO14, falling-edge interrupt) toggles the relay on/off, immediately pre-empting automatic control
- **Override timeout** — System returns to automatic mode 5 seconds after the last button press
- **NeoPixel status indicator** — FREENOVE WS2812 ring (GPIO18) indicates system state:
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
| Microcontroller | ESP32-WROVER | — | 240 MHz dual-core Xtensa LX6 |
| PIR motion sensor | HC-SR501 (or similar) | GPIO33 | Input, HIGH = motion detected |
| Relay module | 4-channel 5V relay | GPIO23 | Only channel 1 used; switches LED load |
| Push button | Momentary, normally open | GPIO14 | Internal pull-up; FALLING edge interrupt |
| NeoPixel ring | FREENOVE WS2812 (4× LEDs) | GPIO18 | Only first LED used for state indication |
| Load LED | Red LED | Via relay NO contact | Simulates room lighting load |
| Resistor | 220Ω × 1 | Load LED | Current limiting |

### Wiring Summary

```
ESP32 GPIO23  →  Relay channel 1 IN
ESP32 GPIO33  →  PIR OUT              (PIR VCC → 5V, PIR GND → GND)
ESP32 GPIO14  →  Button leg 1         (Button leg 2 → GND)
ESP32 GPIO18  →  NeoPixel ring DIN    (Ring VCC → 5V, Ring GND → GND)

Relay COM  →  VCC
Relay NO   →  220Ω → Load LED anode
              Load LED cathode → GND
```

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
├── .vscode/               — VS Code / PlatformIO IDE settings
└── .gitignore
```

---

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) CLI or VS Code extension
- ESP32 connected via USB (CH340 driver required on Windows)

### Build & Flash (On PlatformIO Core CLI)

```
# Build the firmware
pio run

# Build and upload to connected ESP32
pio run --target upload

# Open serial monitor (115200 baud)
pio device monitor
```

---

## Dependencies

Managed automatically by PlatformIO via `platformio.ini`:

| Library | Version | Purpose |
|---------|---------|---------|
| `adafruit/Adafruit NeoPixel` | ^1.12.0 | WS2812 NeoPixel driver |

---

## License

MIT