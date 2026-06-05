/*
* Functions for obtaining the color of each pixel in the screen (used to render graphics)
*/

#pragma once

#include "types.h"
#include "state.h"
#include "interface.h"
#include "graphics/graphicstate.h"
#include <cstring>


namespace gb {

inline bool LCDEnabled (State &state) {return state.memory.f(Addr::LCDC) & 0x80;}

inline bool winUseTileMapHigh (State &state) {return state.memory.f(Addr::LCDC) & 0x40;}

inline bool isWinEnabled (State &state) {return state.memory.f(Addr::LCDC) & 0x20;}

inline bool useBGWTileDataAreaLow (State &state) {return state.memory.f(Addr::LCDC) & 0x10;}

inline bool useBGTileMapHigh (State &state) {return state.memory.f(Addr::LCDC) & 0x08;}

inline bool objSizeBig (State &state) {return state.memory.f(Addr::LCDC) & 0x04;}

inline bool objEnabled (State &state) {return state.memory.f(Addr::LCDC) & 0x02;}

inline bool bgWinEnabled (State &state) {return state.memory.f(Addr::LCDC) & 0x01;}


inline ScreenMode getMode (State &state, Byte &current_line)
{
  ulong current_dot = state.timing.cycles%DOTS_PER_FRAME;
  current_line = current_dot/DOTS_PER_LINE;
  if (current_line >= SCREEN_PX_H) {
    return ScreenMode::VBLANK;
  }
  ulong dot_in_line = current_dot%DOTS_PER_LINE;
  if (dot_in_line < DOTS_OAM_SCAN) {
    return ScreenMode::OAMSC;
  }
  else if (dot_in_line < DOTS_OAM_SCAN + DOTS_DRAW_PX) {
    return ScreenMode::DRAW;
  }
  return ScreenMode::HBLANK;
}


inline void setInterrupts (Byte line_n, ScreenMode mode, State &state)
{
  Byte stat_value = state.memory.f(Addr::STAT);

  // Handle STAT (LCD) interrupts
  // Check if LYC == LY interrupt needs to be triggered
  if (state.memory.f(Addr::LYC) == line_n) {
    if ((stat_value & 0x40) != 0 and // LYC int select to 1
        not state.screen.ly_lyc_flag_already_set
        ) {
      state.screen.ly_lyc_flag_already_set = true;
      setInterrupt<Interrupt::LCD>(state);
    }
  } else {
    state.screen.ly_lyc_flag_already_set = false;
  }
  // Check if mode interrupt needs to be triggered
  if ((mode != state.screen.last_mode) and 
      ((mode == ScreenMode::HBLANK and (stat_value & 0x08) != 0) or
       (mode == ScreenMode::VBLANK and (stat_value & 0x10) != 0) or
       (mode == ScreenMode::OAMSC  and (stat_value & 0x20) != 0))) {
    setInterrupt<Interrupt::LCD>(state);
  }

  // Handle VBLank interrupts
  if (mode != state.screen.last_mode and mode == ScreenMode::VBLANK) {
    setInterrupt<Interrupt::VBLANK>(state);
  }

  state.screen.last_mode = mode;
}


inline void updateLCDStatusRegs (Byte line_n, ScreenMode mode, State &state)
{
  Byte &stat_val = state.memory.f(Addr::STAT);
  state.memory.f(Addr::LY) = line_n;
  stat_val &= 0xF8; // clear three lsb of stat
  stat_val |= static_cast<Byte>(mode); // update PPU mode to two lsb of stat
  stat_val |= Byte(state.memory.f(Addr::LYC) == line_n) << 2; // update LYC == LY flag
}


template<bool IS_OBJECT>
inline PaletteColors colorsFromPalette(Byte palette)
{
  PaletteColors colors = {
    Byte(IS_OBJECT ? static_cast<Byte>(BWColors::TRANS) : (palette & 0x03)),
    Byte((palette >> 2) & 0x03),
    Byte((palette >> 4) & 0x03),
    Byte((palette >> 6) & 0x03)
  };
  return colors;
}


// Get the colors of a tile's line.
// - tile_data_index: tile's data index
// - tile_line: number of the tile's line (0 to 7, both inclusive)
// - is_object: wether the tile corresponds to an object or BG/Window
// - use_obp0: if is_object wether to use palette OBJ0 or OBJ1, ignored otherwise
// - dst: memory address where the colors will be written, must be at least 8 bytes wide
// - state: emulator's state struct
template<bool OBJECT>
inline void getTileLine (
  Byte tile_data_index,
  Byte tile_line,
  bool use_obp0,
  Byte *dst,
  Byte *color_ids,
  State &state,
  Byte from = 0,
  Byte to = 8
)
{
  Short tile_data_address;
  // Compute tile data address from Block 0 (0x8000 - 0x87FF) and Block 1 (0x8800 - 0x8FFF)
  if (OBJECT or useBGWTileDataAreaLow(state)) {
    if (OBJECT and objSizeBig(state)) {
        if (tile_line >= 8) {
            tile_data_index |= 0x01;  // bottom tile
            tile_line -= 8;
        } else {
            tile_data_index &= 0xFE;  // top tile
        }
    }
    tile_data_address = 0x8000 + tile_data_index * 16;
  }
  // Compute tile data address from Block 1 (0x8800 - 0x8FFF) and Block 2 (0x9000 - 0x97FF)
  else {
    tile_data_address = 0x9000 + int16_t(int8_t(tile_data_index))*16;
  }
  // Retrieve tile data and get palette indices from it
  Byte tile_data_low = state.memory.f(tile_data_address + tile_line*2);
  Byte tile_data_high = state.memory.f(tile_data_address + tile_line*2 + 1);
  // Transform palette indices to actual colors
  const PaletteColors *colors;
  if constexpr (OBJECT) {
    colors = &(use_obp0 ? state.screen.obp0_palette : state.screen.obp1_palette);
  } else {
    colors = &state.screen.bgw_palette;
  }
  Short interleaved = (INTERLEAVE_LUT[tile_data_high] << 1) | INTERLEAVE_LUT[tile_data_low];
  for (Byte i = from; i < to; i++) {
    Byte bit = 7 - i; // 7-i because lsb is tile's rightmost pixel
    Byte palette_id = (interleaved >> (bit * 2)) & 3;
    dst[i - from] = (*colors)[palette_id];
    // Color/palette ids are needed to determine color when OBJ priority is false
    if constexpr (not OBJECT) {
      color_ids[i - from] = palette_id;
    }
  }
}


void renderLineOBJ (Byte line_n, ScreenLineData &line, const ScreenLineData &bgw_line_idcs, State &state)
{
  if (not objEnabled(state)) {
    return;
  }

  std::array<Short, 10> objs_in_line;
  Byte obj_height = (objSizeBig(state) ? 16 : 8);
  Byte n_objs = 0;

  // Scan Object Attribute Memory object by object and if the object overlaps with the line store it
  for (Short obj_addr = 0xFE00; obj_addr < 0xFEA0; obj_addr += 4) {
    Byte obj_y = state.memory.f(obj_addr);
    // line_n + 16 because obj_y has an offset of 16 and subtracting could cause underflow
    if (obj_y <= (line_n + 16) and (obj_y + obj_height) > (line_n + 16)) {
      objs_in_line[n_objs++] = obj_addr;
      if (n_objs >= 10) {
        break;
      }
    }
  }

  // Draw the selected objects into the line
  // px_written holds 255 if no object has been written to the pixel and the x position of the
  // leftmost object's pixel otherwise. Used to compute object's drawing priorities.
  std::array<Byte, SCREEN_PX_W> px_written;
  px_written.fill(255);
  for (Byte i = 0; i < n_objs; i++) {
    Byte obj_y = state.memory.f(objs_in_line[i]);
    Byte obj_x = state.memory.f(objs_in_line[i] + 1);
    Byte tile_data_index = state.memory.f(objs_in_line[i] + 2);
    Byte obj_attrs = state.memory.f(objs_in_line[i] + 3);
    std::array<Byte, 8> obj_line;
    bool priority = ((obj_attrs & 0x80) != 0);
    bool y_flip = ((obj_attrs & 0x40) != 0);
    bool x_flip = ((obj_attrs & 0x20) != 0);
    bool use_obp0 = ((obj_attrs & 0x10) == 0);
    Byte tile_line = line_n + 16 - obj_y;
    if (y_flip) {
      tile_line = obj_height - tile_line - 1;
    }
    getTileLine<true>(tile_data_index, tile_line, use_obp0, obj_line.data(), nullptr, state);
    for (Byte j = 0; j < 8; j++) {
      Byte n_px = obj_x + j - 8;
      // Check object is inside the viewport and there is no other higher priority object there
      if ((obj_x + j) >= 8 and (obj_x + j) < 168 and obj_x < px_written[n_px]) {
        if (not priority or bgw_line_idcs[n_px] == 0) {
          Byte px_color = (x_flip ? obj_line[7-j] : obj_line[j]);
          if (px_color != static_cast<Byte>(BWColors::TRANS)) {
            line[n_px] = px_color;
            px_written[n_px] = obj_x;
          }
        }
      }
    }
  }
}


void renderLineBGW (Byte line_n, ScreenLineData &bg_line, ScreenLineData &bgw_line_idcs, State &state)
{
  // If master BG and Window enable is off return white line
  if (not bgWinEnabled(state)) {
    bg_line.fill(static_cast<Byte>(BWColors::WHITE));
    bgw_line_idcs.fill(0);
    return;
  }

  Byte scy = state.memory.f(Addr::SCY); // Viewport Y position
  Byte scx = state.memory.f(Addr::SCX); // Viewport X position
  Byte bg_line_n = (scy + line_n)%256; // Y coord of the line relative to BG
  Byte tile_y = bg_line_n/8; // Y pos in the 32x32 bg's tile map
  Byte tile_line = bg_line_n%8; // Number of the line in the tile (0 to 7, both inclusive)
  Short addr_tile = (useBGTileMapHigh(state) ? 0x9C00 : 0x9800) + tile_y*32; // Addr of the tile on the left
  Byte wy = state.memory.f(Addr::WY);
  int  wx = state.memory.f(Addr::WX); // Needs to be signed to prevent underflow when doing - 7

  // Render background pixels until window found
  int scx_right = scx + SCREEN_PX_W;
  if (isWinEnabled(state) and wy <= line_n) {
    scx_right = scx + std::max(0, std::min(wx, int(SCREEN_PX_W + 7)) - 7);
  }
  int n_bg_px = scx_right - scx;
  int px_idx = scx;
  Byte *bg_line_ptr = &(bg_line[0]);
  Byte *bg_line_idcs_ptr = &(bgw_line_idcs[0]);
  int tile_i = scx/8;
  while (px_idx < scx_right) {
    Short tile_data_ind = state.memory.f(addr_tile + tile_i);
    Byte from = px_idx%8;
    Byte to = std::min(8, from + (scx_right - px_idx));
    getTileLine<false>(
      tile_data_ind, tile_line, false, bg_line_ptr, bg_line_idcs_ptr, state, from, to
    );
    Byte px_read = to - from;
    bg_line_ptr += px_read;
    bg_line_idcs_ptr += px_read;
    px_idx += px_read;
    tile_i = (tile_i + 1)%32;
  }

  // Render window (if any)
  const int total_win_px = SCREEN_PX_W + int(scx) - px_idx;
  if (total_win_px != 0) {
    Byte tile_y = state.screen.window_y/8;
    Byte tile_line = state.screen.window_y%8;
    addr_tile = (winUseTileMapHigh(state) ? 0x9C00 : 0x9800) + tile_y*32;
    int win_px = std::max(0, 7 - wx);
    const int win_end = win_px + total_win_px;
    while (win_px < win_end) {
      Short tile_data_ind = state.memory.f(addr_tile + win_px/8);
      Byte from = win_px%8;
      Byte to = std::min(8, from + (win_end - win_px));
      getTileLine<false>(
        tile_data_ind, tile_line, false, bg_line_ptr, bg_line_idcs_ptr, state, from, to
      );
      Byte px_read = to - from;
      bg_line_ptr += px_read;
      bg_line_idcs_ptr += px_read;
      win_px += px_read;
    }
    state.screen.window_y++;
  }
}


inline void renderLine (Byte line_n, State &state)
{
  state.screen.bgw_palette = colorsFromPalette<false>(state.memory.f(Addr::BGP));
  state.screen.obp0_palette = colorsFromPalette<true>(state.memory.f(Addr::OBP0));
  state.screen.obp1_palette = colorsFromPalette<true>(state.memory.f(Addr::OBP1));
  ScreenLineData &line_data = (*state.screen.line)[line_n].pixel;
  ScreenLineData bgw_line_idcs;
  renderLineBGW(line_n, line_data, bgw_line_idcs, state);
  renderLineOBJ(line_n, line_data, bgw_line_idcs, state);
}


void updateMemoryAccess (State &state, ScreenMode mode) {
  if (mode != state.screen.last_mode) {
    if (mode == ScreenMode::DRAW) {
      state.memory.setVRAMWriteEnabled(false);
      state.memory.setOAMWriteEnabled(false);
    }
    else {
      state.memory.setVRAMWriteEnabled(true);
      state.memory.setOAMWriteEnabled(mode != ScreenMode::OAMSC);
    }
  }
}


template<class InterfaceT>
inline void updateGraphics (State &state, InterfaceT &interface)
{
  if (not LCDEnabled(state)) {
    state.memory.f(Addr::STAT) &= 0xFC; // Clear bits 0 & 1, set mode 0
    state.memory.f(Addr::LY) = 0;
    return;
  }

  Byte line_n;
  ScreenMode mode = getMode(state, line_n);
  updateMemoryAccess(state, mode);
  setInterrupts(line_n, mode, state);
  updateLCDStatusRegs(line_n, mode, state);

  if (mode == ScreenMode::VBLANK) {
    state.screen.window_y = 0;
  }

  // Check if a new line needs to be rendered
  ulong current_frame = state.timing.cycles/DOTS_PER_FRAME;
  if (mode == ScreenMode::HBLANK and current_frame != (*state.screen.line)[line_n].frame_last_updated) {
    renderLine(line_n, state);
    // If rendered last line of frame and frame has been rendered from line 0, call screen update
    if (line_n == SCREEN_PX_H-1 and current_frame == (*state.screen.line)[0].frame_last_updated) {
      state.screen.line = interface.updateScreen();
      for (auto &l : *state.screen.line) {
        l.frame_last_updated = 0;
      }
    }
    (*state.screen.line)[line_n].frame_last_updated = current_frame;
  }
}

} // namespace gb