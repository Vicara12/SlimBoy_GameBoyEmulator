/*
* Instruction execution handling
*/

#pragma once

#include <utility>
#include "state.h"
#include "interface.h"
#include "utils/debuginstr.h"
#include "instructions/instruction.h"
#include "graphics/graphics.h"
#include "audio/audio.h"
#include "buttoninputs.h"
#include "timing.h"



template<class InterfaceT, bool debug>
void execute (State &state, InterfaceT &interface) {
  ulong n_instrs = 0;
  Byte opcode = 0x00, data0 = 0x00, data1 = 0x00;

  while (not state.end_emulation) {
    if (not state.halted) {
      opcode = state.memory.r(state.PC);
      data0  = state.memory.r((state.PC+1)&0xFFFF);
      data1  = state.memory.r((state.PC+2)&0xFFFF);
      if constexpr (debug) {
        interface.print(cycleStr(opcode, data0, data1, state) + "\n");
      }
      state.PC += instrLen(opcode);
      state.cycles += executeInstruction(opcode, data0, data1, state);
    } else {
      state.cycles += 4; // Make clock work when halted
    }
    // Update buttons once normally and hang looking for button input if stopped
    do {
      // This is here so that emulator doesn't freeze on stop
      state.end_emulation = interface.endEmulation();
      updateButtons(n_instrs, state, interface);
      if (state.stopped) {
        interface.sleepMillis(10);
        state.memory.w(Addr::DIV, 0); // Write something to DIV so that it resets
      }
    } while (state.stopped and not state.end_emulation);
    updateTimeRegisters(state);
    updateGraphics(state, interface);
    updateAudio(state, interface);
    checkAndCallInterrupt(state);
    synchExecution(state, interface);
    n_instrs++;
  }
}