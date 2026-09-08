# PrometheusGrapher

A real-time mathematical function graphing calculator application for the **M5Stack Cardputer** (ESP32-S3 based device). This project provides an intuitive interface to visualize and explore mathematical equations directly on the device's display.

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
  - Real-time plotting of mathematical functions
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
- [:white_check_mark:]  Add scrool speed adjustment
- [:red_square:]  Add asymptote graphing support
- [:red_square:]  multi-equation graphing
- [:red_square:]  Add ability to jump to specific cordinate
- [:hourglass_flowing_sand:]  Add zoom and axis scaling (individual scaling might be tough)
- [:hourglass_flowing_sand:]  Add boot/SplashScreen
- [:red_square:]  Add Settings menu
- [:red_square:]  Add custom plot colours(maybe even have a special rainbow mode)
- [:red_square:]  Update to using multi page help screen to show more controls on-screen
- [:hourglass_flowing_sand:]  Add grid marking lables [at default scaling every line is 5 units]
- [:red_square:]  Add ability to mark points and display cordinates of ced point.
- [:red_square:]  Add screen auto sleep/dim
- [:red_square:]  Push limits of grid rendering out by ~10-20 to prevent them from seemingly spawning in.

## Project Structure

```
PrometheusGrapher/
├── src/
│   ├── main.cpp              # Main application logic and UI
│   ├── ExpressionParser.cpp  # Mathematical expression parser
│   ├── ExpressionParser.h    # Parser interface
│   ├── Settings.cpp          # Settings management
│   ├── Settings.h            # Settings interface
│   ├── SettingsList.h        # Settings list definitions
│   └── Icons.c               # Icon bitmap definitions
├── include/                  # Custom header files
├── lib/                      # External libraries
├── test/                     # Test suite
├── Extra Dev Files/
│   └── Icons/                # Icon development resources
│       ├── Icon_Bitmaps/     # Pre-generated header files
│       └── Icon_Image_Files/ # Source icon images
├── platformio.ini            # PlatformIO configuration
├── build.py                  # Custom build script
├── custom_8Mb.csv            # Flash partition configuration
└── README.md                 # This file
```

## Hardware Requirements

- **M5Stack Cardputer** (ESP32-S3 based)
- USB connection for uploading firmware
- ~8MB Flash memory minimum

## Dependencies

- **M5Cardputer** (v1.1.1+) - M5Stack hardware library
- **M5GFX** - Graphics library for display rendering
- **Arduino Framework** - ESP32 Arduino compatibility layer
- Custom **ExpressionParser** - Mathematical expression evaluation

All dependencies are configured in `platformio.ini` and automatically installed during build.

## Building & Uploading

### Prerequisites
- [PlatformIO](https://platformio.org/install/cli) installed
- M5Stack Cardputer connected via USB

### Build Instructions

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
