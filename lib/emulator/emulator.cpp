#include <limits>
#include "emulator.h"
#include "types.h"
#include "state.h"
#include "interface.h"
#include "cpu/cpu.h"
#include "utils/initialization.h"
#include "utils/debug.h"
#include "audio/audio.h"


void emulator (Interface *interface, GameRom *game_rom, EmulatorConfig cfg)
{
  State *state = new State;
  state->t_init_emulation = interface->realTimeMicros();
  state->config.debug = cfg.debug;
  state->config.target_speed = cfg.synch_execution ? 1 : std::numeric_limits<float>::max();
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
    execute(state, interface, edb);
  }
  replaceBootRom(state, game_rom);
  // Run game
  if (cfg.debug) {
    runInDebug(state, interface);
  } else {
    execute(state, interface);
  }
}