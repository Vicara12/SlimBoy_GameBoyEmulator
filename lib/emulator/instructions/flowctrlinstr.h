#pragma once

/*
* Code for all the flow control instructions
*/

#include "state.h"
#include "cpu/interrupts.h"

// JP nn: jump to 16 bit address nn
inline int instr_JP_nn (Byte lsb, Byte msb, State &state)
{
  state.PC = JOIN_REGS(msb, lsb);
  return 16;
}

// JP cc, nn: conditionally jump to 16 bit address nn
template<Flag flag>
inline int instr_JP_cc_nn (Byte lsb, Byte msb, bool set, State &state)
{
  if (set == ((state.F & static_cast<Byte>(flag)) != 0)) {
    instr_JP_nn(lsb, msb, state);
    return 16;
  }
  return 12;
}

// JP (HL): jump to address contained in HL
inline int instr_JP_HL (State &state)
{
  state.PC = REG_HL(state);
  return 4;
}

// JR n: add 8 bit signed immediate n to current address and jump to it
inline int instr_JR_n (SByte n, State &state)
{
  state.PC += n;
  return 12;
}

// JP cc, nn: conditionally add 8 bit signed immediate n to PC
template<Flag flag>
inline int instr_JR_cc_n (SByte n, bool set, State &state)
{
  if (set == ((state.F & static_cast<Byte>(flag)) != 0)) {
    instr_JR_n(n, state);
    return 12;
  }
  return 8;
}

// CALL nn: push address of next instruction onto stack and jump to address nn
inline int instr_CALL_nn (Byte lsb, Byte msb, State &state)
{
  state.memory.w(--state.SP, state.PC >> 8);   // push msb of PC
  state.memory.w(--state.SP, state.PC & 0xFF); // push lsb of PC
  state.PC = JOIN_REGS(msb, lsb);
  return 24;
}

// CALL cc, nn: call address nn if the condition cc is true
template<Flag flag>
inline int instr_CALL_cc_nn (Byte lsb, Byte msb, bool set, State &state)
{
  if (set == ((state.F & static_cast<Byte>(flag)) != 0)) {
    instr_CALL_nn(lsb, msb, state);
    return 24;
  }
  return 12;
}

// RST n: push present address into stack and jump to address 0x0000 + n
inline int instr_RST_n (Byte addr, State &state)
{
  instr_CALL_nn(addr, 0x00, state);
  return 16;
}

// RET: pop two bytes from stack and jump to that address
inline int instr_RET (State &state)
{
  Byte addr_lsb = state.memory.r(state.SP++);
  Byte addr_msb = state.memory.r(state.SP++);
  state.PC = JOIN_REGS(addr_msb, addr_lsb);
  return 16;
}

// RET: pop two bytes from stack and jump to that address if condition is met
template<Flag flag>
inline int instr_RET_cc (bool set, State &state)
{
  if (set == ((state.F & static_cast<Byte>(flag)) != 0)) {
    instr_RET(state);
    return 20;
  }
  return 8;
}

// RETI: RET + enable interrupts
inline int instr_RETI (State &state)
{
  state.ime = true; // Activate interrupts
  return instr_RET(state);
}