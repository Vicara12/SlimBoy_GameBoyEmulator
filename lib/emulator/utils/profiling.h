#pragma once

#include <array>
#include <sstream>
#include <iomanip>
#include <limits>

#ifdef PROFILE
#include <Arduino.h>
#endif


template<size_t n_markers>
class Profiler {
  inline static constexpr uint32_t PRINT_FREQ = 240'000'000 * 2;

  inline static std::array<uint32_t, n_markers> cpu_cycles;
  inline static std::array<uint32_t, n_markers-1> cycle_count = {0};
  inline static uint32_t next_print_cycles = 0;

  inline Profiler() = default;

  inline static void printProfiling() {
    uint32_t total_cycles = 0;
    for (auto c : cycle_count) {
      total_cycles += c;
    }
    if (total_cycles == 0) {
      return;
    }
    std::stringstream ss;
    ss << "[" << std::fixed << std::setprecision(4);
    for (size_t i = 0; i < n_markers-1; i++) {
      if (i != 0)
        ss << " ";
      ss << i << ":" << double(cycle_count[i])/double(total_cycles);
    }
    ss << "]";
    #ifdef PROFILE
    Serial.println(ss.str().c_str());
    #endif
  }

public:

  template<size_t marker>
  inline static void measure() {
    #ifdef PROFILE
    static_assert(marker < n_markers, "Marker exceeds the number of markers");
    if constexpr (marker == 0) {
      for (size_t i = 0; i < n_markers-1; i++) {
        cycle_count[i] += cpu_cycles[i+1] - cpu_cycles[i];
      }
      uint32_t cycles = esp_cpu_get_ccount();
      if (cycles - next_print_cycles >= PRINT_FREQ) {
        printProfiling();
        cycle_count = {0};
        next_print_cycles = next_print_cycles + PRINT_FREQ;
      }
    }
    cpu_cycles[marker] = micros();
    #endif
  }
};