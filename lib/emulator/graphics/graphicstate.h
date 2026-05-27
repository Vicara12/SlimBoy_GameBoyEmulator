/*
* Several constants and structures regarding graphics
*/

#pragma once

#include <array>
#include "types.h"


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
using ScreenLineData = std::array<float, SCREEN_PX_W>;

struct ScreenLine{
  ulong frame_last_updated = 0;
  ScreenLineData pixel;
};

using ScreenPixels = std::array<ScreenLine, SCREEN_PX_H>;

struct ScreenFrame {
  ScreenMode last_mode = ScreenMode::HBLANK;
  bool ly_lyc_flag_already_set = false;
  Byte window_y = 0;
  ScreenPixels *line;
};