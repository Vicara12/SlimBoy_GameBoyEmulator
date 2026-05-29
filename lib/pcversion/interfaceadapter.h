/*
* Functions provided to emulator's Interface struct to couple it to the PC version.
*/

#pragma once

#include <array>
#include <iostream>
#include <mutex>
#include <chrono>
#include <thread>
#include "interface.h"
#include "audiostream.h"



class PCInterface : public HardwareInterface<PCInterface> {
  EmulatorAudioStream<8*AUDIO_BUFFER_SIZE> audio_stream;
  std::chrono::steady_clock::time_point ini_t;
  
public:

  PCInterface();

  inline void print (const std::string_view &s) {std::cout << s;};

  inline void println (const std::string_view &s) {std::cout << s << std::endl;};

  inline std::string userLineInput () {std::string line; std::getline(std::cin, line); return line;};

  inline void sleepMillis (uint t) {std::this_thread::sleep_for(std::chrono::milliseconds(t));};

  inline ulong realTimeMicros () {
    return std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now() - ini_t
    ).count();
  };
  
  inline void playAudio (AudioPacket&& ap) {audio_stream.pushSamples(std::move(ap));};

  auto& getAudioStream () {return audio_stream;}
};