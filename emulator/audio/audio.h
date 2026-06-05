#pragma once

#include <algorithm>
#include "audiostate.h"
#include "interface.h"
#include "state.h"
#include "types.h"


namespace gb {

// General audio registers
inline bool audioEnabled(State &state) {
  return state.memory.f(Addr::NR52) & 0x80;
}

template<AudioChannel channel, bool set_on>
inline void setChxOnOff (State &state) {
  static constexpr Byte MASK = (1 << (static_cast<int>(channel) - 1));

  if constexpr (set_on) {
    state.memory.f(Addr::NR52) |= MASK;
  } else {
    state.memory.f(Addr::NR52) &= ~MASK;
  }
}

template<AudioChannel channel>
inline bool panChR (State &state) {
  static constexpr Byte MASK = (0x01 << (static_cast<int>(channel) - 1));
  return (state.memory.f(Addr::NR51) & MASK) != 0;
}

template<AudioChannel channel>
inline bool panChL (State &state) {
  static constexpr Byte MASK = (0x10 << (static_cast<int>(channel) - 1));
  return (state.memory.f(Addr::NR51) & MASK) != 0;
}

inline Byte volumeR (State &state) {return state.memory.f(Addr::NR50) & 0x07;}

inline Byte volumeL (State &state) {return (state.memory.f(Addr::NR50) & 0x70) >> 4;}


// Channel specific functionalities
inline Byte ch1SweepPace      (State &state) {return (state.memory.f(Addr::NR10) & 0x70) >> 4;}
inline bool ch1SweepDirection (State &state) {return (state.memory.f(Addr::NR10) & 0x08);}
inline Byte ch1SweepStep      (State &state) {return state.memory.f(Addr::NR10) & 0x07;}

template<AudioChannel channel>
inline Byte chDuty (State &state) {
  static_assert(
    channel == AudioChannel::CH1 or channel == AudioChannel::CH2,
    "Only channels 1 and 2 have a duty cycle"
  );
  static constexpr Short reg = (channel == AudioChannel::CH1 ? Addr::NR11 : Addr::NR21);
  return (state.memory.f(reg) & 0xC0) >> 6;
}

template<AudioChannel channel>
inline Byte chInitialLenTimer (State &state) {
  if constexpr      (channel == AudioChannel::CH1) return state.memory.f(Addr::NR11) & 0x3F;
  else if constexpr (channel == AudioChannel::CH2) return state.memory.f(Addr::NR21) & 0x3F;
  else if constexpr (channel == AudioChannel::CH3) return state.memory.f(Addr::NR31);
  else                                             return state.memory.f(Addr::NR41) & 0x3F;
}


template <AudioChannel channel>
inline Byte chVol (State &state) {
  if constexpr      (channel == AudioChannel::CH1) return state.memory.f(Addr::NR12) >> 4;
  else if constexpr (channel == AudioChannel::CH2) return state.memory.f(Addr::NR22) >> 4;
  else if constexpr (channel == AudioChannel::CH3) return (state.memory.f(Addr::NR32) & 0x60) >> 5;
  else                                             return state.memory.f(Addr::NR42) >> 4;
}

template <AudioChannel channel>
inline bool chEnvDir (State &state) {
  static_assert(channel != AudioChannel::CH3, "Channel 3 has no ENV DIR value");
  if constexpr      (channel == AudioChannel::CH1) return (state.memory.f(Addr::NR12) & 0x08) != 0;
  else if constexpr (channel == AudioChannel::CH2) return (state.memory.f(Addr::NR22) & 0x08) != 0;
  else                                             return (state.memory.f(Addr::NR42) & 0x08) != 0;
}

template <AudioChannel channel>
inline Byte chEnvelopePace (State &state) {
  static_assert(channel != AudioChannel::CH3, "Channel 3 has no SWEEP PACE value");
  if constexpr      (channel == AudioChannel::CH1) return state.memory.f(Addr::NR12) & 0x07;
  else if constexpr (channel == AudioChannel::CH2) return state.memory.f(Addr::NR22) & 0x07;
  else                                             return state.memory.f(Addr::NR42) & 0x07;
}

template <AudioChannel channel>
inline bool chZeroVolEnv (State &state) {
  static_assert(channel != AudioChannel::CH3, "Channel 3 has no ENV DIR value");
  if constexpr      (channel == AudioChannel::CH1) return (state.memory.f(Addr::NR12) & 0xF8) == 0;
  else if constexpr (channel == AudioChannel::CH2) return (state.memory.f(Addr::NR22) & 0xF8) == 0;
  else                                             return (state.memory.f(Addr::NR42) & 0xF8) == 0;
}


template <AudioChannel channel>
inline bool chTriggered (State &state) {
  if constexpr      (channel == AudioChannel::CH1) return (state.memory.f(Addr::NR14) & 0x80) != 0;
  else if constexpr (channel == AudioChannel::CH2) return (state.memory.f(Addr::NR24) & 0x80) != 0;
  else if constexpr (channel == AudioChannel::CH3) return (state.memory.f(Addr::NR34) & 0x80) != 0;
  else                                             return (state.memory.f(Addr::NR44) & 0x80) != 0;
}

template <AudioChannel channel>
inline bool chLenEnabled (State &state) {
  if constexpr      (channel == AudioChannel::CH1) return (state.memory.f(Addr::NR14) & 0x40) != 0;
  else if constexpr (channel == AudioChannel::CH2) return (state.memory.f(Addr::NR24) & 0x40) != 0;
  else if constexpr (channel == AudioChannel::CH3) return (state.memory.f(Addr::NR34) & 0x40) != 0;
  else                                             return (state.memory.f(Addr::NR44) & 0x40) != 0;
}

template <AudioChannel channel>
inline Short getChPeriod (State &state) {
  static_assert(channel != AudioChannel::CH4, "Channel 4 has no period");
  if constexpr (channel == AudioChannel::CH1)
    return (Short(state.memory.f(Addr::NR14) & 0x07) << 8) | Short(state.memory.f(Addr::NR13));
  else if constexpr (channel == AudioChannel::CH2)
    return (Short(state.memory.f(Addr::NR24) & 0x07) << 8) | Short(state.memory.f(Addr::NR23));
  else
    return (Short(state.memory.f(Addr::NR34) & 0x07) << 8) | Short(state.memory.f(Addr::NR33));
}

template<AudioChannel channel>
inline void setChPeriod (State &state, Short new_value) {
  static_assert(channel != AudioChannel::CH4, "Channel 4 has no period");
  if constexpr (channel == AudioChannel::CH1) {
    state.memory.f(Addr::NR13) = new_value;
    state.memory.f(Addr::NR14) &= 0xF8;
    state.memory.f(Addr::NR14) |= (new_value >> 8) & 0x07;
  }
  else if constexpr (channel == AudioChannel::CH2) {
    state.memory.f(Addr::NR23) = new_value;
    state.memory.f(Addr::NR24) &= 0xF8;
    state.memory.f(Addr::NR24) |= (new_value >> 8) & 0x07;
  }
  else {
    state.memory.f(Addr::NR33) = new_value;
    state.memory.f(Addr::NR34) &= 0xF8;
    state.memory.f(Addr::NR34) |= (new_value >> 8) & 0x07;
  }
}

inline bool ch3DACEnabled (State &state) {return state.memory.f(Addr::NR30) & 0x80;}

inline bool ch4ShortMode (State &state) {return state.memory.f(Addr::NR43) & 0x08;}


inline void resetAudioBuffers (AudioState &audio_state) {
  audio_state.aud_pkg = AudioPacket();
  audio_state.aud_pkg.buffer_l.clear();
  audio_state.aud_pkg.buffer_r.clear();
  audio_state.aud_pkg.buffer_l.reserve(AUDIO_BUFFER_SIZE);
  audio_state.aud_pkg.buffer_r.reserve(AUDIO_BUFFER_SIZE);
}


template<AudioChannel channel>
inline void pulseChannelState (State &state, PulseChannelData &ch_data) {
  static constexpr Short NRX2 = (channel == AudioChannel::CH1 ? Addr::NR12 : Addr::NR22);
  static constexpr Short NRX4 = (channel == AudioChannel::CH1 ? Addr::NR14 : Addr::NR24);
  bool NRX2_written = state.memory.specialAddrWritten(NRX2);
  bool NRX4_written = state.memory.specialAddrWritten(NRX4);
  if (NRX2_written or NRX4_written) {
    if (NRX4_written) {
      if (chLenEnabled<channel>(state))
        ch_data.auto_off_clk = state.timing.cycles + (64 - chInitialLenTimer<channel>(state)) * (CLOCK_FREQ/256);
      else
        ch_data.auto_off_clk = std::numeric_limits<ulong>::max();
    }
    
    if (chZeroVolEnv<channel>(state)) {
      ch_data.on = false;
      setChxOnOff<channel, false>(state);
    }
    else if (chTriggered<channel>(state)) {
      state.memory.f(Addr::NR14) &= 0x7F; // Clear trigger bit
      ch_data.on = true;
      // Period overflow will be detected, sequence_idx increased to 0 and that will trigger CH1 config
      ch_data.period_overflow_clk = state.timing.cycles;
      ch_data.envelope_pace = chEnvelopePace<channel>(state);
      ch_data.envelope_next_clk = state.timing.cycles + ch_data.envelope_pace*(CLOCK_FREQ/64);
      ch_data.sequence_idx = 7;
      ch_data.volume = chVol<channel>(state) * 17; // Scale [0,15] -> [0,255]
      if constexpr (channel == AudioChannel::CH1)
        ch_data.last_pace = ch1SweepPace(state);
      setChxOnOff<channel, true>(state);
    }
  }
}


template<AudioChannel channel, typename ChDataT>
inline void checkNewEnvelopeVolume (State &state, ChDataT &ch_data) {
  if (ch_data.envelope_pace != 0 and state.timing.cycles >= ch_data.envelope_next_clk) {
    ch_data.envelope_next_clk += ch_data.envelope_pace*(CLOCK_FREQ/64);
    if (chEnvDir<channel>(state)) {
      if (ch_data.volume != 255)
        ch_data.volume += 17;
    } else {
      if (ch_data.volume != 0)
        ch_data.volume -= 17;
    }
  }
}


inline void checkNewPace (State &state, PulseChannelData &ch_data) {
  Short new_pace = ch1SweepPace(state);
  if (state.timing.cycles >= ch_data.pace_change_clk) {
    ch_data.pace_change_clk += new_pace * (CLOCK_FREQ/128);
    ch_data.last_pace = new_pace;
    Short period = getChPeriod<AudioChannel::CH1>(state);
    Short change = period >> ch1SweepStep(state);
    if (ch1SweepDirection(state)) {
      if (period >= change)
        setChPeriod<AudioChannel::CH1>(state, period-change);
      else
        setChPeriod<AudioChannel::CH1>(state, 0);
    } else {
      Short new_period = period + change;
      if (new_period >= 0x0800) {
        ch_data.on = false;
      } else {
        setChPeriod<AudioChannel::CH1>(state, new_period);
      }
    }
  }
}


template<AudioChannel channel>
inline void processPulseChannel (
  State &state,
  PulseChannelData &ch_data,
  int &sample_l,
  int &sample_r
) {
  pulseChannelState<channel>(state, ch_data);

  if (state.timing.cycles >= ch_data.auto_off_clk) {
    ch_data.auto_off_clk = std::numeric_limits<ulong>::max();
    ch_data.on = false;
  }

  if (not ch_data.on) {
    return;
  }

  if constexpr (channel == AudioChannel::CH1) {
    if (ch1SweepPace(state) == 0) {
      ch_data.pace_change_clk = std::numeric_limits<ulong>::max();
    }
    else if (ch_data.last_pace == 0) {
      ch_data.pace_change_clk = state.timing.cycles;
    }
  }

  if (state.timing.cycles >= ch_data.period_overflow_clk) {
    ulong reminder = state.timing.cycles - ch_data.period_overflow_clk;
    ch_data.period_overflow_clk = state.timing.cycles + 4 * (0x0800 - getChPeriod<channel>(state)) - reminder;
    ch_data.sequence_idx = (ch_data.sequence_idx + 1)%8;
    // Configuration is only changed at the begining of an audio sequencer
    if (ch_data.sequence_idx == 0) {
      ch_data.duty_idx = chDuty<channel>(state);
    }
    checkNewEnvelopeVolume<channel, PulseChannelData>(state, ch_data);
    // Check for new pace
    if constexpr (channel == AudioChannel::CH1) {
      checkNewPace(state, ch_data);
    }
    ch_data.signal_value = AUDIO_SEQUENCER[ch_data.duty_idx][ch_data.sequence_idx] * ch_data.volume;
  }

  if (panChL<channel>(state)) {
    sample_l += ch_data.signal_value;
  }
  if (panChR<channel>(state)) {
    sample_r += ch_data.signal_value;
  }
}


inline Byte waveChannelVolumeShift (State &state) {
  switch (chVol<AudioChannel::CH3>(state)) {
    case 0b00: return 8; //   0% volume
    case 0b01: return 0; // 100% volume
    case 0b10: return 1; //  50% volume
    case 0b11: return 2; //  25% volume
  }
  return 0; // Impossible
}


inline void processWaveChannel (State &state, int &sample_l, int &sample_r) {
  WaveChannelData &ch3 = state.audio.ch3;
  if (not ch3DACEnabled(state)) {
    ch3.on = false;
  }
  else if (state.memory.specialAddrWritten(Addr::NR34)) {
    if (chTriggered<AudioChannel::CH3>(state)) {
      state.memory.f(Addr::NR34) &= 0x7F; // Clear trigger bit
      ch3.on = true;
      ch3.period_overflow_clk = state.timing.cycles;
      ch3.ram_idx = 0;
    }
    if (chLenEnabled<AudioChannel::CH3>(state))
      ch3.auto_off_clk = state.timing.cycles + (256 - chInitialLenTimer<AudioChannel::CH3>(state)) * (CLOCK_FREQ/256);
    else
      ch3.auto_off_clk = std::numeric_limits<ulong>::max();
  }

  if (state.timing.cycles >= ch3.auto_off_clk) {
    ch3.auto_off_clk = std::numeric_limits<ulong>::max();
    ch3.on = false;
  }

  if (not ch3.on) {
    return;
  }

  if (state.timing.cycles >= ch3.period_overflow_clk) {
    ch3.period_overflow_clk += 2 * (0x0800 - getChPeriod<AudioChannel::CH3>(state));
    ch3.ram_idx = (ch3.ram_idx+1)%32;
    Byte mem_val = state.memory.f(Addr::WPRAM + ch3.ram_idx/2);
    Byte wave_val = ((ch3.ram_idx % 2) == 0 ? (mem_val >> 4) : (mem_val & 0x0F));
    ch3.signal_value = (((255/15) * wave_val) >> waveChannelVolumeShift(state));
  }

  if (panChL<AudioChannel::CH3>(state)) {
    sample_l += ch3.signal_value;
  }
  if (panChR<AudioChannel::CH3>(state)) {
    sample_r += ch3.signal_value;
  }
}


inline ulong noiseShiftCycles (State &state) {
  Byte nr43_val = state.memory.f(Addr::NR43);
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


inline Byte cycleLFSR (State &state) {
  Short &lfsr = state.audio.ch4.lfsr;  
  Short new_bit = (lfsr & 0x01) ^ ((lfsr & 0x02) >> 1); 
  lfsr >>= 1;
  lfsr |= (new_bit << 14);
  if (ch4ShortMode(state)) {
    lfsr &= ~(1 << 6);
    lfsr |= (new_bit << 6);
  }
  return (~lfsr) & 0x01; 
}


inline void noiseChannelState(State &state, NoiseChannelData &ch4) {
  bool NR43_written = state.memory.specialAddrWritten(Addr::NR43);
  if (chZeroVolEnv<AudioChannel::CH4>(state)) {
    ch4.on = false;
  }
  else if (state.memory.specialAddrWritten(Addr::NR44)) {
    if (chTriggered<AudioChannel::CH4>(state)) {
      state.memory.f(Addr::NR44) &= 0x7F; // Clear trigger bit
      ch4.on = true;
      ch4.volume = chVol<AudioChannel::CH4>(state) * 17; // Scale [0,15] -> [0,255]
      ch4.envelope_pace = chEnvelopePace<AudioChannel::CH4>(state);
      ch4.envelope_next_clk = state.timing.cycles + ch4.envelope_pace*(CLOCK_FREQ/64);
      ch4.lfsr = 0x7FFF;
      NR43_written = true; // This will trigger LFSR frequency calculation below
    }
    if (chLenEnabled<AudioChannel::CH4>(state))
      ch4.auto_off_clk = state.timing.cycles + (64 - chInitialLenTimer<AudioChannel::CH4>(state)) * (CLOCK_FREQ/256);
    else
      ch4.auto_off_clk = std::numeric_limits<ulong>::max();
  }
  if (NR43_written) {
    ch4.noise_shift_cycles = noiseShiftCycles(state);
    ch4.period_overflow_clk = state.timing.cycles;
  }

  if (state.timing.cycles >= ch4.auto_off_clk) {
    ch4.auto_off_clk = std::numeric_limits<ulong>::max();
    ch4.on = false;
  }
}


inline void processNoiseChannel(State &state, int &sample_l, int &sample_r) {
  NoiseChannelData &ch4 = state.audio.ch4;
  noiseChannelState(state, ch4);

  if (not ch4.on) {
    return;
  }

  if (state.timing.cycles >= ch4.period_overflow_clk) {
    ulong reminder = state.timing.cycles - ch4.period_overflow_clk;
    ch4.period_overflow_clk = state.timing.cycles + ch4.noise_shift_cycles - reminder;
    checkNewEnvelopeVolume<AudioChannel::CH4, NoiseChannelData>(state, ch4);
    ch4.signal_value = cycleLFSR(state) * ch4.volume;
  }

  if (panChL<AudioChannel::CH4>(state)) {
    sample_l += ch4.signal_value;
  }
  if (panChR<AudioChannel::CH4>(state)) {
    sample_r += ch4.signal_value;
  }
}


inline void clearAudioRegs (State &state)
{
  for (Short addr = 0xFF10; addr < 0xFF15; addr++)
    state.memory.f(addr) = 0x00;
  for (Short addr = 0xFF16; addr < 0xFF1F; addr++)
    state.memory.f(addr) = 0x00;
  for (Short addr = 0xFF20; addr < 0xFF25; addr++)
    state.memory.f(addr) = 0x00;
}


template<class InterfaceT>
inline void updateAudio (State &state, InterfaceT &interface) {
  while (state.timing.cycles >= state.audio.cycles_next_push) {
    state.audio.cycles_next_push += PUSH_AUDIO_EACH;

    if (not audioEnabled(state)) {
      if (not state.audio.registers_cleared) {
        state.audio.registers_cleared = true;
        clearAudioRegs(state);
      }
      continue;
    } else {
      state.audio.registers_cleared = false;
    }

    int sample_l = 0;
    int sample_r = 0;

    processPulseChannel<AudioChannel::CH1>(state, state.audio.ch1, sample_l, sample_r);
    processPulseChannel<AudioChannel::CH2>(state, state.audio.ch2, sample_l, sample_r);
    processWaveChannel(state, sample_l, sample_r);
    processNoiseChannel(state, sample_l, sample_r);

    // Control channel volume and add them to buffer
    // We divide by 8 because volume goes from 0+1 to 7+1 and by 4 because there are 4 channels
    state.audio.aud_pkg.buffer_l.push_back(int(volumeL(state) + 1)*sample_l/(8*4));
    state.audio.aud_pkg.buffer_r.push_back(int(volumeR(state) + 1)*sample_r/(8*4));


    // If buffer has enough data, send it to be played through the speakers
    if (state.audio.aud_pkg.buffer_l.size() >= AUDIO_BUFFER_SIZE) {
      interface.playAudio(std::move(state.audio.aud_pkg));
      // TODO try double buffering
      resetAudioBuffers(state.audio);
    }
  }
}

} // namespace gb