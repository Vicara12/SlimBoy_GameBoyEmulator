#pragma once

#include "state.h"


namespace gb {

enum class Interrupt : Byte {
  VBLANK = 0x01,
  LCD    = 0x02,
  TIMER  = 0x04,
  SRIAL  = 0x08,
  JOYPAD = 0x10
};


template<Interrupt interrupt>
inline void setInterrupt (State &state) {
  state.memory.f(Addr::IF) |= static_cast<Byte>(interrupt);
}


inline void checkAndCallInterrupt (State &state) {
  Byte &if_reg = state.memory.f(Addr::IF);
  Byte &ie_reg = state.memory.f(Addr::IE);
  Byte active_interrupts = (if_reg & ie_reg) & 0x1F;
  if (state.halted and active_interrupts != 0) {
    state.halted = false;
  }
  
  // Check first if there is any interrupt or if they are enabled by the interrupt master enable
  if (active_interrupts == 0 or not state.ime) {
    return;
  }

  // Counting zeros to the right means finding the most primary active interrupt
  int interrupt_idx = __builtin_ctz(static_cast<unsigned int>(active_interrupts));
  state.ime = false;
  if_reg &= ~(1 << interrupt_idx);
  state.memory.f(--state.SP) = state.PC >> 8;
  state.memory.f(--state.SP) = state.PC & 0xFF;
  state.PC = 0x0040 + (interrupt_idx * 8);
  state.timing.cycles += 20;
  return;
}

} // namespace gb