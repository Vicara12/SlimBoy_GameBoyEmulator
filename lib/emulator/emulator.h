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
  bool synch_execution = true;
  bool skip_boot_room = false;
};


// Run the emulator (boot + run)
template<class InterfaceT, bool debug>
void emulator (InterfaceT &interface, const GameRom &game_rom, EmulatorConfig cfg) {
  // State is created in the heap because large stack variables can crash small systems
  State *state = new State;
  state->memory.initialize(game_rom);
  state->target_speed = cfg.synch_execution ? 1 : std::numeric_limits<float>::max();
  state->screen.line = interface.updateScreen();
  resetAudioBuffers(state->audio);
  if (cfg.skip_boot_room) {
    setPostBootState(*state);
    state->memory.replaceBootRom();
  }
  execute<InterfaceT, debug>(*state, interface);
  delete state;
  interface.informEmulationEnded();
}