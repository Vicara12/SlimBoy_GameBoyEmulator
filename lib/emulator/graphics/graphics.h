/*
* Functions for obtaining the color of each pixel in the screen (used to render graphics)
*/

#pragma once

#include "types.h"
#include "state.h"
#include "interface.h"
#include "graphics/graphicstate.h"


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
  ulong current_dot = state.cycles%DOTS_PER_FRAME;
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


inline PaletteColors colorsFromPalette(Byte palette, bool object)
{
  PaletteColors colors = {
    Byte(object ? static_cast<Byte>(BWColors::TRANS) : (palette & 0x03)),
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
inline void getTileLine (
  Byte tile_data_index,
  Byte tile_line,
  bool is_object,
  bool use_obp0,
  Byte *dst,
  Byte *color_ids,
  State &state
)
{
  Short tile_data_address;
  // Compute tile data address from Block 0 (0x8000 - 0x87FF) and Block 1 (0x8800 - 0x8FFF)
  if (is_object or useBGWTileDataAreaLow(state)) {
    if (is_object and objSizeBig(state)) {
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
  std::array<Byte,2> tile_data;
  tile_data[0] = state.memory.f(tile_data_address + tile_line*2);
  tile_data[1] = state.memory.f(tile_data_address + tile_line*2 + 1);
  std::array<Byte,8> palette_ids;
  for (Byte i = 0; i < 8; i++) {
    Byte msb = ((tile_data[1] & (1 << (7-i))) != 0); // 7-i because lsb is tile's rightmost pixel
    Byte lsb = ((tile_data[0] & (1 << (7-i))) != 0);
    palette_ids[i] = msb*2 + lsb;
  }
  // Transform palette indices to actual colors
  Byte palette;
  if (is_object) {
    palette = state.memory.f(use_obp0 ? Addr::OBP0 : Addr::OBP1);
  } else {
    palette = state.memory.f(Addr::BGP);
  }
  PaletteColors colors = colorsFromPalette(palette, is_object);
  for (Byte i = 0; i < 8; i++) {
    dst[i] = colors[palette_ids[i]];
  }
  // Color/palette ids are needed to determine color when OBJ priority is false
  if (color_ids != nullptr) {
    for (Byte i = 0; i < 8; i++) {
      color_ids[i] = palette_ids[i];
    }
  }
}


// Returns a tuple of two arrays: the first contains colors and the second wether it has priority
inline std::tuple<std::array<Byte, SCREEN_PX_W>, std::array<bool, SCREEN_PX_W>>
renderLineOBJ (Byte line_n, State &state)
{
  std::array<Byte, SCREEN_PX_W> line_obj;
  std::array<bool, SCREEN_PX_W> pixel_priority;
  line_obj.fill(static_cast<Byte>(BWColors::TRANS));
  pixel_priority.fill(false);

  if (not objEnabled(state)) {
    return {line_obj, pixel_priority};
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
    getTileLine(tile_data_index, tile_line, true, use_obp0, obj_line.data(), nullptr, state);
    for (Byte j = 0; j < 8; j++) {
      Byte n_px = obj_x + j - 8;
      // Check object is inside the viewport and there is no other higher priority object there
      if ((obj_x + j) >= 8 and (obj_x + j) < 168 and obj_x < px_written[n_px]) {
        Byte px_color = (x_flip ? obj_line[7-j] : obj_line[j]);
        if (px_color != static_cast<Byte>(BWColors::TRANS)) {
          line_obj[n_px] = px_color;
          pixel_priority[n_px] = priority;
          px_written[n_px] = obj_x;
        }
      }
    }
  }
  return {line_obj, pixel_priority};
}


inline std::tuple<std::array<Byte, SCREEN_PX_W>, std::array<Byte, SCREEN_PX_W>>
renderLineWindow (Byte window_line_n, State &state)
{
  std::array<Byte, SCREEN_PX_W> win_line;
  std::array<Byte, SCREEN_PX_W> win_line_color_idcs;
  Byte tile_y = window_line_n/8;
  Byte tile_line = window_line_n%8;
  Short base_addr_win = (winUseTileMapHigh(state) ? 0x9C00 : 0x9800);
  Short addr_left_tile = base_addr_win + tile_y*32;
  for (Short i = 0; i < SCREEN_PX_W/8; i++) {
    Short tile_data_ind = state.memory.f(addr_left_tile + i);
    getTileLine(
      tile_data_ind, tile_line, false, false, &(win_line[0]) + i*8,
      &(win_line_color_idcs[0]) + i*8, state
    );
  }
  return {win_line, win_line_color_idcs};
}


inline std::tuple<std::array<Byte, SCREEN_PX_W>, std::array<Byte, SCREEN_PX_W>>
renderLineBGW (Byte line_n, State &state)
{
  // If master BG and Window enable is off return white line
  if (not bgWinEnabled(state)) {
    std::array<Byte,SCREEN_PX_W> bg_line;
    std::array<Byte,SCREEN_PX_W> bg_line_color_idcs;
    bg_line.fill(static_cast<Byte>(BWColors::WHITE));
    bg_line_color_idcs.fill(0);
    return {bg_line, bg_line_color_idcs};
  }

  // Get the entire BG screen line from the 256x256 map
  std::array<Byte,256> bg_line;
  std::array<Byte,256> bg_line_color_idcs;
  Byte scy = state.memory.f(Addr::SCY); // Viewport Y position
  Byte scx = state.memory.f(Addr::SCX); // Viewport X position
  Short base_addr_bg = (useBGTileMapHigh(state) ? 0x9C00 : 0x9800); // Select bg tile map area
  Byte bg_line_n = (scy + line_n)%256; // Y coord of the line relative to BG
  Byte tile_y = bg_line_n/8; // Y pos in the 32x32 bg's tile map
  Byte tile_line = bg_line_n%8; // Number of the line in the tile (0 to 7, both inclusive)
  Short addr_left_tile = base_addr_bg + tile_y*32; // Addr of the tile on the left
  for (Short i = 0; i < 32; i++) {
    Short tile_data_ind = state.memory.f(addr_left_tile + i);
    getTileLine(
      tile_data_ind, tile_line, false, false, &(bg_line[0]) + i*8,
      &(bg_line_color_idcs[0]) + i*8, state
    );
  }

  std::array<Byte, SCREEN_PX_W> bg_w_viewport_line;
  std::array<Byte, SCREEN_PX_W> bg_w_viewport_line_color_idcs;
  Byte wy = state.memory.f(Addr::WY);
  int  wx = state.memory.f(Addr::WX); // Needs to be signed to prevent underflow when doing - 7
  // If enabled, get the window pixels and merge them with BG. Otherwise just copy BG pixels.
  if (isWinEnabled(state) and wy <= line_n) {
    auto [win_line, win_line_color_idx] = renderLineWindow(state.screen.window_y, state);
    // Render normal screen pixels until window found
    for (int i = 0; i + 7 < std::min(wx, int(SCREEN_PX_W + 7)); i++) {
      bg_w_viewport_line[i] = bg_line[(i+scx)%256];
      bg_w_viewport_line_color_idcs[i] = bg_line_color_idcs[(i+scx)%256];
    }
    // Render window pixels
    int base_win_px = std::max(0, 7 - wx); // Idx of the leftmost pixel of the window shown
    int base_win_pos = std::max(0, wx - 7); // Position of the leftmost pixel of the window in screen
    // If there is something to draw from the window, increment window_y
    if (SCREEN_PX_W - wx + 7 > 0) {
      state.screen.window_y++;
    }
    for (int i = 0; i < std::clamp(SCREEN_PX_W + 7 - wx, 0u, uint(SCREEN_PX_W)); i++) {
      bg_w_viewport_line[base_win_pos + i] = win_line[base_win_px + i];
      bg_w_viewport_line_color_idcs[base_win_pos + i] = win_line_color_idx[base_win_px + i];
    }
  } else {
    for (int i = 0; i < SCREEN_PX_W; i++) {
      bg_w_viewport_line[i] = bg_line[(i+scx)%256];
      bg_w_viewport_line_color_idcs[i] = bg_line_color_idcs[(i+scx)%256];
    }
  }

  return {bg_w_viewport_line, bg_w_viewport_line_color_idcs};
}


inline float colorNumToFloat (Byte color)
{
  switch (color) {
    case static_cast<Byte>(BWColors::WHITE):
      return 1.00;
    case static_cast<Byte>(BWColors::LGRAY):
      return 0.66;
    case static_cast<Byte>(BWColors::DGRAY):
      return 0.33;
    case static_cast<Byte>(BWColors::BLACK):
      return 0.00;
  }
  return 0.00;
}


inline void renderLine (Byte line_n, State &state)
{
  auto [obj_line, obj_priority] = renderLineOBJ(line_n, state);
  auto [bgw_line, bgw_line_idcs] = renderLineBGW(line_n, state);
  // Merge BG, Window and Object pixels and write to screen as float intensity
  for (Byte i = 0; i < SCREEN_PX_W; i++) {
    // If priority then BG and Window colors != 0 draw over OBJ
    if (obj_priority[i]) {
      Byte pixel_color = (obj_line[i] == static_cast<Byte>(BWColors::TRANS) or bgw_line_idcs[i] != 0 ? bgw_line[i] : obj_line[i]);
      (*state.screen.line)[line_n].pixel[i] = colorNumToFloat(pixel_color);
    } else {
      Byte pixel_color = (obj_line[i] == static_cast<Byte>(BWColors::TRANS) ? bgw_line[i] : obj_line[i]);
      (*state.screen.line)[line_n].pixel[i] = colorNumToFloat(pixel_color);
    }
  }
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
  ulong current_frame = state.cycles/DOTS_PER_FRAME;
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