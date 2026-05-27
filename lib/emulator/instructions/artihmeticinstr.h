#pragma once

/*
* Code for all the arithmetic instructions
*/

#include "state.h"
#include "memory/memory.h"


// When using Byte data type the function given to f should use data type Short (2 bytes), as the
// calculation for the carry requires more precision
template<typename FuncType>
inline void handleFlags (Byte a, Byte b, State &state, FuncType f, bool added_carry)
{
  // TODO check this piece of trash
  condSetFlag<Flag::ZERO>(state, Byte(f(a, b)) == 0);
  // If carry is to be taken into account, divide into two operations
  if (added_carry) {
    condSetFlag<Flag::HALF_CARRY>(state, (f((a & 0x0F), 1) & 0xF0) != 0);
    condSetFlag<Flag::HALF_CARRY>(state, (f((f(a, 1) & 0x0F), ((b-1) & 0x0F)) & 0xF0) != 0);
    condSetFlag<Flag::CARRY>(state, (f(a, 1) & 0xFF00) != 0);
    condSetFlag<Flag::CARRY>(state, (f(Byte(f(a, 1)), Byte(b-1)) & 0xFF00) != 0);
  } else {
    condSetFlag<Flag::HALF_CARRY>(state, (f((a & 0x0F), (b & 0x0F)) & 0xF0) != 0);
    condSetFlag<Flag::CARRY>(state, (f(a, b) & 0xFF00) != 0);
  }
}

// immediate = true  and carry = false: ADD A, n: add 8 bit immediate n to register A
// immediate = false and carry = false: ADD A, r: add register r to register A
// immediate = true  and carry = true: ADC A, n: add 8 bit immediate n + carry flag to register A
// immediate = false and carry = true: ADC A, r: add register r + carry flag to register A 
inline int instr_ADX_A_r (Reg r, State &state, bool immediate, bool carry)
{
  bool add_carry = ((state.F & static_cast<Byte>(Flag::CARRY)) != 0)*carry;
  r += Byte(add_carry);
  clearAllFlags(state);
  handleFlags(state.A, r, state, [](Short a, Short b)->Short {return a+b;}, add_carry);
  state.A += r;
  return 4 + 4*immediate;
}

// carry = false: ADD A, (HL): add content of memory at address HL into register A
// carry = true:  ADC A, (HL): add content of memory at address HL + carry into register A
inline int instr_ADX_A_mem_HL (State &state, bool carry)
{
  Byte r = state.memory.r(REG_HL(state));
  bool add_carry = ((state.F & static_cast<Byte>(Flag::CARRY)) != 0)*carry;
  r += Byte(add_carry);
  clearAllFlags(state);
  handleFlags(state.A, r, state, [](Short a, Short b)->Short {return a+b;}, add_carry);
  state.A += r;
  return 8;
}

// immediate = true  and carry = false: SUB A, n: subtract 8 bit immediate n to register A
// immediate = false and carry = false: SUB A, r: subtract register r to register A
// immediate = true  and carry = true: SBC A, n: subtract 8 bit immediate n + carry flag to register A
// immediate = false and carry = true: SBC A, r: subtract register r + carry flag to register A 
inline int instr_SBX_A_r (Reg r, State &state, bool immediate, bool carry)
{
  bool add_carry = ((state.F & static_cast<Byte>(Flag::CARRY)) != 0)*carry;
  r += Byte(add_carry);
  clearAllFlags(state);
  setFlag<Flag::SUBTRACT>(state);
  handleFlags(state.A, r, state, [](Short a, Short b)->Short {return a-b;}, add_carry);
  state.A -= r;
  return 4 + 4*immediate;
}

// carry = false: SUB A, (HL): subtract content of memory at address HL into register A
// carry = true:  SBC A, (HL): subtract content of memory at address HL + carry into register A
inline int instr_SBX_A_mem_HL (State &state, bool carry)
{
  Byte r = state.memory.r(REG_HL(state));
  bool add_carry = ((state.F & static_cast<Byte>(Flag::CARRY)) != 0)*carry;
  r += Byte(add_carry);
  clearAllFlags(state);
  setFlag<Flag::SUBTRACT>(state);
  handleFlags(state.A, r, state, [](Short a, Short b)->Short {return a-b;}, add_carry);
  state.A -= r;
  return 8;
}

