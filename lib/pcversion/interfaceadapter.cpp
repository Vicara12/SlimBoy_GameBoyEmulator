#include "interfaceadapter.h"
#include "audio/audiostate.h"



PCInterface::PCInterface()
  : audio_stream(SAMPLE_RATE)
  , ini_t(std::chrono::steady_clock::now())
{}