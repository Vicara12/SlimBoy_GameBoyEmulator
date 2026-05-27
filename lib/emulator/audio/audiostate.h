#pragma once

#include <vector>
#include <limits>
#include "types.h"
#include "cpu/timingstate.h"


inline constexpr ulong SAMPLE_RATE       = 32768;
inline constexpr ulong AUDIO_UPDATE_FREQ = 32;
inline constexpr ulong AUDIO_BUFFER_SIZE = SAMPLE_RATE / AUDIO_UPDATE_FREQ;
inline constexpr ulong PUSH_AUDIO_EACH   = CLOCK_FREQ / SAMPLE_RATE;


enum class AudioChannel : int {
  CH1 = 1,
  CH2 = 2,
  CH3 = 3,
  CH4 = 4
};


constexpr std::array<std::array<Byte,8>,4> AUDIO_SEQUENCER {{
  {0,0,0,0,0,0,0,1}, // 00: 12.5% duty
  {1,0,0,0,0,0,0,1}, // 01: 25% duty
  {1,0,0,0,0,1,1,1}, // 10: 50% duty
  {0,1,1,1,1,1,1,0}  // 11: 75% duty
}};


struct AudioPacket {
  std::vector<Byte> buffer_l;
  std::vector<Byte> buffer_r;
};


struct PulseChannelData {
  ulong period_overflow_clk = 0;
  ulong envelope_next_clk = 0;
  ulong auto_off_clk = std::numeric_limits<ulong>::max();
  ulong pace_change_clk = std::numeric_limits<ulong>::max();
  Byte signal_value = 0;
  Byte duty_idx = 0;
  Byte sequence_idx = 0;
  Byte volume = 0;
  Byte envelope_pace = 0;
  Byte last_pace = 0;
  bool on = false;
};


struct WaveChannelData {
  ulong period_overflow_clk = 0;
  ulong auto_off_clk = std::numeric_limits<ulong>::max();
  Byte ram_idx = 0;
  Byte signal_value = 0;
  bool on = false;
};


struct NoiseChannelData {
  ulong noise_shift_cycles = 0;
  ulong period_overflow_clk = 0;
  ulong auto_off_clk = std::numeric_limits<ulong>::max();
  ulong envelope_next_clk = 0;
  Short lfsr = 0;
  Byte signal_value = 0;
  Byte volume = 0;
  Byte envelope_pace = 0;
  bool on = false;
};


struct AudioState {
  ulong cycles_next_push = 0;
  AudioPacket aud_pkg;
  PulseChannelData ch1;
  PulseChannelData ch2;
  WaveChannelData  ch3;
  NoiseChannelData ch4;
  bool registers_cleared = false;
};