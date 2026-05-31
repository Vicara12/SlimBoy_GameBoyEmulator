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
#include "cartridge/cartridge.h"


struct EmulatorConfig {
  bool synch_execution = true;
  bool skip_boot_room = false;
};


// Run the emulator (boot + run)
template<class InterfaceT, bool debug>
void emulator (InterfaceT &interface, const GameRom &cartridge_data, EmulatorConfig cfg) {
  try {
    // State is created in the heap because large stack variables can crash small systems
    State *state = new State;
    CartridgeInfo cart_info = loadGame(cartridge_data, *state);
    printCartridgeInfo(cart_info, interface);
    state->target_speed = cfg.synch_execution ? 1 : std::numeric_limits<float>::max();
    state->screen.line = interface.updateScreen();
    resetAudioBuffers(state->audio);
    if (cfg.skip_boot_room) {
      setPostBootState(*state);
      state->memory.replaceBootRom();
    }
    execute<InterfaceT, debug>(*state, interface);
    if (cart_info.hardware.battery) {
      interface.saveRAM(state->memory.copyRAM());
    }
    delete state;
    interface.informEmulationEnded();
  } catch (const std::exception &e) {
    interface.println(std::format("ERROR: {}", e.what()));
    interface.informEmulationEnded();
  }
}