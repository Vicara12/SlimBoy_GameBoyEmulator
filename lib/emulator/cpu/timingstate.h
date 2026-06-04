#pragma once

#include "types.h"

inline constexpr ulong CLOCK_FREQ = 4194304;


inline ulong getDivFromTAC (Byte value_TAC) {
  static constexpr ulong values [] = {(1 << 10), (1 << 4), (1 << 6), (1 << 8)};
  return values[value_TAC & 0x03];
}


struct Timing {
  ulong cycles_next_DIV_inc = 0;
  ulong cycles_next_TIMA_inc = 0;
  ulong cycles_div_TIMA = getDivFromTAC(0);
  bool load_tma = false;
  bool enable_TIMA = false;
  ulong t_last_synch = 0;
  ulong cycles_last_synch = 0;
  ulong cycles_next_synch = 0;
  ulong t_last_emu_rate = 0;
  ulong cycles = 0; // Total execution cycles (execution clock)
};