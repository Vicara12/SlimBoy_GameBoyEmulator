#pragma once

#include <atomic>
#include <SFML/Audio.hpp>
#include "audio/audiostate.h"


template<size_t BUFFER_SIZE>
class EmulatorAudioStream : public sf::SoundStream {
private:
  static_assert((BUFFER_SIZE & (BUFFER_SIZE - 1)) == 0, "Buffer size must be a power of 2");

  static constexpr ulong MASK = BUFFER_SIZE - 1;

  std::array<sf::Int16, BUFFER_SIZE> ring_buffer;
  std::vector<sf::Int16> playing_chunk;
  alignas(64) std::atomic<size_t> write_idx{0};
  alignas(64) std::atomic<size_t> read_idx{0};
  size_t sr;

public:
  EmulatorAudioStream(unsigned int sample_rate) : sr(sample_rate) {
    initialize(2, sr);
  }

  inline void pushSamples(const AudioPacket &new_samples) {
    if (new_samples.buffer_l.size() != new_samples.buffer_r.size()) {
      std::cerr << "ERROR: L & R sound channels buffer size mismatch" << std::endl;
    }

    size_t r = read_idx.load(std::memory_order_acquire);
    size_t w = write_idx.load(std::memory_order_relaxed);

    size_t free_space = BUFFER_SIZE - (w - r);
    size_t to_write = std::min(new_samples.buffer_l.size() + new_samples.buffer_r.size(), free_space);

    for (size_t i = 0; 2*i < to_write; i++) {
      // Convert from uint8_t to int16_t in such a way that 0 -> int16 min and 255 -> int16 max
      ring_buffer[w & MASK] = static_cast<int16_t>(new_samples.buffer_l[i]) * 257 - 32768;
      ring_buffer[(w + 1) & MASK] = static_cast<int16_t>(new_samples.buffer_r[i]) * 257 - 32768;
      w += 2;
    }

    write_idx.store(w, std::memory_order_release);
  }

protected:

  virtual bool onGetData(Chunk& data) override {
    size_t w = write_idx.load(std::memory_order_acquire);
    size_t r = read_idx.load(std::memory_order_relaxed);
    if (w != r) {
      playing_chunk.resize(w - r);

      for (size_t i = 0; i < playing_chunk.size(); i++) {
        playing_chunk[i] = ring_buffer[(r + i) & MASK];
      }
      read_idx.store(w, std::memory_order_release);
    } else {
      playing_chunk.assign(AUDIO_BUFFER_SIZE / 4, -32768);
    }

    data.samples = playing_chunk.data();
    data.sampleCount = playing_chunk.size();

    return true;
  }

  // Required override for sf::SoundStream
  virtual void onSeek(sf::Time timeOffset) override {}
};