#include "Font5x7.h"

namespace core {

const Glyph5x7 kPlaceholderGlyph = {
    0b01110, 0b10101, 0b10011, 0b10101, 0b11001, 0b10101, 0b01110,
};

namespace {

const Glyph5x7 kSpace = {0, 0, 0, 0, 0, 0, 0};

const Glyph5x7 kA = {0b01110, 0b10001, 0b10001, 0b11111,
                      0b10001, 0b10001, 0b10001};
const Glyph5x7 kB = {0b11110, 0b10001, 0b10001, 0b11110,
                      0b10001, 0b10001, 0b11110};
const Glyph5x7 kC = {0b01111, 0b10000, 0b10000, 0b10000,
                      0b10000, 0b10000, 0b01111};
const Glyph5x7 kD = {0b11110, 0b10001, 0b10001, 0b10001,
                      0b10001, 0b10001, 0b11110};
const Glyph5x7 kE = {0b11111, 0b10000, 0b10000, 0b11110,
                      0b10000, 0b10000, 0b11111};
const Glyph5x7 kF = {0b11111, 0b10000, 0b10000, 0b11110,
                      0b10000, 0b10000, 0b10000};
const Glyph5x7 kG = {0b01111, 0b10000, 0b10000, 0b10011,
                      0b10001, 0b10001, 0b01110};
const Glyph5x7 kH = {0b10001, 0b10001, 0b10001, 0b11111,
                      0b10001, 0b10001, 0b10001};
const Glyph5x7 kI = {0b01110, 0b00100, 0b00100, 0b00100,
                      0b00100, 0b00100, 0b01110};
const Glyph5x7 kJ = {0b00111, 0b00010, 0b00010, 0b00010,
                      0b00010, 0b10010, 0b01100};
const Glyph5x7 kK = {0b10001, 0b10010, 0b10100, 0b11000,
                      0b10100, 0b10010, 0b10001};
const Glyph5x7 kL = {0b10000, 0b10000, 0b10000, 0b10000,
                      0b10000, 0b10000, 0b11111};
const Glyph5x7 kM = {0b10001, 0b11011, 0b10101, 0b10101,
                      0b10001, 0b10001, 0b10001};
const Glyph5x7 kN = {0b10001, 0b11001, 0b10101, 0b10101,
                      0b10011, 0b10001, 0b10001};
const Glyph5x7 kO = {0b01110, 0b10001, 0b10001, 0b10001,
                      0b10001, 0b10001, 0b01110};
const Glyph5x7 kP = {0b11110, 0b10001, 0b10001, 0b11110,
                      0b10000, 0b10000, 0b10000};
const Glyph5x7 kQ = {0b01110, 0b10001, 0b10001, 0b10001,
                      0b10101, 0b10010, 0b01101};
const Glyph5x7 kR = {0b11110, 0b10001, 0b10001, 0b11110,
                      0b10100, 0b10010, 0b10001};
const Glyph5x7 kS = {0b01111, 0b10000, 0b10000, 0b01110,
                      0b00001, 0b00001, 0b11110};
const Glyph5x7 kT = {0b11111, 0b00100, 0b00100, 0b00100,
                      0b00100, 0b00100, 0b00100};
const Glyph5x7 kU = {0b10001, 0b10001, 0b10001, 0b10001,
                      0b10001, 0b10001, 0b01110};
const Glyph5x7 kV = {0b10001, 0b10001, 0b10001, 0b10001,
                      0b10001, 0b01010, 0b00100};
const Glyph5x7 kW = {0b10001, 0b10001, 0b10001, 0b10101,
                      0b10101, 0b10101, 0b01010};
const Glyph5x7 kX = {0b10001, 0b10001, 0b01010, 0b00100,
                      0b01010, 0b10001, 0b10001};
const Glyph5x7 kY = {0b10001, 0b10001, 0b01010, 0b00100,
                      0b00100, 0b00100, 0b00100};
const Glyph5x7 kZ = {0b11111, 0b00001, 0b00010, 0b00100,
                      0b01000, 0b10000, 0b11111};

const Glyph5x7 k0 = {0b01110, 0b10001, 0b10011, 0b10101,
                      0b11001, 0b10001, 0b01110};
const Glyph5x7 k1 = {0b00100, 0b01100, 0b00100, 0b00100,
                      0b00100, 0b00100, 0b01110};
const Glyph5x7 k2 = {0b01110, 0b10001, 0b00001, 0b00010,
                      0b00100, 0b01000, 0b11111};
const Glyph5x7 k3 = {0b11111, 0b00010, 0b00100, 0b00010,
                      0b00001, 0b10001, 0b01110};
const Glyph5x7 k4 = {0b00010, 0b00110, 0b01010, 0b10010,
                      0b11111, 0b00010, 0b00010};
const Glyph5x7 k5 = {0b11111, 0b10000, 0b11110, 0b00001,
                      0b00001, 0b10001, 0b01110};
const Glyph5x7 k6 = {0b00110, 0b01000, 0b10000, 0b11110,
                      0b10001, 0b10001, 0b01110};
const Glyph5x7 k7 = {0b11111, 0b00001, 0b00010, 0b00100,
                      0b01000, 0b01000, 0b01000};
const Glyph5x7 k8 = {0b01110, 0b10001, 0b10001, 0b01110,
                      0b10001, 0b10001, 0b01110};
const Glyph5x7 k9 = {0b01110, 0b10001, 0b10001, 0b01111,
                      0b00001, 0b00010, 0b01100};

const Glyph5x7 kPeriod = {0, 0, 0, 0, 0, 0b01100, 0b01100};
const Glyph5x7 kComma = {0, 0, 0, 0, 0b01100, 0b01100, 0b01000};
const Glyph5x7 kBang = {0b00100, 0b00100, 0b00100, 0b00100,
                         0b00100, 0, 0b00100};
const Glyph5x7 kQuestion = {0b01110, 0b10001, 0b00001, 0b00010,
                             0b00100, 0, 0b00100};
const Glyph5x7 kApostrophe = {0b01000, 0b01000, 0, 0, 0, 0, 0};
const Glyph5x7 kHyphen = {0, 0, 0, 0b11111, 0, 0, 0};

// --- extras for math expression display (^ * / + ( ) :) ---
const Glyph5x7 kCaret = {0b00100, 0b01010, 0b10001, 0, 0, 0, 0};
const Glyph5x7 kAsterisk = {0, 0b10101, 0b01110, 0b11111, 0b01110, 0b10101, 0};
const Glyph5x7 kSlash = {0b00001, 0b00010, 0b00010, 0b00100, 0b01000, 0b01000, 0b10000};
const Glyph5x7 kPlus = {0, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0};
const Glyph5x7 kLParen = {0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010};
const Glyph5x7 kRParen = {0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000};
const Glyph5x7 kColon = {0, 0b00100, 0b00100, 0, 0b00100, 0b00100, 0};

}  // namespace

const uint8_t* glyphFor(char c) {
  if (c >= 'a' && c <= 'z') c = char(c - 'a' + 'A');  // fold to upper

  switch (c) {
    case ' ': return kSpace;
    case 'A': return kA; case 'B': return kB; case 'C': return kC;
    case 'D': return kD; case 'E': return kE; case 'F': return kF;
    case 'G': return kG; case 'H': return kH; case 'I': return kI;
    case 'J': return kJ; case 'K': return kK; case 'L': return kL;
    case 'M': return kM; case 'N': return kN; case 'O': return kO;
    case 'P': return kP; case 'Q': return kQ; case 'R': return kR;
    case 'S': return kS; case 'T': return kT; case 'U': return kU;
    case 'V': return kV; case 'W': return kW; case 'X': return kX;
    case 'Y': return kY; case 'Z': return kZ;
    case '0': return k0; case '1': return k1; case '2': return k2;
    case '3': return k3; case '4': return k4; case '5': return k5;
    case '6': return k6; case '7': return k7; case '8': return k8;
    case '9': return k9;
    case '.': return kPeriod;
    case ',': return kComma;
    case '!': return kBang;
    case '?': return kQuestion;
    case '\'': return kApostrophe;
    case '-': return kHyphen;
    case '^': return kCaret;
    case '*': return kAsterisk;
    case '/': return kSlash;
    case '+': return kPlus;
    case '(': return kLParen;
    case ')': return kRParen;
    case ':': return kColon;
    default: return kPlaceholderGlyph;
  }
}

}  // namespace core
