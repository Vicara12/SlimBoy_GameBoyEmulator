/*
* Several defines and structures regarding graphics
*/

#pragma once

#include <array>
#include "types.h"


// Timing constants
#define DOTS_PER_FRAME  70224
#define DOTS_PER_LINE   456
#define DOTS_OAM_SCAN   80
#define DOTS_DRAW_PX    200

// Screen constants
#define SCREEN_PX_W     160
#define SCREEN_PX_H     144
#define LINES_IN_SCREEN 154
#define BG_PX_SIZE      256

// Registers
#define LCDC_REGISTER 0xFF40 // LCD Control
#define STAT_REGISTER 0xFF41 // LCD Status
#define SCY_REGISTER  0xFF42 // Background viewport top-left Y coord
#define SCX_REGISTER  0xFF43 // Background viewport top-left X coord
#define LY_REGISTER   0xFF44 // LCD Y coordinate
#define LYC_REGISTER  0xFF45 // LY compare
#define DMA_REGISTER  0xFF46 // Transfer from ROM/RAM to OAM
#define BGP_REGISTER  0xFF47 // BG and Window palette data
#define OBP0_REGISTER 0xFF48 // Object palette 0
#define OBP1_REGISTER 0xFF49 // Object palette 1
#define WY_REGISTER   0xFF4A // Window position Y
#define WX_REGISTER   0xFF4B // Window position X+7

#define LCDC_LCD_ENABLED(state)             ((state->memory[LCDC_REGISTER] & 0x80) != 0)
#define LCDC_WIN_TILE_MAP_HIGH(state)       ((state->memory[LCDC_REGISTER] & 0x40) != 0)
#define LCDC_WIN_ENABLE(state)              ((state->memory[LCDC_REGISTER] & 0x20) != 0)
#define LCDC_BG_W_TILE_DATA_AREA_LOW(state) ((state->memory[LCDC_REGISTER] & 0x10) != 0)
#define LCDC_BG_TILE_MAP_HIGH(state)        ((state->memory[LCDC_REGISTER] & 0x08) != 0)
#define LCDC_OBJ_SIZE_BIG(state)            ((state->memory[LCDC_REGISTER] & 0x04) != 0)
#define LCDC_OBJ_ENABLED(state)             ((state->memory[LCDC_REGISTER] & 0x02) != 0)
#define LCDC_BG_WIN_ENABLED(state)          ((state->memory[LCDC_REGISTER] & 0x01) != 0)

// Screen modes
#define MODE0_HBLANK 0
#define MODE1_VBLANK 1
#define MODE2_OAMSC  2
#define MODE3_DRAW   3

// Colors
#define COLOR_WHITE 0
#define COLOR_LGRAY 1
#define COLOR_DGRAY 2
#define COLOR_BLACK 3
#define COLOR_TRANS 4


using PaletteColors = std::array<Byte,4>;
using ScreenLineData = std::array<float, SCREEN_PX_W>;


struct ScreenLine{
  ulong frame_last_updated = 0;
  ScreenLineData pixel;
};

struct ScreenFrame {
  Byte last_mode = 0;
  bool ly_lyc_flag_already_set = false;
  Byte window_y = 0;
  std::array<ScreenLine, SCREEN_PX_H> line;
};