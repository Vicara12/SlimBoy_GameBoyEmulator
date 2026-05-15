#pragma once

#include <algorithm>
#include "audiostate.h"
#include "interface.h"
#include "state.h"
#include "types.h"


inline void resetAudioBuffers (AudioState &audio_state) {
  audio_state.audio_buffer_l.clear();
  audio_state.audio_buffer_r.clear();
  audio_state.audio_buffer_l.reserve(AUDIO_BUFFER_SIZE);
  audio_state.audio_buffer_r.reserve(AUDIO_BUFFER_SIZE);
}


inline void SET_CH1_PERIOD (State *state, Short new_value) {
  state->memory[NR13_REGISTER] = new_value;
  state->memory[NR14_REGISTER] &= 0xF8;
  state->memory[NR14_REGISTER] |= (new_value >> 8) & 0x07;
}


inline void processCH1 (
  State *state,
  int &sample_l,
  int &sample_r
) {
  auto &ch1 = state->audio.ch1;
  if (ch1.NR14_written or ch1.NR12_written) {
    if (ch1.NR14_written) {
      if (NR14_CH1_LEN_ENABLED(state))
        ch1.auto_off_clk = state->cycles + (64 - NR11_CH1_INITIAL_LEN_TIMER(state)) * (CLOCK_FREQ/256);
      else
        ch1.auto_off_clk = std::numeric_limits<ulong>::max();
    }
    
    if (state->memory[NR12_REGISTER] & 0xF8 == 0) {
      state->audio.ch1.on = false;
      NR52_SET_CH1_OFF(state);
    }
    else if (NR14_CH1_TRIGGERED(state) and (state->memory[NR12_REGISTER] & 0xF8) != 0) {
      state->memory[NR14_REGISTER] &= 0x7F; // Clear trigger bit
      ch1.on = true;
      // Below period overflow will be detected, idx increased to 0 and that will trigger CH1 config
      ch1.period_overflow_clk = state->cycles;
      ch1.envelope_sweep_pace = NR12_CH1_SWEEP_PACE(state);
      ch1.envelope_next_clk = state->cycles + ch1.envelope_sweep_pace*(CLOCK_FREQ/64);
      ch1.sequence_idx = 7;
      ch1.volume = NR12_CH1_VOL(state) * 17; // Scale [0,15] -> [0,255]
      ch1.last_pace = NR10_CH1_SWEEP_PACE(state);
      NR52_SET_CH1_ON(state);
    }
    ch1.NR14_written = false;
    ch1.NR12_written = false;
  }

  if (state->cycles >= ch1.auto_off_clk) {
    ch1.auto_off_clk = std::numeric_limits<ulong>::max();
    ch1.on = false;
  }

  if (not ch1.on) {
    return;
  }

  Byte new_pace = NR10_CH1_SWEEP_PACE(state);
  if (new_pace == 0) {
    ch1.pace_change_clk = std::numeric_limits<ulong>::max();
  }
  else if (ch1.last_pace == 0) {
    ch1.pace_change_clk = state->cycles;
  }

  if (state->cycles >= ch1.period_overflow_clk) {
    ulong reminder = state->cycles - ch1.period_overflow_clk;
    ch1.period_overflow_clk = state->cycles + 4 * (0x0800 - GET_CH1_PERIOD(state)) - reminder;
    ch1.sequence_idx = (ch1.sequence_idx + 1)%8;
    // Configuration is only changed at the begining of an audio sequencer
    if (ch1.sequence_idx == 0) {
      ch1.duty_idx = NR11_CH1_DUTY(state);
    }
    // Check for new volume value (envelope)
    if (ch1.envelope_sweep_pace != 0 and state->cycles >= ch1.envelope_next_clk) {
      ch1.envelope_next_clk += ch1.envelope_sweep_pace*(CLOCK_FREQ/64);
      if (NR12_CH1_ENV_DIR(state)) {
        if (ch1.volume != 255)
          ch1.volume += 17;
      } else {
        if (ch1.volume != 0)
          ch1.volume -= 17;
      }
    }
    // Check for new pace
    if (state->cycles >= ch1.pace_change_clk) {
      ch1.pace_change_clk += new_pace * (CLOCK_FREQ/128);
      ch1.last_pace = new_pace;
      Short period = GET_CH1_PERIOD(state);
      Short change = period >> NR10_CH1_SWEEP_STEP(state);
      if (NR10_CH1_SWEEP_DIRECTION(state)) {
        if (period >= change)
          SET_CH1_PERIOD(state, period-change);
        else
          SET_CH1_PERIOD(state, 0);
      } else {
        Short new_period = period + change;
        if (new_period >= 0x0800) {
          ch1.on = false;
        } else {
          SET_CH1_PERIOD(state, new_period);
        }
      }
    }
    ch1.signal_value = AUDIO_SEQUENCER[ch1.duty_idx][ch1.sequence_idx] * ch1.volume;
  }

  if (NR51_PAN_CH1_L(state)) {
    sample_l += ch1.signal_value;
  }
  if (NR51_PAN_CH1_R(state)) {
    sample_r += ch1.signal_value;
  }
}


#include <math.h>
inline void testAudio (
  State *state,
  int &sample_l,
  int &sample_r
) {
  float val1 = std::sin(2 * M_PI * 440 * (state->cycles/PUSH_AUDIO_EACH)/SAMPLE_RATE);
  float val2 = std::sin(2 * M_PI * 540 * (state->cycles/PUSH_AUDIO_EACH)/SAMPLE_RATE + M_1_PI / 2);
  float evelope = std::abs(std::sin(2 * M_PI * 0.5 * (state->cycles/PUSH_AUDIO_EACH)/SAMPLE_RATE));

  sample_l = 255 * evelope * (val1 + 1)/2;
  sample_r = 255 * std::sqrt(1 - evelope * evelope) * (val2 + 1)/2;
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
      if (state->audio.registers_cleared) {
        state->audio.registers_cleared = true;
        clearAudioRegs(state);
      }
      continue;
    } else {
      state->audio.registers_cleared = false;
    }

    int sample_l = 0;
    int sample_r = 0;

    processCH1(state, sample_l, sample_r);

    // testAudio(state, sample_l, sample_r);

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