#pragma once

#include <vector>
#include "types.h"
#include "generaldefines.h"


#define SAMPLE_RATE     (1024*8)
#define PUSH_AUDIO_EACH (CLOCK_FREQ / SAMPLE_RATE)


struct AudioState {
  ulong cycles_next_push = 0;
  std::vector<Byte> audio_buffer_l;
  std::vector<Byte> audio_buffer_r;
  float freq_l = 440; // TODO remove from here below
  float freq_r = 540;
  int idx_l = 0;
  int idx_r = 0;
};