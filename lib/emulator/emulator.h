/*
* Main file of the emulator
*/

#pragma once

#include "types.h"
#include "state.h"
#include "interface.h"
#include "audio/audio.h"
#include "utils/initialization.h"
#include "cpu/cpu.h"


struct EmulatorConfig {
  bool debug = false;
  bool synch_execution = true;
  bool skip_boot_room = false;
};


// Run the emulator (boot + run)
template<class InterfaceT>
void emulator (InterfaceT &interface, GameRom *game_rom, EmulatorConfig cfg) {
  State *state = new State;
  state->t_init_emulation = interface.realTimeMicros();
  state->config.debug = cfg.debug;
  state->config.target_speed = cfg.synch_execution ? 1 : std::numeric_limits<float>::max();
  state->screen.line = interface.updateScreen();
  resetAudioBuffers(state->audio);
  ExecutionDebug edb;
  edb.breakpoint = 0x0100; // Set breakpoint at the end of boot rom
  if (cfg.skip_boot_room) {
    loadGame(state, game_rom);
    setPostBootState(state);
  } else {
    // Boot sequence
    loadBootRom(state);
    loadGame(state, game_rom);
    execute<InterfaceT>(state, interface, edb);
  }
  replaceBootRom(state, game_rom);
  // Run game
  if (cfg.debug) {
    runInDebug<InterfaceT>(state, interface);
  } else {
    execute<InterfaceT>(state, interface);
  }
}