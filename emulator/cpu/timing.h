#pragma once

#include "state.h"
#include "interrupts.h"


namespace gb {

inline constexpr ulong EMU_SYNCH_FREQ = 128;
inline constexpr ulong EMU_SYNCH_CYCLES = CLOCK_FREQ / EMU_SYNCH_FREQ;


inline void updateTimeRegisters (State &state) {
  Byte &div = state.memory.f(Addr::DIV);
  if (state.timing.cycles_next_DIV_inc < state.timing.cycles) {
    state.timing.cycles_next_DIV_inc += 256;
    div++;
  }

  if (state.timing.enable_TIMA) {
    Byte &tima = state.memory.f(Addr::TIMA);
    const Byte tma = state.memory.f(Addr::TMA);
    while (state.timing.cycles_next_TIMA_inc < state.timing.cycles) {
      tima = (state.timing.load_tma ? tma : tima + 1);
      if (tima == 0 and not state.timing.load_tma) {
        state.timing.cycles_next_TIMA_inc += 4;
        state.timing.load_tma = true;
        setInterrupt<Interrupt::TIMER>(state);
      } else {
        state.timing.cycles_next_TIMA_inc += state.timing.cycles_div_TIMA;
        state.timing.load_tma = false;
      }
    }
  }
}


template<class InterfaceT>
inline void synchExecution (State &state, InterfaceT &interface) {
  if (state.timing.cycles < state.timing.cycles_next_synch) {
    return;
  }
  state.timing.cycles_next_synch += EMU_SYNCH_CYCLES;
  ulong t_now = interface.realTimeMicros();
  ulong delta_t = t_now - state.timing.t_last_synch;
  float delta_emu_t = float(state.timing.cycles - state.timing.cycles_last_synch)/CLOCK_FREQ*1e6;
  float emulation_rate = delta_emu_t/delta_t;
  // Update every 500 ms
  if (float(t_now - state.timing.t_last_synch) > 500e3) {
    interface.informEmuRate(emulation_rate);
    state.timing.t_last_synch = t_now;
    state.timing.cycles_last_synch = state.timing.cycles;
  }
  float diff_ms = float(delta_emu_t/state.target_speed - delta_t)/1e3;
  if (diff_ms > 5) {
    interface.sleepMillis(diff_ms);
  }
}

} // namespace gb