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


inline void resetAudioBuffers (AudioState &audio_state) {
  audio_state.audio_buffer_l.clear();
  audio_state.audio_buffer_r.clear();
  audio_state.audio_buffer_l.reserve(AUDIO_BUFFER_SIZE);
  audio_state.audio_buffer_r.reserve(AUDIO_BUFFER_SIZE);
}


template<AudioChannel channel>
inline void channelState (State *state, PulseChannelData &ch_data) {
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


template<AudioChannel channel>
inline void checkNewEnvelopeVolume (State *state, PulseChannelData &ch_data) {
  if (ch_data.envelope_pace != 0 and state->cycles >= ch_data.envelope_next_clk) {
    ch_data.envelope_next_clk += ch_data.envelope_pace*(CLOCK_FREQ/64);
    if ( CHX_ENV_DIR<channel>(state)) {
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
  channelState<channel>(state, ch_data);

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
    checkNewEnvelopeVolume<channel>(state, ch_data);
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


inline void clearAudioRegs (State *state)
{
  for (Short addr = 0xFF10; addr < 0xFF15; addr++)
    state->memory[addr] = 0x00;
  for (Short addr = 0xFF16; addr < 0xFF1F; addr++)
    state->memory[addr] = 0x00;
  for (Short addr = 0xFF20; addr < 0xFF25; addr++)
    state->memory[addr] = 0x00;
}


inline void updateAudio (State *state, Interface *interface) {
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

    // Control channel volume and add them to buffer
    // We divide by 8 because volume goes from 0+1 to 7+1 and by 4 because there are 4 channels
    state->audio.audio_buffer_l.push_back(int(NR50_L_VOL(state) + 1)*sample_l/(8*4));
    state->audio.audio_buffer_r.push_back(int(NR50_R_VOL(state) + 1)*sample_r/(8*4));


    // If buffer has enough data, send it to be played through the speakers
    if (state->audio.audio_buffer_l.size() >= SAMPLE_RATE/8) {
      interface->playAudio({state->audio.audio_buffer_l, state->audio.audio_buffer_r});
      resetAudioBuffers(state->audio);
    }
  }
}