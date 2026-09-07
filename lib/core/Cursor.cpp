#include "Cursor.h"

namespace core {

bool cursorVisible(uint32_t nowMs, uint32_t periodMs) {
  if (periodMs == 0) return true;
  return (nowMs / periodMs) % 2 == 0;
}

}  // namespace core
