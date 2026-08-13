# Cardputer Graphing Calculator

A `y = f(x)` graphing calculator for the M5Stack Cardputer / Cardputer ADV, built with PlatformIO + Arduino framework + the `M5Cardputer` library.

## Build & flash

```bash
# from this folder
pio run                 # build
pio run -t upload       # flash over USB
pio device monitor       # serial monitor (115200 baud), optional
```

Requires [PlatformIO](https://platformio.org/) (CLI or the VS Code extension). No manual library install needed — `platformio.ini` pulls in `M5Cardputer` (which brings in `M5GFX`/`M5Unified`) automatically on first build.

If `pio run -t upload` can't find the port, plug in the Cardputer, make sure it's powered on, and pass the port explicitly: `pio run -t upload --upload-port /dev/ttyACM0` (or `COM3` on Windows).

## Using it

Type an expression in terms of `x` on the keyboard — it appears on the bottom bar and the graph redraws live above it.

### Supported syntax
- Operators: `+ - * / ^ ( )`
- Variable: `x`
- Constants: `pi`, `e`
- Functions: `sin cos tan asin acos atan sqrt abs log ln exp floor ceil`
- Two-argument power: `pow(a, b)`

Examples: `sin(x)`, `x^2 - 3`, `sqrt(abs(x))`, `sin(x)*cos(2*x)`, `1/x`, `pow(x,3)-2*x`

### Controls

| Keys | Action |
|---|---|
| letters / digits / operators | edit the expression |
| `Enter` | force re-plot (also happens automatically as you type) |
| `Del` | delete last character |
| `Fn` + `Del` | clear the whole expression |
| `Fn` + `,` | pan left |
| `Fn` + `/` | pan right |
| `Fn` + `;` | pan up |
| `Fn` + `.` | pan down |
| `Fn` + `=` | zoom in |
| `Fn` + `-` | zoom out |
| `Fn` + `r` | reset view to the default window |

If the expression can't be parsed (syntax error, unknown function, etc.), the input bar turns red and shows `ERR`.

## Notes / possible extensions

- The parser is a small hand-written recursive-descent evaluator (see `parseExpression`/`parseTerm`/`parsePower`/`parsePrimary` in `src/main.cpp`) — it re-evaluates the expression once per horizontal pixel (240 times) on every redraw, which is cheap enough to feel instant on the ESP32-S3.
- Points where the function is undefined (division by zero, `sqrt` of a negative number, etc.) simply break the line instead of connecting across the gap.
- The board target in `platformio.ini` (`esp32-s3-devkitc-1` + `M5Cardputer` lib) is the same one M5Stack's own docs use for both the original Cardputer and the Cardputer ADV, since they share the same Stamp-S3A/ESP32-S3 core and 240x135 panel.
- Ideas if you want to extend it: multiple traces, a cursor/trace readout of `(x, y)`, saved favorite expressions on the SD card, derivative/tangent-line overlay, or table-of-values mode.
