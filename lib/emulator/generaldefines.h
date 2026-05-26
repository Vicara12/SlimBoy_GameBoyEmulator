#pragma once

#include <array>
#include "types.h"



#define GB_MEM_SIZE 0x10000
#define CLOCK_FREQ 4194304


// Button utils
#define BUTTON_RIGHT  0x01
#define BUTTON_LEFT   0x02
#define BUTTON_UP     0x04
#define BUTTON_DOWN   0x08
#define BUTTON_A      0x10
#define BUTTON_B      0x20
#define BUTTON_SEL    0x40
#define BUTTON_START  0x80


// Special registers
#define P1_REGISTER   0xFF00 // Controller input data

#define DIV_REGISTER  0xFF04 // Clock which is updated at 16384Hz (one overflow per second)
#define TIMA_REGISTER 0xFF05 // Clock incremented at freq specified by TAC, at of TMA is written
#define TMA_REGISTER  0xFF06 // Value written to TIMA at overflow
#define TAC_REGISTER  0xFF07 // Timer control

#define BOOT_ROM_REGISTER 0xFF50 // Boot ROM mapping control

#define IF_REGISTER   0xFF0F // Interrupt flag
#define IE_REGISTER   0xFFFF // Interrupt Enable

#define GET_TAC_CLOCK_SEL(value_TAC) (value_TAC & 0x03)
#define GET_TAC_ENABLE(value_TAC)    ((value_TAC & 0x4) != 0)

#define VBLANK_INTERRUPT  0x01
#define LCD_INTERRUPT     0x02
#define TIMER_INTERRUPT   0x04
#define SERIAL_INTERRUPT  0x08
#define JOYPAD_INTERRUPT  0x10

#define SET_INTERRUPT(state, interrupt) (state->memory[IF_REGISTER] |= interrupt)

// Flag setting utils
#define ZERO_FLAG       0x80
#define SUBTRACT_FLAG   0x40
#define HALF_CARRY_FLAG 0x20
#define CARRY_FLAG      0x10

#define GET_ZERO_FLAG(state)       ((state->F & ZERO_FLAG) != 0)
#define GET_SUBTRACT_FLAG(state)   ((state->F & SUBTRACT_FLAG) != 0)
#define GET_HALF_CARRY_FLAG(state) ((state->F & HALF_CARRY_FLAG) != 0)
#define GET_CARRY_FLAG(state)      ((state->F & CARRY_FLAG) != 0)

#define CLEAR_ALL_FLAGS(state) state->F = 0x00

#define SET_ZERO_FLAG(state) state->F |= ZERO_FLAG
#define COND_SET_ZERO_FLAG(state, condition) state->F |= ZERO_FLAG*(condition)
#define RESET_ZERO_FLAG(state) state->F &= ~ZERO_FLAG

#define SET_SUBTRACT_FLAG(state) state->F |= SUBTRACT_FLAG
#define COND_SET_SUBTRACT_FLAG(state, condition) state->F |= SUBTRACT_FLAG*(condition)
#define RESET_SUBTRACT_FLAG(state) state->F &= ~SUBTRACT_FLAG

#define SET_HALF_CARRY_FLAG(state) state->F |= HALF_CARRY_FLAG
#define COND_SET_HALF_CARRY_FLAG(state, condition) state->F |= HALF_CARRY_FLAG*(condition)
#define RESET_HALF_CARRY_FLAG(state) state->F &= ~HALF_CARRY_FLAG

#define SET_CARRY_FLAG(state) state->F |= CARRY_FLAG
#define COND_SET_CARRY_FLAG(state, condition) state->F |= CARRY_FLAG*(condition)
#define RESET_CARRY_FLAG(state) state->F &= ~CARRY_FLAG

// Double register utils
#define JOIN_REGS(r1, r2) ((DReg(r1) << 8) | DReg(r2))
#define REG_AF(state) JOIN_REGS(state->A, state->F)
#define REG_BC(state) JOIN_REGS(state->B, state->C)
#define REG_DE(state) JOIN_REGS(state->D, state->E)
#define REG_HL(state) JOIN_REGS(state->H, state->L)

#define STORE_SHORT(value, dst_high, dst_low) dst_high = Byte(value >> 8); dst_low = Byte(value & 0xFF)
#define SET_REG_AF(value, state) STORE_SHORT(value, state->A, state->F)
#define SET_REG_BC(value, state) STORE_SHORT(value, state->B, state->C)
#define SET_REG_DE(value, state) STORE_SHORT(value, state->D, state->E)
#define SET_REG_HL(value, state) STORE_SHORT(value, state->H, state->L)

// Memory access utils
#define SET_INTERRUPT_STATUS(value, state) state->memory[IE_REGISTER] = value;