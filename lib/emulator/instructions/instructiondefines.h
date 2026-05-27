#pragma once

#include "types.h"


// Flag setting utils
enum class Flag : Byte {
  ZERO       = 0x80,
  SUBTRACT   = 0x40,
  HALF_CARRY = 0x20,
  CARRY      = 0x10
};

template<Flag flag>
inline bool getFlag (State &state) {
  return (state.F & static_cast<Byte>(flag)) != 0;
}

inline void clearAllFlags (State &state) {
  state.F = 0x00;
}

template<Flag flag>
void setFlag (State &state) {
  state.F |= static_cast<Byte>(flag);
}

template<Flag flag>
void clearFlag (State &state) {
  state.F &= ~static_cast<Byte>(flag);
}

template<Flag flag>
void condSetFlag (State &state, bool condition) {
  static constexpr Byte MASK = static_cast<Byte>(flag);
  state.F |=  MASK*condition; // Set if true
}


// Double register utils
#define JOIN_REGS(r1, r2) ((DReg(r1) << 8) | DReg(r2))
#define REG_AF(state) JOIN_REGS(state.A, state.F)
#define REG_BC(state) JOIN_REGS(state.B, state.C)
#define REG_DE(state) JOIN_REGS(state.D, state.E)
#define REG_HL(state) JOIN_REGS(state.H, state.L)

#define STORE_SHORT(value, dst_high, dst_low) dst_high = Byte(value >> 8); dst_low = Byte(value & 0xFF)
#define SET_REG_AF(value, state) STORE_SHORT(value, state.A, state.F)
#define SET_REG_BC(value, state) STORE_SHORT(value, state.B, state.C)
#define SET_REG_DE(value, state) STORE_SHORT(value, state.D, state.E)
#define SET_REG_HL(value, state) STORE_SHORT(value, state.H, state.L)