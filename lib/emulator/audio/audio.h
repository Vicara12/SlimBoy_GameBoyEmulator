#pragma once

#include <algorithm>
#include "audiostate.h"
#include "interface.h"
#include "state.h"
#include "types.h"


// General audio registers
#define NR52_AUDIO_ENABLED(state) ((state->memory[NR52_REGISTER] & 0x80) != 0)

template<AudioChannel channel, bool set_on>
inline void NR52_SET_CHX_ON_OFF (State *state) {
  constexpr Byte mask = (1 << (static_cast<int>(channel) - 1));

  if constexpr (set_on) {
    state->memory[NR52_REGISTER] |= mask;
  } else {
    state->memory[NR52_REGISTER] &= ~mask;
  }
}

template<AudioChannel channel>
inline bool NR51_PAN_CHX_R (State *state) {
  constexpr Byte mask = (0x01 << (static_cast<int>(channel) - 1));
  return (state->memory[NR51_REGISTER] & mask) != 0;
}

template<AudioChannel channel>
inline bool NR51_PAN_CHX_L (State *state) {
  constexpr Byte mask = (0x10 << (static_cast<int>(channel) - 1));
  return (state->memory[NR51_REGISTER] & mask) != 0;
}

#define NR50_R_VOL(state) (state->memory[NR50_REGISTER] & 0x07)
#define NR50_L_VOL(state) ((state->memory[NR50_REGISTER] & 0x70) >> 4)


// Channel specific functionalities
#define NR10_CH1_SWEEP_PACE(state)      ((state->memory[NR10_REGISTER] & 0x70) >> 4)
#define NR10_CH1_SWEEP_DIRECTION(state) ((state->memory[NR10_REGISTER] & 0x08) != 0)
#define NR10_CH1_SWEEP_STEP(state)      (state->memory[NR10_REGISTER] & 0x07)

template<AudioChannel channel>
inline Byte CHX_DUTY (State *state) {
  static_assert(
    channel == AudioChannel::CH1 or channel == AudioChannel::CH2,
    "Only channels 1 and 2 have a duty cycle"
  );
  constexpr Short reg = (channel == AudioChannel::CH1 ? NR11_REGISTER : NR21_REGISTER);
  return (state->memory[reg] & 0xC0) >> 6;
}

template<AudioChannel channel>
inline Byte CHX_INITIAL_LEN_TIMER (State *state) {
  if constexpr      (channel == AudioChannel::CH1) return state->memory[NR11_REGISTER] & 0x3F;
  else if constexpr (channel == AudioChannel::CH2) return state->memory[NR21_REGISTER] & 0x3F;
  else if constexpr (channel == AudioChannel::CH3) return state->memory[NR31_REGISTER];
  else                                             return state->memory[NR41_REGISTER] & 0x3F;
}


template <AudioChannel channel>
inline Byte CHX_VOL (State *state) {
  static_assert(channel != AudioChannel::CH3, "Channel 3 has no VOL value");
  if constexpr      (channel == AudioChannel::CH1) return state->memory[NR12_REGISTER] >> 4;
  else if constexpr (channel == AudioChannel::CH2) return state->memory[NR22_REGISTER] >> 4;
  else                                             return state->memory[NR42_REGISTER] >> 4;
}

template <AudioChannel channel>
inline bool CHX_ENV_DIR (State *state) {
  static_assert(channel != AudioChannel::CH3, "Channel 3 has no ENV DIR value");
  if constexpr      (channel == AudioChannel::CH1) return (state->memory[NR12_REGISTER] & 0x08) != 0;
  else if constexpr (channel == AudioChannel::CH2) return (state->memory[NR22_REGISTER] & 0x08) != 0;
  else                                             return (state->memory[NR42_REGISTER] & 0x08) != 0;
}

template <AudioChannel channel>
inline Byte CHX_ENVELOPE_PACE (State *state) {
  static_assert(channel != AudioChannel::CH3, "Channel 3 has no SWEEP PACE value");
  if constexpr      (channel == AudioChannel::CH1) return state->memory[NR12_REGISTER] & 0x07;
  else if constexpr (channel == AudioChannel::CH2) return state->memory[NR22_REGISTER] & 0x07;
  else                                             return state->memory[NR42_REGISTER] & 0x07;
}

