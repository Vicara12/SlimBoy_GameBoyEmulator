#pragma once

/*
* Code for all the bit arithmetic instructions
*/

#include "../state.h"
#include "instructiondefines.h"
#include "../memory/memory.h"


namespace gb {

// SWAP n: swap upper and lower nibbles of n
inline int instr_SWAP_n (Reg &n, State &state)
{
  clearAllFlags(state);
  condSetFlag<Flag::ZERO>(state, n == 0);
  Byte low_nibble = n & 0x0F;
  n = (n >> 4) | (low_nibble << 4);
  return 8;
}

// SWAP (HL): swap upper and lower nibbles of data stored at (HL)
inline int instr_SWAP_mem_HL (State &state)
{
  DReg HL = REG_HL(state);
  Reg mem_val = state.memory.r(HL);
  instr_SWAP_n(mem_val, state);
  state.memory.w(HL, mem_val);
  return 16;
}

// CPL: flip all bits in register A
inline int instr_CPL (State &state)
{
  setFlag<Flag::SUBTRACT>(state);
  setFlag<Flag::HALF_CARRY>(state);
  state.A = ~(state.A);
  return 4;
}

// complement = false: SCF: set carry flag
// complement = true:  CCF: flip carry flag
inline int instr_CCF_SCF (State &state, bool complement)
{
  clearFlag<Flag::SUBTRACT>(state);
  clearFlag<Flag::HALF_CARRY>(state);
  if (complement) {
    state.F ^= static_cast<Byte>(Flag::CARRY);
  } else {
    setFlag<Flag::CARRY>(state);
  }
  return 4;
}

// RLC n: rotate n left, old bit 7 to carry flag
inline int instr_RLC_n (Reg &n, State &state, bool cb_prefix)
{
  clearAllFlags(state);
  Byte high_bit = (n & 0x80) != 0;
  condSetFlag<Flag::CARRY>(state, high_bit);
  n = (n << 1) | high_bit ;
  // For some reason the CB and non-CB version with reg A work differently
  if (cb_prefix) {
    condSetFlag<Flag::ZERO>(state, n == 0);
    return 8;
  }
  return 4;
}

// RLC (HL)
inline int instr_RLC_mem_HL (State &state)
{
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_RLC_n(val, state, true);
  state.memory.w(HL, val);
  return 16;
}

// RL n: rotate left through carry flag
inline int instr_RL_n (Reg &n, State &state, bool cb_prefix)
{
  Byte old_carry = getFlag<Flag::CARRY>(state);
  clearAllFlags(state);
  condSetFlag<Flag::CARRY>(state, (n & 0x80) != 0);
  n = (n << 1) | old_carry;
  // For some reason the CB and non-CB version with reg A work differently
  if (cb_prefix) {
    condSetFlag<Flag::ZERO>(state, n == 0);
    return 8;
  }
  return 4;
}

// RL (HL)
inline int instr_RL_mem_HL (State &state)
{
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_RL_n(val, state, true);
  state.memory.w(HL, val);
  return 16;
}

// RRC n: rotate n right, old bit 0 to carry flag
inline int instr_RRC_n (Reg &n, State &state, bool cb_prefix)
{
  clearAllFlags(state);
  Byte low_bit = (n & 0x01) != 0;
  condSetFlag<Flag::CARRY>(state, low_bit);
  n = (n >> 1) | (low_bit << 7);
  // For some reason the CB and non-CB version with reg A work differently
  if (cb_prefix) {
    condSetFlag<Flag::ZERO>(state, n == 0);
    return 8;
  }
  return 4;
}

// RRC (HL)
inline int instr_RRC_mem_HL (State &state)
{
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_RRC_n(val, state, true);
  state.memory.w(HL, val);
  return 16;
}

// RR n: rotate right through carry flag
inline int instr_RR_n (Reg &n, State &state, bool cb_prefix)
{
  Byte old_carry = getFlag<Flag::CARRY>(state);
  clearAllFlags(state);
  condSetFlag<Flag::CARRY>(state, (n & 0x01) != 0);
  n = (n >> 1) | (old_carry*0x80);
  if (cb_prefix) {
    condSetFlag<Flag::ZERO>(state, n == 0);
    return 8;
  }
  return 4;
}

// RR (HL)
inline int instr_RR_mem_HL (State &state)
{
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_RR_n(val, state, true);
  state.memory.w(HL, val);
  return 16;
}

// SLA n: shift left arithmetical into carry
inline int instr_SLA_n (Reg &reg, State &state)
{
  clearAllFlags(state);
  condSetFlag<Flag::CARRY>(state, (reg & 0x80) != 0);
  reg = reg << 1;
  condSetFlag<Flag::ZERO>(state, reg == 0);
  return 8;
}

// SLA (HL)
inline int instr_SLA_mem_HL (State &state)
{
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_SLA_n(val, state);
  state.memory.w(HL, val);
  return 16;
}

// SRA n: shift right into carry
inline int instr_SRA_n (Reg &reg, State &state)
{
  clearAllFlags(state);
  condSetFlag<Flag::CARRY>(state, reg & 0x01);
  reg = Byte(SByte(reg) >> 1);
  condSetFlag<Flag::ZERO>(state, reg == 0);
  return 8;
}

// SRA (HL)
inline int instr_SRA_mem_HL (State &state)
{
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_SRA_n(val, state);
  state.memory.w(HL, val);
  return 16;
}

// SRL n: logical shift n right into carry
inline int instr_SRL_n (Reg &reg, State &state)
{
  clearAllFlags(state);
  condSetFlag<Flag::CARRY>(state, reg & 0x01);
  reg = reg >> 1;
  condSetFlag<Flag::ZERO>(state, reg == 0);
  return 8;
}

// SRL (HL)
inline int instr_SRL_mem_HL (State &state)
{
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_SRL_n(val, state);
  state.memory.w(HL, val);
  return 16;
}

// BIT b, r: test bit b in register r
template <int b>
inline int instr_BIT_b_r (Reg &reg, State &state)
{
  constexpr Byte bit_mask = 1 << b;
  clearFlag<Flag::SUBTRACT>(state);
  setFlag<Flag::HALF_CARRY>(state);
  clearFlag<Flag::ZERO>(state);
  condSetFlag<Flag::ZERO>(state, (reg & bit_mask) == 0);
  return 8;
}

// BIT b, (HL)
template <int b>
inline int instr_BIT_b_mem_HL (State &state)
{
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_BIT_b_r<b>(val, state);
  state.memory.w(HL, val);
  return 12;
}

// SET b, r: set bit b in register r
template<int b>
inline int instr_SET_b_r (Reg &reg)
{
  constexpr Byte bit_mask = 1 << b;
  reg |= bit_mask;
  return 8;
}

// SET b, (HL)
template<int b>
inline int instr_SET_b_mem_HL (State &state)
{
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_SET_b_r<b>(val);
  state.memory.w(HL, val);
  return 16;
}

// RES b, r: reset bit b in register r
template<int b>
inline int instr_RES_b_r (Reg &reg)
{
  constexpr Byte bit_mask = Byte(~(1 << b));
  reg &= bit_mask;
  return 8;
}

// RES b, (HL)
template<int b>
inline int instr_RES_b_mem_HL (State &state)
{
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_RES_b_r<b>(val);
  state.memory.w(HL, val);
  return 16;
}

} // namespace gb