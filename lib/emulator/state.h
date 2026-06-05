
/*
* Code for everything related to memory and registers
*/

#pragma once

#include "memory/memory.h"
#include "graphics/graphicstate.h"
#include "audio/audiostate.h"
#include "cpu/timingstate.h"


namespace gb {

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
  Memory memory;
  bool halted = false;
  bool stopped = false;
  bool ime = false; // Interrupt Master Enable cpu flag
  Timing timing;
  ScreenFrame screen;
  AudioState audio;
  float target_speed = 1.0;
  bool end_emulation = false;
};

} // namespace gb