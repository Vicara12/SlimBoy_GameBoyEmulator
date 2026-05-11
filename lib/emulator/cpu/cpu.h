/*
* Instruction execution handling
*/

#pragma once


#include <utility>
#include "state.h"
#include "interface.h"

#define INSTRS_PER_BUTTON_UPDATE 64 // Each X instructions the new buttons pressed check is run


typedef struct {
  Short breakpoint = 0xFFFF;
  int exec_n = -1;
  std::array<int, 3> rom_bp = {-1,-1,-1};
} ExecutionDebug;


void execute (State *state, Interface *interface, const ExecutionDebug &db = ExecutionDebug());


inline void updateTimeRegisters (State *state)
{
  state->memory[DIV_REGISTER] = (state->cycles - state->cycles_last_DIV)/256;
  // TODO: reset this register when the STOP mode ends
  if (state->enable_TIMA) {
    Byte old_TIMA = state->memory[TIMA_REGISTER];
    ulong t_value_TIMA = (state->cycles - state->cycles_last_TIMA)/state->cycles_div_TIMA;
    state->memory[TIMA_REGISTER] = t_value_TIMA + state->memory[TMA_REGISTER];
    // Detect TIMA overflow
    if (old_TIMA > state->memory[TIMA_REGISTER]) {
      // Account for overflowed cycles
      state->cycles_last_TIMA = state->cycles - state->memory[TIMA_REGISTER]*state->cycles_div_TIMA;
      state->memory[TIMA_REGISTER] += state->memory[TMA_REGISTER];
      SET_INTERRUPT(state, TIMER_INTERRUPT);
    }
  }
}


inline void updateButtons (ulong n_instrs, State *state, Interface *interface)
{
  Byte old_p1_reg = state->memory[P1_REGISTER];
  Byte button_inputs = interface->readButtons();
  Byte input = 0xF0;
  // Low at bit 5 indicates A/B/Sel/Start read
  if ((old_p1_reg & 0x20) == 0) {
    input |= (button_inputs >> 4);
  }
  // Low at bit 4 indicates U/D/L/R read
  if ((old_p1_reg & 0x10) == 0) {
    input |= (button_inputs & 0x0F);
  }
  // Button pressed means LOW (0) and vice versa
  state->memory[P1_REGISTER] = (old_p1_reg & 0x30) | (~input);

  // If any change in button pressed activate interrupt and exit stop state (if in it)
  if (((old_p1_reg ^ state->memory[P1_REGISTER]) & 0x0F) != 0) {
    SET_INTERRUPT(state, JOYPAD_INTERRUPT);
    state->stopped = false;
  }
}


inline void checkAndCallInterrupt (State *state)
{
  Byte active_interrupts = (state->memory[IF_REGISTER] & state->memory[IE_REGISTER]);
  if (state->halted and active_interrupts != 0) {
    state->halted = false;
  }
  
  // Check first if there is any interrupt or if they are enabled by the interrupt master enable
  if (active_interrupts == 0 or not state->ime) {
    return;
  };

  // Check interrupts one by one, if some interrupt is triggered return (don't check others)
  for (Byte i = 0; i < 5; i++) {
    Byte interrupt_flag = 1 << i;
    // If both IE and IF's corresponding bits are set to one
    if (active_interrupts & interrupt_flag) {
      state->ime = false;
      state->memory[IF_REGISTER] = state->memory[IF_REGISTER] & (~interrupt_flag); // clear interrupt bit
      // Push PC to stack
      state->memory[--state->SP] = state->PC >> 8;
      state->memory[--state->SP] = state->PC & 0xFF;
      // Set new value of PC
      switch (interrupt_flag)
      {
      case VBLANK_INTERRUPT:
        state->PC = 0x0040;
        break;
      case LCD_INTERRUPT:
        state->PC = 0x0048;
        break;
      case TIMER_INTERRUPT:
        state->PC = 0x0050;
        break;
      case SERIAL_INTERRUPT:
        state->PC = 0x0058;
        break;
      case JOYPAD_INTERRUPT:
        state->PC = 0x0060;
        break;
      default:
        break;
      }
      return;
    }
  }
}


inline void synchExecution (State *state, Interface *interface)
{
  ulong t_now = interface->realTimeMicros() - state->t_init_emulation;
  float emulator_time = float(state->cycles)/CLOCK_FREQ*1e6;
  float emulation_rate = emulator_time/t_now;
  // Update every 500 ms
  if (float(t_now - state->last_rate_call) > 500e3) {
    interface->informEmuRate(emulation_rate);
    state->last_rate_call = t_now;
  }
  float diff_ms = float(emulator_time - state->config.target_speed*t_now)/1e3;
  if (diff_ms > 1) {
    interface->sleepMillis(diff_ms);
  }
}