template <AudioChannel channel>
inline bool CHX_ZERO_VOL_ENV (State *state) {
  static_assert(channel != AudioChannel::CH3, "Channel 3 has no ENV DIR value");
  if constexpr      (channel == AudioChannel::CH1) return (state->memory[NR12_REGISTER] & 0xF8) == 0;
  else if constexpr (channel == AudioChannel::CH2) return (state->memory[NR22_REGISTER] & 0xF8) == 0;
  else                                             return (state->memory[NR42_REGISTER] & 0xF8) == 0;
}


template <AudioChannel channel>
inline bool CHX_TRIGGERED (State *state) {
  if constexpr      (channel == AudioChannel::CH1) return (state->memory[NR14_REGISTER] & 0x80) != 0;
  else if constexpr (channel == AudioChannel::CH2) return (state->memory[NR24_REGISTER] & 0x80) != 0;
  else if constexpr (channel == AudioChannel::CH3) return (state->memory[NR34_REGISTER] & 0x80) != 0;
  else                                             return (state->memory[NR44_REGISTER] & 0x80) != 0;
}

template <AudioChannel channel>
inline bool CHX_LEN_ENABLED (State *state) {
  if constexpr      (channel == AudioChannel::CH1) return (state->memory[NR14_REGISTER] & 0x40) != 0;
  else if constexpr (channel == AudioChannel::CH2) return (state->memory[NR24_REGISTER] & 0x40) != 0;
  else if constexpr (channel == AudioChannel::CH3) return (state->memory[NR34_REGISTER] & 0x40) != 0;
  else                                             return (state->memory[NR44_REGISTER] & 0x40) != 0;
}

template <AudioChannel channel>
inline Short GET_CHX_PERIOD (State *state) {
  static_assert(channel != AudioChannel::CH4, "Channel 4 has no period");
  if constexpr (channel == AudioChannel::CH1)
    return (Short(state->memory[NR14_REGISTER] & 0x07) << 8) | Short(state->memory[NR13_REGISTER]);
  else if constexpr (channel == AudioChannel::CH2)
    return (Short(state->memory[NR24_REGISTER] & 0x07) << 8) | Short(state->memory[NR23_REGISTER]);
  else
    return (Short(state->memory[NR34_REGISTER] & 0x07) << 8) | Short(state->memory[NR33_REGISTER]);
}

template<AudioChannel channel>
inline void SET_CHX_PERIOD (State *state, Short new_value) {
  static_assert(channel != AudioChannel::CH4, "Channel 4 has no period");
  if constexpr (channel == AudioChannel::CH1) {
    state->memory[NR13_REGISTER] = new_value;
    state->memory[NR14_REGISTER] &= 0xF8;
    state->memory[NR14_REGISTER] |= (new_value >> 8) & 0x07;
  }
  else if constexpr (channel == AudioChannel::CH2) {
    state->memory[NR23_REGISTER] = new_value;
    state->memory[NR24_REGISTER] &= 0xF8;
    state->memory[NR24_REGISTER] |= (new_value >> 8) & 0x07;
  }
  else {
    state->memory[NR33_REGISTER] = new_value;
    state->memory[NR34_REGISTER] &= 0xF8;
    state->memory[NR34_REGISTER] |= (new_value >> 8) & 0x07;
  }
}

#define NR30_CH3_DAC_ENABLED(state) ((state->memory[NR30_REGISTER] & 0x80) != 0)
#define NR32_CH3_VOL(state)         ((state->memory[NR32_REGISTER] & 0x60) >> 5)

#define NR43_CH4_SHORT_MODE(state)  ((state->memory[NR43_REGISTER] & 0x08) != 0)


inline void resetAudioBuffers (AudioState &audio_state) {
  audio_state.aud_pkg = AudioPacket();
  audio_state.aud_pkg.buffer_l.clear();
  audio_state.aud_pkg.buffer_r.clear();
  audio_state.aud_pkg.buffer_l.reserve(AUDIO_BUFFER_SIZE);
  audio_state.aud_pkg.buffer_r.reserve(AUDIO_BUFFER_SIZE);
}