// immediate = true  and type = 0: AND n: and 8 bit immediate n with register A
// immediate = false and type = 0: AND r: and register r or (HL) with register A
// immediate = true  and type = 1: OR n: or 8 bit immediate n with register A
// immediate = false and type = 1: OR r: or register r or (HL) with register A
// immediate = true  and type = 2: XOR n: xor 8 bit immediate n with register A
// immediate = false and type = 2: XOR r: xor register r or (HL) with register A
inline int instr_AND_OR_XOR_A_r (Reg r, State &state, bool immediate, bool use_hl, int type)
{
  if (use_hl) {
    r = state.memory.r(REG_HL(state));
  }
  clearAllFlags(state);
  switch (type)
  {
    case 0:
      setFlag<Flag::HALF_CARRY>(state);
      state.A &= r;
      break;
    case 1:
      state.A |= r;
      break;
    case 2:
      state.A ^= r;
      break;
    default:
      return -1;
  }
  condSetFlag<Flag::ZERO>(state, state.A == 0);
  return 4 + 4*(immediate or use_hl);
}

// CP r: compare A with register, 8 bit value or (HL)
inline int instr_CP_r (Reg r, State &state, bool immediate, bool use_hl)
{
  if (use_hl) {
    r = state.memory.r(REG_HL(state));
  }
  clearAllFlags(state);
  setFlag<Flag::SUBTRACT>(state);
  handleFlags(state.A, r, state, [](Short a, Short b)->Short {return a-b;}, false);
  return 4 + 4*(immediate or use_hl);
}

// dec = false: INC r: increment register
// dec = true:  DEC r: decrement register
inline int instr_INC_DEC_r (Reg &r, State &state, bool dec)
{
  // can't do clear all as caryy flag can't be affected
  clearFlag<Flag::HALF_CARRY>(state);
  if (dec) {
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
inline int instr_INC_DEC_mem_HL (State &state, bool dec)
{
  DReg HL = REG_HL(state);
  Reg val = state.memory.r(HL);
  instr_INC_DEC_r(val, state, dec);
  state.memory.w(HL, val); // this is done to use the special write function
  return 12;
}

// ADD HL, n: add two byte register n to HL
inline int instr_ADD_HL_n (DReg reg, State &state)
{
  clearFlag<Flag::SUBTRACT>(state);
  clearFlag<Flag::HALF_CARRY>(state);
  clearFlag<Flag::CARRY>(state);
  DReg HL = REG_HL(state);
  condSetFlag<Flag::HALF_CARRY>(state, ((HL & 0x0FFF) + (reg & 0x0FFF)) > 0x0FFF);
  condSetFlag<Flag::CARRY>(state, (uint32_t(reg) + uint32_t(HL)) > 0xFFFF);
  SET_REG_HL((HL + reg), state);
  return 8;
}

// ADD SP, n: add one byte signed immediate to SP
inline int instr_ADD_SP_n (SByte n, State &state)
{
  clearAllFlags(state);
  condSetFlag<Flag::HALF_CARRY>(state, (((state.SP & 0x0F) + (n & 0x0F)) & 0xFFF0) != 0);
  condSetFlag<Flag::CARRY>(state, (((state.SP & 0xFF) + (n & 0xFF)) & 0xFF00) != 0);
  state.SP += n;
  return 16;
}

// dec = false: INC nn: increment two byte register
// dec = true:  DEC nn: decrement two byte register
inline int instr_INC_DEC_nn_16bits (Reg &reg_high, Reg &reg_low, bool dec)
{
  DReg nn = JOIN_REGS(reg_high, reg_low);
  if (dec) {
    nn--;
  } else {
    nn++;
  }
  STORE_SHORT(nn, reg_high, reg_low);
  return 8;
}

// dec = false: INC SP: increment stack pointer
// dec = true:  DEC SP: decrement stack pointer
inline int instr_INC_DEC_SP (State &state, bool dec)
{
  if (dec) {
    state.SP--;
  } else {
    state.SP++;
  }
  return 8;
}