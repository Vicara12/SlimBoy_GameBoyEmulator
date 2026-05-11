/*
* Main file of the emulator
*/

#pragma once

#include "types.h"
#include "state.h"
#include "interface.h"


struct EmulatorConfig {
  bool debug = false;
  bool synch_execution = true;
  bool skip_boot_room = false;
};


// Run the emulator (boot + run)
void emulator (Interface *interface, GameRom *game_rom, EmulatorConfig cfg);