#include "cpu/cpu.h"
#include "instructions/instruction.h"
#include "graphics/graphics.h"
#include "audio/audio.h"
#include "utils/debuginstr.h"



void execute (State *state, Interface *interface, const ExecutionDebug &db)
{
  ulong n_instrs = 0;
  Byte opcode = 0x00, data0 = 0x00, data1 = 0x00;

  while (not state->config.end_emulation and n_instrs != db.exec_n) {
    if (not state->halted) {
      opcode = state->memory[state->PC];
      data0 = state->memory[(state->PC+1)&0xFFFF];
      data1 = state->memory[(state->PC+2)&0xFFFF];
      if (state->config.debug) {
        interface->print(cycleStr(opcode, data0, data1, state) + "\n");
      }
      state->PC += instrLen(opcode);
      state->cycles += executeInstruction(opcode, data0, data1, state);
    } else {
      state->cycles += 4; // Make clock work when halted
    }
    updateTimeRegisters(state);
    // Update buttons once normally and hang looking for button input if stopped
    do {
      updateButtons(n_instrs, state, interface);
      if (state->stopped) {
        interface->sleepMillis(10);
      }
    } while (state->stopped);
    updateGraphics(state, interface);
    updateAudio(state, interface);
    checkAndCallInterrupt(state);
    state->config.end_emulation = interface->endEmulation();
    synchExecution(state, interface);
    n_instrs++;

    if (state->PC == db.breakpoint or
        ((state->memory[state->PC] == db.rom_bp[0]) and
         (state->memory[(state->PC+1)&0xFFFF] == db.rom_bp[1] or db.rom_bp[1] == -1) and
         (state->memory[(state->PC+2)&0xFFFF] == db.rom_bp[2] or db.rom_bp[2] == -1))) {
      break;
    }
  }
}