# PrometheusGrapher

A real-time mathematical function graphing calculator application for the **M5Stack Cardputer** (ESP32-S3 based device), by SupremeEgg75. This project provides an intuitive interface to visualize and explore mathematical equations directly on the device's display.

## Migration note (read this first)

This copy of the project has been migrated onto a testable core/hal/ui/sim
architecture (the same one used in a companion "cardputer-typewriter" demo
project). If you're comparing this against your original source, here's
exactly what changed and why:

- **`ExpressionParser` moved to `lib/core/`**, ported from Arduino's
  `String` to `std::string` and with `<Arduino.h>` dropped, so it has
  zero hardware includes. Same grammar, same AST, same evaluation
  semantics — verified against the original algorithm's expected
  output for 17 cases (precedence, implicit multiplication, domain
  errors, parse failures) before being trusted. Device code converts
  at the boundary (`std::string(arduinoString.c_str())`).
- **New: `lib/core/GrapherState`** — the equation slots, which one is
  graphed, and the pan position, which used to be loose globals
  (`lineEntries[]`, `selEquation`, `posx`/`posy`) in `main.cpp`. Now a
  small hardware-free class with native tests.
- **New: `lib/core/Framebuffer` + `lib/ui/GraphRenderer`** — and this
  is the one actual behavior change, not just a refactor: **the
  original `DrawGraphPage()` never actually plotted the curve.** It
  drew the grid and axes and then just printed the selected equation's
  slot *number* where the comment says
  `// === PUT EQUATION GRAPHING DRAWER HERE ===`. `GraphRenderer` fills
  that in — it samples the equation across the screen width using the
  existing (but previously unused for this) `evaluateExpression()` and
  actually draws the curve, correctly leaving gaps where the function
  is undefined (e.g. `sqrt(x)` for negative `x`). Verified visually —
  a plotted sine wave, a parabola, and the `sqrt(x)` domain gap all
  render exactly as expected; see the screenshots referenced in the
  handoff conversation.
- **`FunctionMenu()`, `SelectExpression()` (renamed
  `SelectEquationSlot()`), and `helpScreen()` were deliberately NOT
  migrated** — they're menu/list screens with no logic worth
  unit-testing, so rewriting their working M5GFX drawing code wasn't
  worth the risk for this pass. They now read/write equation text
  through `GrapherState` instead of touching `lineEntries[]` directly,
  but draw the same way they always did.
- **`Settings.cpp` (marked WIP in the original) is untouched.**
- **`platformio.ini`**: the original `[env:cardputer_adv]` block is
  preserved byte-for-byte (every commented-out feature flag included)
  with one addition — a `build_src_filter` so it only builds
  `src/device/*`, since `src/sim/*` (new) is a native-only simulator
  that must never end up in the firmware. Two new environments,
  `[env:test-native]` and `[env:sim]`, were added alongside it.

**What's verified vs. not**, same honesty as always: `lib/core` and
`lib/ui` were compiled and run directly with a plain `g++` in this
sandbox (no PlatformIO here) — parser cross-checks, `GrapherState`
logic, and rendered output (including real plotted curves, viewed as
images) were all confirmed against real compiled output before being
written into `test/test_native/test_main.cpp`. **`src/device/*` and
`[env:cardputer_adv]` were NOT compiled** — no ESP32 toolchain or
M5Unified/M5GFX libraries available in this sandbox. The device code
is a faithful, mechanical port of logic that already worked in your
original `main.cpp`; the main place to look first if something's off
is `DrawGraphPage()`'s redraw-tracking (`lastPanX`/`lastPanY`/
`lastSelected`), since that's the one part restructured rather than
copied verbatim (see the comment block at the top of
`src/device/main.cpp` for exactly what changed and why).

### Building and testing this version

```bash
pio run -e cardputer_adv                    # the real firmware, unchanged target
pio test -e test-native                     # native tests: parser, state, render regressions
pio run -e sim && .pio/build/sim/program --screens captures/
.pio/build/sim/program --serve 8123         # open http://localhost:8123 — pan, graph equations, no shell needed
.pio/build/sim/program --live               # same, but in this terminal instead of a browser
```

`--serve` gives you a text box to type a new equation into a slot and
graph it immediately, plus arrow keys to pan and digit keys 1-4 to
graph an existing slot — all against the exact same `GrapherState` +
`GraphRenderer` code the device runs.

---

## Overview

PrometheusGrapher is a specialized graphing application designed for the M5Stack Cardputer platform. It allows users to enter and visualize mathematical expressions in real-time, with features for equation management, graph navigation, and customizable display settings.

## Features

### Current Features
- **Expression Parser**: Full mathematical expression parser supporting:
  - Basic arithmetic operations: `+`, `-`, `*`, `/`
  - Exponentiation: `x^2`, `x^3`, etc.
  - Trigonometric functions: `sin()`, `cos()`, `tan()`
  - Mathematical functions: `sqrt()`, `abs()`, `log()`, `ln()`, `exp()`
  - Single variable (`x`) function evaluation
  - Support for constants like `pi`

