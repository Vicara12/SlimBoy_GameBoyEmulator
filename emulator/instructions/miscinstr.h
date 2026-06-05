#pragma once

/*
* Code for several instructions that don't belong to a particular class.
*/

#include "../state.h"
#include "instructiondefines.h"


namespace gb {

// DAA: decimal adjust register A
inline int instr_DAA (State &state)
{
  bool hc_flag = getFlag<Flag::HALF_CARRY>(state);
  bool c_flag = getFlag<Flag::CARRY>(state);
  bool sub_flag = getFlag<Flag::SUBTRACT>(state);
  clearFlag<Flag::ZERO>(state);
  clearFlag<Flag::HALF_CARRY>(state);
  if (sub_flag) {
    state.A -=  c_flag*0x60;
    state.A -= hc_flag*0x06;
  } else {
    condSetFlag<Flag::CARRY>(state, state.A > 0x99);
    state.A += ( c_flag or ( state.A         > 0x99))*0x60;
    state.A += (hc_flag or ((state.A & 0x0F) > 0x09))*0x06;
  }
  condSetFlag<Flag::ZERO>(state, state.A == 0);
  return 4;
}

// NOP: no operation
inline int instr_NOP ()
{
  return 4;
}

// HALT: power down CPU until an interrupt occurs
inline int instr_HALT (State &state)
{
  state.halted = true;
  return 4;
}

// STOP: halt CPU and LCD display until button pressed
inline int instr_STOP (State &state)
{
  state.stopped = true;
  return 4;
}

// enable = false: DI: disable interrupts after instruction execution
// enable = true:  EI: enable interrupts after instruction execution
inline int instr_DI_EI (State &state, bool enable)
{
  state.ime = enable;
  return 4;
}

} // namespace gb