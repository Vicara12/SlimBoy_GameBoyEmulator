#pragma once

#include "audiostate.h"
#include "interface.h"
#include "state.h"

#include <math.h> // TODO remove


inline void resetAudioBuffers (AudioState &audio_state) {
  audio_state.audio_buffer_l.clear();
  audio_state.audio_buffer_r.clear();
  audio_state.audio_buffer_l.reserve(1024);
  audio_state.audio_buffer_r.reserve(1024);
}


// Send whatever audio is in the buffer and pad with 127 until target length
inline void flushAudio (State *state, Interface *interface) {
  int pad_len = SAMPLE_RATE/8 - state->audio.audio_buffer_l.size();
  for (int i = 0; i < pad_len; i++) {
    state->audio.audio_buffer_l.push_back(127);
    state->audio.audio_buffer_r.push_back(127);
  }

  interface->playAudio({state->audio.audio_buffer_l, state->audio.audio_buffer_r});
  resetAudioBuffers(state->audio);
}


inline void updateAudio (State *state, Interface *interface) {
  while (state->cycles >= state->audio.cycles_next_push) {
    state->audio.cycles_next_push += PUSH_AUDIO_EACH;

    // Process audio
    float next_sample_l = std::sin(state->audio.freq_l * 2 * M_PI * float(state->audio.idx_l)/SAMPLE_RATE);
    float next_sample_r = std::sin(state->audio.freq_r * 2 * M_PI * float(state->audio.idx_r)/SAMPLE_RATE);
    float envelope_l = std::sin(1 * M_PI * float(state->audio.idx_l)/SAMPLE_RATE);
    float envelope_r = std::sin(1 * M_PI * (float(state->audio.idx_l)/SAMPLE_RATE + 0.5));
    state->audio.idx_l = (state->audio.idx_l + 1)%SAMPLE_RATE;
    state->audio.idx_r = (state->audio.idx_r + 1)%SAMPLE_RATE;
    state->audio.audio_buffer_l.push_back(255 * (envelope_l * envelope_l) * (next_sample_l + 1) / 4);
    state->audio.audio_buffer_r.push_back(255 * (envelope_r * envelope_r) * (next_sample_r + 1) / 4);

    if (state->audio.audio_buffer_l.size() >= SAMPLE_RATE/8) {
      interface->playAudio({state->audio.audio_buffer_l, state->audio.audio_buffer_r});
      resetAudioBuffers(state->audio);
    }
  }
}