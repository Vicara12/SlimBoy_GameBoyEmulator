#pragma once

#include "state.h"
#include "interrupts.h"

inline constexpr ulong EMU_SYNCH_FREQ = 128;
inline constexpr ulong EMU_SYNCH_CYCLES = CLOCK_FREQ / EMU_SYNCH_FREQ;


inline void updateTimeRegisters (State &state) {
  if (state.memory.specialAddrWritten(Addr::DIV)) {
    state.timing.cycles_last_DIV = state.cycles;
  }
  if (state.memory.specialAddrWritten(Addr::TAC)) {
    Byte tac_val = state.memory.f(Addr::TAC);
    state.timing.enable_TIMA = (tac_val & 0x04);
    state.timing.cycles_div_TIMA = getDivFromTAC(tac_val);
  }
  state.memory.f(Addr::DIV) = (state.cycles - state.timing.cycles_last_DIV)/256;
  if (state.timing.enable_TIMA) {
    Byte &tima_val = state.memory.f(Addr::TIMA);
    Byte &tma_val = state.memory.f(Addr::TMA);
    Byte old_TIMA = tima_val;
    ulong t_value_TIMA = (state.cycles - state.timing.cycles_last_TIMA)/state.timing.cycles_div_TIMA;
    tima_val = t_value_TIMA + tma_val;
    // Detect TIMA overflow
    if (old_TIMA > tima_val) {
      // Account for overflowed cycles
      state.timing.cycles_last_TIMA = state.cycles - tima_val*state.timing.cycles_div_TIMA;
      tima_val += tma_val;
      setInterrupt<Interrupt::TIMER>(state);
    }
  }
}


template<class InterfaceT>
inline void synchExecution (State &state, InterfaceT &interface) {
  if (state.cycles < state.timing.cycles_next_synch) {
    return;
  }
  state.timing.cycles_next_synch += EMU_SYNCH_CYCLES;
  ulong t_now = interface.realTimeMicros();
  ulong delta_t = t_now - state.timing.t_last_synch;
  float delta_emu_t = float(state.cycles - state.timing.cycles_last_synch)/CLOCK_FREQ*1e6;
  float emulation_rate = delta_emu_t/delta_t;
  // Update every 500 ms
  if (float(t_now - state.timing.t_last_synch) > 500e3) {
    interface.informEmuRate(emulation_rate);
    state.timing.t_last_synch = t_now;
    state.timing.cycles_last_synch = state.cycles;
  }
  float diff_ms = float(delta_emu_t/state.target_speed - delta_t)/1e3;
  if (diff_ms > 5) {
    interface.sleepMillis(diff_ms);
  }
}