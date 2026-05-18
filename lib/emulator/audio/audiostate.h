#pragma once

#include <vector>
#include <limits>
#include "types.h"
#include "generaldefines.h"


#define AUDIO_BUFFER_SIZE 1024
#define SAMPLE_RATE       (AUDIO_BUFFER_SIZE*32) // 32768 samples/sec
#define PUSH_AUDIO_EACH   (CLOCK_FREQ / SAMPLE_RATE)

enum class AudioChannel : int {
  CH1 = 1,
  CH2 = 2,
  CH3 = 3,
  CH4 = 4
};


#define NR10_REGISTER 0xFF10 // Channel 1 sweep
#define NR11_REGISTER 0xFF11 // Channel 1 length timer & duty cycle
#define NR12_REGISTER 0xFF12 // Channel 1 volume & envelope
#define NR13_REGISTER 0xFF13 // Channel 1 period low
#define NR14_REGISTER 0xFF14 // Channel 1 period high & control

#define NR21_REGISTER 0xFF16 // Channel 2 length timer & duty cycle
#define NR22_REGISTER 0xFF17 // Channel 2 volume & envelope
#define NR23_REGISTER 0xFF18 // Channel 2 period low
#define NR24_REGISTER 0xFF19 // Channel 2 period high & control

#define NR30_REGISTER 0xFF1A // Channel 3 DAC enable
#define NR31_REGISTER 0xFF1B // Channel 3 length timer
#define NR32_REGISTER 0xFF1C // Channel 3 output level
#define NR33_REGISTER 0xFF1D // Channel 3 period low
#define NR34_REGISTER 0xFF1E // Channel 3 period high & control
#define WAVE_PAT_REGISTER 0xFF30 // Channel 3 wave pattern RAM until 0xFF3F

#define NR41_REGISTER 0xFF20 // Channel 4 length timer
#define NR42_REGISTER 0xFF21 // Channel 4 volume & envelope
#define NR43_REGISTER 0xFF22 // Channel 4 frequency & randomness
#define NR44_REGISTER 0xFF23 // Channel 4 control

#define NR50_REGISTER 0xFF24 // Master volume & VIN panning
#define NR51_REGISTER 0xFF25 // Sound panning
#define NR52_REGISTER 0xFF26 // Audio master control


constexpr std::array<std::array<Byte,8>,4> AUDIO_SEQUENCER {{
  {0,0,0,0,0,0,0,1}, // 00: 12.5% duty
  {1,0,0,0,0,0,0,1}, // 01: 25% duty
  {1,0,0,0,0,1,1,1}, // 10: 50% duty
  {0,1,1,1,1,1,1,0}  // 11: 75% duty
}};


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
  bool NRX4_written = true; // Control register
  bool NRX2_written = true; // Volume and envelope control
  bool on = false;
};


struct AudioState {
  ulong cycles_next_push = 0;
  std::vector<Byte> audio_buffer_l;
  std::vector<Byte> audio_buffer_r;
  PulseChannelData ch1;
  PulseChannelData ch2;
  bool registers_cleared = false;
};