- **Interactive Graph Viewer**:
  - Real-time plotting of mathematical functions (newly implemented as
    part of the migration above — see the migration note)
  - Grid overlay with customizable marker lines
  - Screen navigation/panning with adjustable step speed
  - Automatic scaling based on device display

- **Equation Management**:
  - Support for multiple equation entries (currently 4 slots)
  - Equation selection and editing interface
  - Visual indicators for currently plotted equations
  - Persistent equation entries across sessions

- **User Interface**:
  - Menu-driven function selection
  - Customizable color scheme (system color, background color, step color)
  - Keyboard input via M5Stack Cardputer keyboard
  - Visual feedback with triangular selection indicators

- **Hardware Integration**:
  - Optimized for M5Stack Cardputer display
  - Adjustable brightness control
  - Serial communication for debugging

### Planned Features (To-Do)
- [ ] Scroll speed adjustment
- [ ] Asymptote graphing support
- [ ] Multi-equation simultaneous graphing
- [ ] Camera positioning at specific coordinates
- [ ] Zoom and axis scaling capabilities
- [ ] Boot/splash screen
- [ ] Settings menu
- [ ] Custom plot colors (including rainbow mode)

## Project Structure

```
PrometheusGrapher/
├── lib/
│   ├── core/                 # Hardware-free logic (natively testable)
│   │   ├── ExpressionParser.h/.cpp
│   │   ├── GrapherState.h/.cpp
│   │   ├── Framebuffer.h/.cpp
│   │   ├── Font5x7.h/.cpp
│   │   └── Cursor.h/.cpp
│   ├── ui/
│   │   └── GraphRenderer.h/.cpp   # grid/axes/curve -> Framebuffer
│   └── hal/
│       ├── IDisplay.h
│       └── IKeyboard.h
├── src/
│   ├── device/                # M5Stack Cardputer firmware
│   │   ├── main.cpp
│   │   ├── DisplayM5.h
│   │   ├── Settings.cpp/.h, SettingsList.h
│   │   └── Icons.c
│   └── sim/                   # native simulator (no device needed)
│       ├── main.cpp
│       ├── PpmWriter.h/.cpp
│       ├── TerminalView.h/.cpp
│       ├── InputTerminal.h/.cpp
│       ├── HttpServer.h/.cpp
│       └── WebPage.h
├── test/test_native/          # native Unity tests
├── scripts/link_winsock.py    # Windows-only linker flag for --serve
├── platformio.ini
├── build.py
├── custom_8Mb.csv
└── README.md
```

## Hardware Requirements

- **M5Stack Cardputer** (ESP32-S3 based)
- USB connection for uploading firmware
- ~8MB Flash memory minimum

## Dependencies

- **M5Cardputer** (v1.1.1+) - M5Stack hardware library
- **M5GFX** - Graphics library for display rendering
- **Arduino Framework** - ESP32 Arduino compatibility layer
- Custom **ExpressionParser** - Mathematical expression evaluation (now in `lib/core/`)

All dependencies are configured in `platformio.ini` and automatically installed during build.

## Building & Uploading

```bash
# Build the project
pio run -e cardputer_adv

# Upload to device
pio run -e cardputer_adv --target upload

# Monitor serial output
pio run -e cardputer_adv --target monitor
```

### Configuration

Edit `platformio.ini` to adjust:
- Upload/monitor port (currently commented out, set as needed)
- Serial monitor speed: `115200 baud`
- Build flags and feature definitions

## Usage

### Main Application Flow

1. **Startup**: Device initializes with display brightness set to 10%, then increases to 100%
2. **Graph Viewing**: Main drawing loop renders the currently selected mathematical function
3. **Function Menu**: Interface for selecting and managing equations
4. **Equation Selection**: Choose from 4 available equation slots
5. **Navigation**: Pan the graph view using keyboard controls

### Example Equations

- `x^2` - Basic quadratic
- `sin(x)` - Sine wave
- `cos(x) * 2` - Scaled cosine
- `x^3 - 2*x` - Cubic polynomial
- `sqrt(x)` - Square root (positive x)
- `1/x` - Hyperbola

## Color Scheme

The application uses configurable colors for:
- `SystemColour` - UI elements and grid lines
- `BackgroundColour` - Plot background
- `StepColour` - Coordinate markers

## Technical Details

### Expression Parser Architecture
- Recursive descent parser with operator precedence
- Abstract Syntax Tree (AST) for efficient evaluation
- Supports function composition
- Includes domain checking (prevents crashes from invalid operations)

### Display Constants
- Screen dimensions determined at runtime from device
- Configurable grid step size (default: 20 pixels)
- Camera position tracking for panning (cx, cy)
- Position adjustment step for navigation (default: 5 units)

### Input Handling
- Debounce delay: 20ms (prevents input bouncing)
- Keyboard input integration with M5Cardputer keyboard
- Real-time UI updates on input changes

## Serial Communication

The device outputs debug information at 115200 baud. Connected devices can monitor:
- Startup messages
- Parser debug information
- System state updates

## License

AGPL-3.0

## Credits

- **Author**: SupremeEgg75
- **Inspiration**: Based on "voidos" by avascik
- **Platform**: M5Stack Cardputer

## Repository

[PrometheusGrapher on GitHub](https://github.com/personwithbeans/PrometheusGrapher)
