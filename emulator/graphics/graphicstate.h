/*
* Several constants and structures regarding graphics
*/

#pragma once

#include <array>
#include "../types.h"


namespace gb {

// Timing constants
inline constexpr uint DOTS_PER_FRAME = 70224;
inline constexpr uint DOTS_PER_LINE  = 456;
inline constexpr uint DOTS_OAM_SCAN  = 80;
inline constexpr uint DOTS_DRAW_PX   = 200;

// Screen constants
inline constexpr uint SCREEN_PX_W     = 160;
inline constexpr uint SCREEN_PX_H     = 144;
inline constexpr uint LINES_IN_SCREEN = 154;
inline constexpr uint BG_PX_SIZE      = 256;


enum class ScreenMode : Byte {
  HBLANK = 0,
  VBLANK = 1,
  OAMSC  = 2,
  DRAW   = 3
};


enum class BWColors : Byte {
  WHITE = 0,
  LGRAY = 1,
  DGRAY = 2,
  BLACK = 3,
  TRANS = 4
};


using PaletteColors = std::array<Byte,4>;
using ScreenLineData = std::array<Byte, SCREEN_PX_W>;

struct ScreenLine{
  ulong frame_last_updated = 0;
  ScreenLineData pixel;
};

using ScreenPixels = std::array<ScreenLine, SCREEN_PX_H>;

struct ScreenFrame {
  ScreenMode last_mode = ScreenMode::HBLANK;
  bool ly_lyc_flag_already_set = false;
  Byte window_y = 0;
  PaletteColors bgw_palette;
  PaletteColors obp0_palette;
  PaletteColors obp1_palette;
  ScreenPixels *line;
};


// Palette color extraction requires interleaving bits with zeros (0b11 -> 0b0101)
// Precompute all possible values 256 into a lookup table
constexpr std::array<uint16_t, 256> generateInterleaveLUT() {
    std::array<uint16_t, 256> lut = {};
    for (int i = 0; i < 256; i++) {
        uint16_t val = 0;
        for (int b = 0; b < 8; b++) {
            if (i & (1 << b)) {
                val |= (1 << (b * 2));
            }
        }
        lut[i] = val;
    }
    return lut;
}

static constexpr auto INTERLEAVE_LUT = generateInterleaveLUT();

} // namespace gb