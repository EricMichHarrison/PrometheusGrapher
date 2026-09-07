#pragma once
// lib/core/Font5x7.h
//
// Minimal 5x7 bitmap font. Each glyph is 7 bytes; each byte's low 5 bits
// are the pixel row (MSB-of-the-5 = leftmost column). This is a small,
// deliberately-scoped table — uppercase A-Z, digits 0-9, space, and a
// handful of punctuation — enough to demo the rendering + testing
// pipeline end to end. Unmapped characters fall back to a placeholder
// box glyph (kPlaceholderGlyph) rather than drawing nothing, so typing
// never "eats" a keystroke silently.
//
// Extending this table is just adding another case to glyphFor(); it is
// intentionally NOT code-generated (unlike the reference project's
// design/tools/gen.py pipeline) to keep this skeleton dependency-free.
// A generator script would be a natural next step for a real app.

#include <cstdint>

namespace core {

using Glyph5x7 = uint8_t[7];

extern const Glyph5x7 kPlaceholderGlyph;

// Returns a pointer to a 7-row glyph bitmap for `c`. Always returns a
// valid pointer (never null) — unmapped characters return
// kPlaceholderGlyph.
const uint8_t* glyphFor(char c);

}  // namespace core
