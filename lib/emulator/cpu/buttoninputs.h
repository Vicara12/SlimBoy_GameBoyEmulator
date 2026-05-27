#pragma once

#include "types.h"
#include "state.h"
#include "interrupts.h"


enum class Button : Byte {
  Right  = 0x01,
  Left   = 0x02,
  Up     = 0x04,
  Down   = 0x08,
  A      = 0x10,
  B      = 0x20,
  Select = 0x40,
  Start  = 0x80
};


template<class InterfaceT>
inline void updateButtons (ulong n_instrs, State &state, InterfaceT &interface)
{
  Byte &p1_reg = state.memory.f(Addr::P1);
  Byte old_p1_reg = p1_reg;
  Byte button_inputs = interface.readButtons();
  Byte input = 0xF0;
  // Low at bit 5 indicates A/B/Sel/Start read
  if ((old_p1_reg & 0x20) == 0) {
    input |= (button_inputs >> 4);
  }
  // Low at bit 4 indicates U/D/L/R read
  if ((old_p1_reg & 0x10) == 0) {
    input |= (button_inputs & 0x0F);
  }
  // Button pressed means LOW (0) and vice versa
  p1_reg = (old_p1_reg & 0x30) | (~input);

  // If any change in button pressed activate interrupt and exit stop state (if in it)
  if (((old_p1_reg ^ p1_reg) & 0x0F) != 0) {
    setInterrupt<Interrupt::JOYPAD>(state);
    state.stopped = false;
  }
}