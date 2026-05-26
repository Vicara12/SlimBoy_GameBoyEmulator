
/*
* Code for everything related to memory and registers
*/

#pragma once

#include "types.h"
#include "interface.h"
#include "generaldefines.h"
#include "graphics/graphicstate.h"
#include "audio/audiostate.h"


inline ulong getDivFromTAC (Byte value_TAC) {
  switch (GET_TAC_CLOCK_SEL(value_TAC))
  {
  case 0b00:
    return (1 << 10);
  case 0b01:
    return (1 << 4);
  case 0b10:
    return (1 << 6);
  case 0b11:
    return (1 << 8);
  default:
    break;
  }
  return 1;
}


struct InternalConfig {
  float target_speed = 1.0;
  bool debug = false;
  bool end_emulation = false;
};


struct State {
  Reg A = 0;
  Reg B = 0;
  Reg C = 0;
  Reg D = 0;
  Reg E = 0;
  Reg F = 0;
  Reg H = 0;
  Reg L = 0;
  DReg SP = 0xFFFE;
  DReg PC = 0x0000;
  Byte memory[GB_MEM_SIZE] = {0};
  bool halted = false;
  bool stopped = false;
  bool ime = false; // Interrupt Master Enable cpu flag
  ulong cycles = 0; // Total execution cycles (execution clock)
  ulong cycles_last_DIV = 0;  // Counts the execution cycles of the last write to DIV
  ulong cycles_last_TIMA = 0; // Cycles of last TIMA overflow
  ulong cycles_div_TIMA = getDivFromTAC(0);
  bool enable_TIMA = false;
  bool vram_write_enabled = true;
  bool oam_write_enabled = true;
  ulong last_rate_call = 0; // Last time interface->informEmuRate was called
  ulong t_init_emulation;
  ScreenFrame screen;
  AudioState audio;
  InternalConfig config;
};