template<AudioChannel channel>
inline void pulseChannelState (State *state, PulseChannelData &ch_data) {
  if (ch_data.NRX4_written or ch_data.NRX2_written) {
    if (ch_data.NRX4_written) {
      if (CHX_LEN_ENABLED<channel>(state))
        ch_data.auto_off_clk = state->cycles + (64 - CHX_INITIAL_LEN_TIMER<channel>(state)) * (CLOCK_FREQ/256);
      else
        ch_data.auto_off_clk = std::numeric_limits<ulong>::max();
    }
    
    if (CHX_ZERO_VOL_ENV<channel>(state)) {
      ch_data.on = false;
      NR52_SET_CHX_ON_OFF<channel, false>(state);
    }
    else if (CHX_TRIGGERED<channel>(state)) {
      state->memory[NR14_REGISTER] &= 0x7F; // Clear trigger bit
      ch_data.on = true;
      // Period overflow will be detected, sequence_idx increased to 0 and that will trigger CH1 config
      ch_data.period_overflow_clk = state->cycles;
      ch_data.envelope_pace = CHX_ENVELOPE_PACE<channel>(state);
      ch_data.envelope_next_clk = state->cycles + ch_data.envelope_pace*(CLOCK_FREQ/64);
      ch_data.sequence_idx = 7;
      ch_data.volume = CHX_VOL<channel>(state) * 17; // Scale [0,15] -> [0,255]
      if constexpr (channel == AudioChannel::CH1)
        ch_data.last_pace = NR10_CH1_SWEEP_PACE(state);
      NR52_SET_CHX_ON_OFF<channel, true>(state);
    }
    ch_data.NRX4_written = false;
    ch_data.NRX2_written = false;
  }
}


template<AudioChannel channel, typename ChDataT>
inline void checkNewEnvelopeVolume (State *state, ChDataT &ch_data) {
  if (ch_data.envelope_pace != 0 and state->cycles >= ch_data.envelope_next_clk) {
    ch_data.envelope_next_clk += ch_data.envelope_pace*(CLOCK_FREQ/64);
    if (CHX_ENV_DIR<channel>(state)) {
      if (ch_data.volume != 255)
        ch_data.volume += 17;
    } else {
      if (ch_data.volume != 0)
        ch_data.volume -= 17;
    }
  }
}


inline void checkNewPace (State *state, PulseChannelData &ch_data) {
  Short new_pace = NR10_CH1_SWEEP_PACE(state);
  if (state->cycles >= ch_data.pace_change_clk) {
    ch_data.pace_change_clk += new_pace * (CLOCK_FREQ/128);
    ch_data.last_pace = new_pace;
    Short period = GET_CHX_PERIOD<AudioChannel::CH1>(state);
    Short change = period >> NR10_CH1_SWEEP_STEP(state);
    if (NR10_CH1_SWEEP_DIRECTION(state)) {
      if (period >= change)
        SET_CHX_PERIOD<AudioChannel::CH1>(state, period-change);
      else
        SET_CHX_PERIOD<AudioChannel::CH1>(state, 0);
    } else {
      Short new_period = period + change;
      if (new_period >= 0x0800) {
        ch_data.on = false;
      } else {
        SET_CHX_PERIOD<AudioChannel::CH1>(state, new_period);
      }
    }
  }
}


template<AudioChannel channel>
inline void processPulseChannel (
  State *state,
  PulseChannelData &ch_data,
  int &sample_l,
  int &sample_r
) {
  pulseChannelState<channel>(state, ch_data);

  if (state->cycles >= ch_data.auto_off_clk) {
    ch_data.auto_off_clk = std::numeric_limits<ulong>::max();
    ch_data.on = false;
  }

  if (not ch_data.on) {
    return;
  }

  if constexpr (channel == AudioChannel::CH1) {
    if (NR10_CH1_SWEEP_PACE(state) == 0) {
      ch_data.pace_change_clk = std::numeric_limits<ulong>::max();
    }
    else if (ch_data.last_pace == 0) {
      ch_data.pace_change_clk = state->cycles;
    }
  }

  if (state->cycles >= ch_data.period_overflow_clk) {
    ulong reminder = state->cycles - ch_data.period_overflow_clk;
    ch_data.period_overflow_clk = state->cycles + 4 * (0x0800 - GET_CHX_PERIOD<channel>(state)) - reminder;
    ch_data.sequence_idx = (ch_data.sequence_idx + 1)%8;
    // Configuration is only changed at the begining of an audio sequencer
    if (ch_data.sequence_idx == 0) {
      ch_data.duty_idx = CHX_DUTY<channel>(state);
    }
    checkNewEnvelopeVolume<channel, PulseChannelData>(state, ch_data);
    // Check for new pace
    if constexpr (channel == AudioChannel::CH1) {
      checkNewPace(state, ch_data);
    }
    ch_data.signal_value = AUDIO_SEQUENCER[ch_data.duty_idx][ch_data.sequence_idx] * ch_data.volume;
  }

  if (NR51_PAN_CHX_L<channel>(state)) {
    sample_l += ch_data.signal_value;
  }
  if (NR51_PAN_CHX_R<channel>(state)) {
    sample_r += ch_data.signal_value;
  }
}


