/*
* Memory I/O related functions
*/

#pragma once

#include "types.h"
#include "generaldefines.h"
#include "state.h"


inline void writeMem (Short addr, Byte data, State* state)
{
  // Prevent writing to ROM
  if (addr < 0x8000) {
    return;
  }
  // If write in locked video memory region do not write
  if (LCDC_LCD_ENABLED(state) and (
      (not state->vram_write_enabled and addr >= 0x8000 and addr < 0xA000) or
      (not state->oam_write_enabled  and addr >= 0xFE00 and addr < 0xFEA0))) {
    return;
  }
  state->memory[addr] = data;
  // Check write on RAM, if so write both on orig and mirror
  if (addr >= 0xC000 and addr < 0xDE00) {
    state->memory[addr-0xC000+0xE000] = data;
  }
  else if (addr >= 0xE000 and addr < 0xFE00) {
    state->memory[addr-0xE000+0xC000] = data;
  }
  // Check audio channel registers written
  else if (addr == NR14_REGISTER) {
    state->audio.ch1.NRX4_written = true;
  }
  else if (addr == NR12_REGISTER) {
    state->audio.ch1.NRX2_written = true;
  }
  else if (addr == NR24_REGISTER) {
    state->audio.ch2.NRX4_written = true;
  }
  else if (addr == NR22_REGISTER) {
    state->audio.ch2.NRX2_written = true;
  }
  else if (addr == NR34_REGISTER) {
    state->audio.ch3.NR34_written = true;
  }
  else if (addr == NR43_REGISTER) {
    state->audio.ch4.NR43_written = true;
  }
  else if (addr == NR44_REGISTER) {
    state->audio.ch4.NR44_written = true;
  }
  else if (addr == BOOT_ROM_REGISTER and not state->boot_rom_replaced) {
    state->boot_rom_replaced = true;
    for (Short i = 0x0000; i < 0x0100; i++) {
      state->memory[i] = state->game_boot_rom[i];
    }
  }
  // If write to DIV (0xFF04) register, set its value to zero
  else if (addr == DIV_REGISTER) {
    state->cycles_last_DIV = state->cycles; // it will be set to zero next time its updated
  }
  // If write to TAC, change state with TIMA enabled/disabled and clock div
  else if (addr == TAC_REGISTER) {
    state->enable_TIMA = GET_TAC_ENABLE(data);
    state->cycles_div_TIMA = getDivFromTAC(data);
  }
  // Perform DMA data transfer if a value is written to DMA
  else if (addr == DMA_REGISTER) {
    Short base_addr = Short(data) << 8;
    for (Short i = 0; i < 0x00A0; i++) {
      state->memory[0xFE00 + i] = state->memory[base_addr + i];
    }
  }
}