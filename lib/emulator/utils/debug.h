/*
* Some debugging functionalities
*/

#pragma once

#include <sstream>
#include <iomanip>
#include "types.h"
#include "state.h"
#include "interface.h"
#include "instructions/instruction.h"


struct ExecutionDebug {
  Short breakpoint = 0xFFFF;
  int exec_n = -1;
  std::array<int, 3> rom_bp = {-1,-1,-1};
};


inline std::string formatByte (Byte b, bool inc_0x = false) {
  std::stringstream ss;
  // Convert b to short because for some reason it does not like uint8_t (Byte)
  ss << (inc_0x ? "0x" : "") << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << Short(b);
  return ss.str();
}


inline std::string formatShort (Short s, bool inc_0x = false) {
  std::stringstream ss;
  ss << (inc_0x ? "0x" : "") << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << s;
  return ss.str();
}


template<class InterfaceT>
void showRegisters (State *state, InterfaceT &interface) {
  interface.print("A  = " + formatByte(state->A ) + "    F  = " + formatByte(state->F) + "\n");
  interface.print("B  = " + formatByte(state->B ) + "    C  = " + formatByte(state->C) + "\n");
  interface.print("D  = " + formatByte(state->D ) + "    E  = " + formatByte(state->E) + "\n");
  interface.print("H  = " + formatByte(state->H ) + "    L  = " + formatByte(state->L) + "\n");
  interface.print("SP = " + formatShort(state->SP)  + "  PC = " + formatShort(state->PC) + "\n");
}


template<class InterfaceT>
void showMemoryRange (State *state, Short ini, Short fi, InterfaceT &interface) {
  for (int i = ini; i <= fi; i++) {
    interface.print("[" + formatShort(i) + "] = " + formatByte(state->memory[i]) + "\n");
  }
}


template<class InterfaceT>
State* runInDebug (State *state, InterfaceT &interface) {
  ExecutionDebug edb;
  edb.breakpoint = 0xFFFF;
  edb.exec_n = -1;
  edb.rom_bp = {-1,-1,-1};
  bool exit = false;
  bool first = true;
  auto formatRBP = [] (const ExecutionDebug &edb) -> std::string {
    return (edb.rom_bp[0] == -1 ? "" : formatByte(edb.rom_bp[0])) +
           (edb.rom_bp[1] == -1 ? "" : formatByte(edb.rom_bp[1])) +
           (edb.rom_bp[2] == -1 ? "" : formatByte(edb.rom_bp[2]));
  };

  while (not exit) {
    interface.print("\n");
    if (first) {
      first = false;
    } else {
      execute(state, interface, edb);
    }
    edb.exec_n = -1;
    bool run = false;
    while (not run) {
      interface.print("\n >>> ");
      std::string user_input = interface.userLineInput();
      std::stringstream ss(user_input);
      std::string option;
      std::vector<int> params;
      ss >> option;
      int data;
      while (ss >> std::hex >> data) {
        params.push_back(data);
      }
      if (option == "r") {
        run = true;
        if (not params.empty()) {
          edb.exec_n = params[0];
        }
      }
      else if (option == "n") {
        edb.exec_n = 1;
        run = true;
      }
      else if (option == "b") {
        edb.breakpoint = params[0];
        interface.print("Set breakpoint at PC=" + formatShort(edb.breakpoint) + "\n");
      }
      else if (option == "m") {
        interface.print("[" + formatShort(params[0]) + "] = " + formatByte(state->memory[params[0]]));
      }
      else if (option == "n") {
        showMemoryRange(state, params[0], params[1], interface);
      }
      else if (option == "q") {
        exit = true;
        run = true;
      }
      else if (option == "t") {
        state->config.debug = not state->config.debug;
        if (state->config.debug) {
          interface.print("Print instructions ON\n");
        } else {
          interface.print("Print instructions OFF\n");
        }
      }
      else if (option == "o") {
        edb.rom_bp[0] = params[0];
        if (params.size() > 1) {
          edb.rom_bp[1] = params[1];
        }
        if (params.size() > 2) {
          edb.rom_bp[2] = params[2];
        }
        interface.print("Set rom breakpoint for pattern " + formatRBP(edb) + "\n");
      }
      else if (option == "u") {
        edb.rom_bp[0] = -1;
        edb.rom_bp[1] = -1;
        edb.rom_bp[2] = -1;
        interface.print("Cleared rom pattern breakpoint\n");
      }
      else if (option == "i") {
        showRegisters(state, interface);
        interface.print("bp=" + formatShort(edb.breakpoint) + "  rbp=" + formatRBP(edb) + "\n");
      }
      else if (option == "h") {
        interface.print("   - r [opt HEX]: continue execution (or execute HEX instructions)\n");
        interface.print("   - n: execute one instruction\n");
        interface.print("   - b HEX: set the breakpoint at PC=HEX\n");
        interface.print("   - o HEX0 [opt HEX1] [opt HEX2]: set breakpoint when rom matches args\n");
        interface.print("   - u: clear rom data breakpoint (o)\n");
        interface.print("   - m HEX: show memory contents at HEX\n");
        interface.print("   - n HEX0 HEX1: show memory contents from HEX0 to HEX1 (inc)\n");
        interface.print("   - t: toggle instruction printing\n");
        interface.print("   - i: show register and debug info\n");
        interface.print("   - q: quit\n");
      }
      else {
        interface.print("Unknown option (" + option + "), enter h for a list of commands\n");
      }
    }
  }

  return state;
}