inline Byte waveChannelVolumeShift (State *state) {
  switch (NR32_CH3_VOL(state)) {
    case 0b00: return 8; //   0% volume
    case 0b01: return 0; // 100% volume
    case 0b10: return 1; //  50% volume
    case 0b11: return 2; //  25% volume
  }
  return 0; // Impossible
}


inline void processWaveChannel (State *state, int &sample_l, int &sample_r) {
  WaveChannelData &ch3 = state->audio.ch3;
  if (not NR30_CH3_DAC_ENABLED(state)) {
    ch3.on = false;
  }
  else if (ch3.NR34_written) {
    if (CHX_TRIGGERED<AudioChannel::CH3>(state)) {
      state->memory[NR34_REGISTER] &= 0x7F; // Clear trigger bit
      ch3.on = true;
      ch3.period_overflow_clk = state->cycles;
      ch3.ram_idx = 0;
    }
    if (CHX_LEN_ENABLED<AudioChannel::CH3>(state))
      ch3.auto_off_clk = state->cycles + (256 - CHX_INITIAL_LEN_TIMER<AudioChannel::CH3>(state)) * (CLOCK_FREQ/256);
    else
      ch3.auto_off_clk = std::numeric_limits<ulong>::max();
  }
  ch3.NR34_written = false;

  if (state->cycles >= ch3.auto_off_clk) {
    ch3.auto_off_clk = std::numeric_limits<ulong>::max();
    ch3.on = false;
  }

  if (not ch3.on) {
    return;
  }

  if (state->cycles >= ch3.period_overflow_clk) {
    ch3.period_overflow_clk += 2 * (0x0800 - GET_CHX_PERIOD<AudioChannel::CH3>(state));
    ch3.ram_idx = (ch3.ram_idx+1)%32;
    Byte mem_val = state->memory[WAVE_PAT_REGISTER + ch3.ram_idx/2];
    Byte wave_val = ((ch3.ram_idx % 2) == 0 ? (mem_val >> 4) : (mem_val & 0x0F));
    ch3.signal_value = (((255/15) * wave_val) >> waveChannelVolumeShift(state));
  }

  if (NR51_PAN_CHX_L<AudioChannel::CH3>(state)) {
    sample_l += ch3.signal_value;
  }
  if (NR51_PAN_CHX_R<AudioChannel::CH3>(state)) {
    sample_r += ch3.signal_value;
  }
}


inline ulong noiseShiftCycles (State *state) {
  Byte nr43_val = state->memory[NR43_REGISTER];
  ulong shift = (nr43_val >> 4);
  if (shift > 13) {
    return std::numeric_limits<ulong>::max();
  }
  ulong divider = (nr43_val & 0x07);
  ulong main_freq = 262144;
  if (divider == 0) {
    divider = 1;
    main_freq *= 2;
  }
  return (CLOCK_FREQ / main_freq) * (divider << shift);
}


inline Byte cycleLFSR (State *state) {
  Short lfsr = state->audio.ch4.lfsr;
  Short new_bit = 1 - (((lfsr & 0x02) != 0) ^ (lfsr & 0x01)); // XNOR of bits 0 and 1
  lfsr &= ~(1 << 15);
  lfsr |=  (new_bit << 15);
  if (NR43_CH4_SHORT_MODE(state)) {
    lfsr &= ~(1 << 7);
    lfsr |=  (new_bit << 7);
  }
  state->audio.ch4.lfsr = (lfsr >> 1);
  return lfsr & 1;
}


