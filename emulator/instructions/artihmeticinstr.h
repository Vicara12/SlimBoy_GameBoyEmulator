#pragma once

/*
* Code for all the arithmetic instructions
*/

#include "state.h"
#include "memory/memory.h"


namespace gb {

inline void handleFlag (Short a, Short b, Short result, State &state) {
  condSetFlag<Flag::ZERO>(state, Byte(result) == 0);
  Short xored = (a ^ b ^ result);
  condSetFlag<Flag::HALF_CARRY>(state, xored & 0x10);
  condSetFlag<Flag::CARRY>(state, result > 0xFF);
}


// immediate = true  and use_carry = false: ADD A, n: add 8 bit immediate n to register A
// immediate = false and use_carry = false: ADD A, r: add register r to register A
// immediate = true  and use_carry = true: ADC A, n: add 8 bit immediate n + carry flag to register A
// immediate = false and use_carry = true: ADC A, r: add register r + carry flag to register A
template<bool immediate, bool use_carry>
inline int instr_ADX_A_r (Reg r, State &state) {
  Short carry = (use_carry and getFlag<Flag::CARRY>(state) ? 1 : 0);
  Short result = Short(state.A) + Short(r) + carry;
  clearAllFlags(state);
  handleFlag(state.A, r, result, state);
  state.A = Byte(result);
  return (immediate ? 8 : 4);
}


// carry = false: ADD A, (HL): add content of memory at address HL into register A
// carry = true:  ADC A, (HL): add content of memory at address HL + carry into register A
template<bool use_carry>
inline int instr_ADX_A_mem_HL (State &state) {
  Byte r = state.memory.r(REG_HL(state));
  return instr_ADX_A_r<true, use_carry>(r, state);
}


// immediate = true  and carry = false: SUB A, n: subtract 8 bit immediate n to register A
// immediate = false and carry = false: SUB A, r: subtract register r to register A
// immediate = true  and carry = true: SBC A, n: subtract 8 bit immediate n + carry flag to register A
// immediate = false and carry = true: SBC A, r: subtract register r + carry flag to register A
template<bool immediate, bool use_carry>
inline int instr_SBX_A_r (Reg r, State &state) {
  Short carry = (use_carry and getFlag<Flag::CARRY>(state) ? 1 : 0);
  Short result = Short(state.A) - Short(r) - carry;
  clearAllFlags(state);
  setFlag<Flag::SUBTRACT>(state);
  handleFlag(state.A, r, result, state);
  state.A = Byte(result);
  return (immediate ? 8 : 4);
}


// carry = false: SUB A, (HL): subtract content of memory at address HL into register A
// carry = true:  SBC A, (HL): subtract content of memory at address HL + carry into register A
template<bool use_carry>
inline int instr_SBX_A_mem_HL (State &state) {
  Byte r = state.memory.r(REG_HL(state));
  return instr_SBX_A_r<true, use_carry>(r, state);
}


// immediate = true  and type = 0: AND n: and 8 bit immediate n with register A
// immediate = false and type = 0: AND r: and register r or (HL) with register A
// immediate = true  and type = 1: OR n: or 8 bit immediate n with register A
// immediate = false and type = 1: OR r: or register r or (HL) with register A
// immediate = true  and type = 2: XOR n: xor 8 bit immediate n with register A
// immediate = false and type = 2: XOR r: xor register r or (HL) with register A
template<bool immediate, bool use_hl, int type>
inline int instr_AND_OR_XOR_A_r (Reg r, State &state) {
  if constexpr (use_hl) {
    r = state.memory.r(REG_HL(state));
  }
  clearAllFlags(state);
  if constexpr (type == 0) {
    setFlag<Flag::HALF_CARRY>(state);
    state.A &= r;
  }
  else if constexpr (type == 1) {
    state.A |= r;
  }
  else {
    state.A ^= r;
  }
  condSetFlag<Flag::ZERO>(state, state.A == 0);
  return (immediate or use_hl ? 8 : 4);
}


// CP r: compare A with register, 8 bit value or (HL)
template<bool immediate, bool use_hl>
inline int instr_CP_r (Reg r, State &state) {
  if constexpr (use_hl) {
    r = state.memory.r(REG_HL(state));
  }
  Short result = Short(state.A) - Short(r);
  clearAllFlags(state);
  setFlag<Flag::SUBTRACT>(state);
  handleFlag(state.A, r, result, state);
  return (immediate or use_hl ? 8 : 4);
}


// dec = false: INC r: increment register
// dec = true:  DEC r: decrement register
template<bool dec>
inline int instr_INC_DEC_r (Reg &r, State &state) {
  // can't do clear all as caryy flag can't be affected
  clearFlag<Flag::HALF_CARRY>(state);
  if constexpr (dec) {
    r--;
    setFlag<Flag::SUBTRACT>(state);
    condSetFlag<Flag::HALF_CARRY>(state, (r & 0x0F) == 0xF);
  } else {
    r++;
    clearFlag<Flag::SUBTRACT>(state);
    condSetFlag<Flag::HALF_CARRY>(state, (r & 0x0F) == 0x0);
  }
  clearFlag<Flag::ZERO>(state);
  condSetFlag<Flag::ZERO>(state, r == 0);
  return 4;
}


// dec = false: INC (HL): increment (HL)
// dec = true:  DEC (HL): decrement (HL)
template<bool dec>
inline int instr_INC_DEC_mem_HL (State &state) {
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_INC_DEC_r<dec>(val, state);
  state.memory.w(HL, val); // this is done to use the special write function
  return 12;
}


// ADD HL, n: add two byte register n to HL
inline int instr_ADD_HL_n (DReg reg, State &state) {
  clearFlag<Flag::SUBTRACT>(state);
  clearFlag<Flag::HALF_CARRY>(state);
  clearFlag<Flag::CARRY>(state);
  DReg HL = REG_HL(state);
  uint32_t result = uint32_t(HL) + uint32_t(reg);
  uint32_t carry = reg ^ HL ^ result;
  condSetFlag<Flag::HALF_CARRY>(state, carry & 0x1000);
  condSetFlag<Flag::CARRY>(state, carry & 0x10000);
  SET_REG_HL(Short(result), state);
  return 8;
}


// ADD SP, n: add one byte signed immediate to SP
inline int instr_ADD_SP_n (SByte n, State &state) {
  clearAllFlags(state);
  Short result = state.SP + Short(n);
  Short carry = state.SP ^ Short(n) ^ result;
  condSetFlag<Flag::HALF_CARRY>(state, carry & 0x0010);
  condSetFlag<Flag::CARRY>(state, carry & 0x0100);
  state.SP = result;
  return 16;
}


// dec = false: INC nn: increment two byte register
// dec = true:  DEC nn: decrement two byte register
template<bool dec>
inline int instr_INC_DEC_nn_16bits (Reg &reg_high, Reg &reg_low) {
  DReg nn = JOIN_REGS(reg_high, reg_low);
  if constexpr (dec) nn--;
  else               nn++;
  STORE_SHORT(nn, reg_high, reg_low);
  return 8;
}


// dec = false: INC SP: increment stack pointer
// dec = true:  DEC SP: decrement stack pointer
template<bool dec>
inline int instr_INC_DEC_SP (State &state) {
  if constexpr (dec) state.SP--;
  else               state.SP++;
  return 8;
}

} // namespace gb