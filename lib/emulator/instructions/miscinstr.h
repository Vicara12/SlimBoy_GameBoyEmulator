#pragma once

/*
* Code for several instructions that don't belong to a particular class.
*/

#include "state.h"

// DAA: decimal adjust register A
inline int instr_DAA (State *state)
{
  bool hc_flag = GET_HALF_CARRY_FLAG(state);
  bool c_flag = GET_CARRY_FLAG(state);
  bool sub_flag = GET_SUBTRACT_FLAG(state);
  RESET_ZERO_FLAG(state);
  RESET_HALF_CARRY_FLAG(state);
  if (sub_flag) {
    state->A -=  c_flag*0x60;
    state->A -= hc_flag*0x06;
  } else {
    COND_SET_CARRY_FLAG(state, state->A > 0x99);
    state->A += ( c_flag or ( state->A         > 0x99))*0x60;
    state->A += (hc_flag or ((state->A & 0x0F) > 0x09))*0x06;
  }
  COND_SET_ZERO_FLAG(state, state->A == 0);
  return 4;
}

// NOP: no operation
inline int instr_NOP ()
{
  return 4;
}

// HALT: power down CPU until an interrupt occurs
inline int instr_HALT (State *state)
{
  state->halted = true;
  return 4;
}

// STOP: halt CPU and LCD display until button pressed
inline int instr_STOP (State *state)
{
  state->stopped = true;
  return 4;
}

// enable = false: DI: disable interrupts after instruction execution
// enable = true:  EI: enable interrupts after instruction execution
inline int instr_DI_EI (State *state, bool enable)
{
  state->ime = enable;
  return 4;
}