inline void noiseChannelState(State *state, NoiseChannelData &ch4) {
  if (CHX_ZERO_VOL_ENV<AudioChannel::CH4>(state)) {
    ch4.on = false;
  }
  else if (ch4.NR44_written) {
    if (CHX_TRIGGERED<AudioChannel::CH4>(state)) {
      state->memory[NR44_REGISTER] &= 0x7F; // Clear trigger bit
      ch4.on = true;
      ch4.volume = CHX_VOL<AudioChannel::CH4>(state) * 17; // Scale [0,15] -> [0,255]
      ch4.envelope_pace = CHX_ENVELOPE_PACE<AudioChannel::CH4>(state);
      ch4.envelope_next_clk = state->cycles + ch4.envelope_pace*(CLOCK_FREQ/64);
      ch4.lfsr = 0;
      ch4.NR43_written = true; // This will trigger LFSR frequency calculation below
    }
    if (CHX_LEN_ENABLED<AudioChannel::CH4>(state))
      ch4.auto_off_clk = state->cycles + (64 - CHX_INITIAL_LEN_TIMER<AudioChannel::CH4>(state)) * (CLOCK_FREQ/256);
    else
      ch4.auto_off_clk = std::numeric_limits<ulong>::max();
  }
  if (ch4.NR43_written) {
    ch4.NR43_written = false;
    ch4.noise_shift_cycles = noiseShiftCycles(state);
    ch4.period_overflow_clk = state->cycles;
  }
  ch4.NR44_written = false;

  if (state->cycles >= ch4.auto_off_clk) {
    ch4.auto_off_clk = std::numeric_limits<ulong>::max();
    ch4.on = false;
  }
}


inline void processNoiseChannel(State *state, int &sample_l, int &sample_r) {
  NoiseChannelData &ch4 = state->audio.ch4;
  noiseChannelState(state, ch4);

  if (not ch4.on) {
    return;
  }

  if (state->cycles >= ch4.period_overflow_clk) {
    ulong reminder = state->cycles - ch4.period_overflow_clk;
    ch4.period_overflow_clk = state->cycles + ch4.noise_shift_cycles - reminder;
    checkNewEnvelopeVolume<AudioChannel::CH4, NoiseChannelData>(state, ch4);
    ch4.signal_value = cycleLFSR(state) * ch4.volume;
  }

  if (NR51_PAN_CHX_L<AudioChannel::CH4>(state)) {
    sample_l += ch4.signal_value;
  }
  if (NR51_PAN_CHX_R<AudioChannel::CH4>(state)) {
    sample_r += ch4.signal_value;
  }
}


inline void clearAudioRegs (State *state)
{
  for (Short addr = 0xFF10; addr < 0xFF15; addr++)
    state->memory[addr] = 0x00;
  for (Short addr = 0xFF16; addr < 0xFF1F; addr++)
    state->memory[addr] = 0x00;
  for (Short addr = 0xFF20; addr < 0xFF25; addr++)
    state->memory[addr] = 0x00;
}


template<class InterfaceT>
inline void updateAudio (State *state, InterfaceT &interface) {
  while (state->cycles >= state->audio.cycles_next_push) {
    state->audio.cycles_next_push += PUSH_AUDIO_EACH;

    if (not NR52_AUDIO_ENABLED(state)) {
      if (not state->audio.registers_cleared) {
        state->audio.registers_cleared = true;
        clearAudioRegs(state);
      }
      continue;
    } else {
      state->audio.registers_cleared = false;
    }

    int sample_l = 0;
    int sample_r = 0;

    processPulseChannel<AudioChannel::CH1>(state, state->audio.ch1, sample_l, sample_r);
    processPulseChannel<AudioChannel::CH2>(state, state->audio.ch2, sample_l, sample_r);
    processWaveChannel(state, sample_l, sample_r);
    processNoiseChannel(state, sample_l, sample_r);

    // Control channel volume and add them to buffer
    // We divide by 8 because volume goes from 0+1 to 7+1 and by 4 because there are 4 channels
    state->audio.aud_pkg.buffer_l.push_back(int(NR50_L_VOL(state) + 1)*sample_l/(8*4));
    state->audio.aud_pkg.buffer_r.push_back(int(NR50_R_VOL(state) + 1)*sample_r/(8*4));


    // If buffer has enough data, send it to be played through the speakers
    if (state->audio.aud_pkg.buffer_l.size() >= AUDIO_BUFFER_SIZE) {
      interface.playAudio(std::move(state->audio.aud_pkg));
      resetAudioBuffers(state->audio);
    }
  }
}