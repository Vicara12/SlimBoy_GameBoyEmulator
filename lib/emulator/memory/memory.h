/*
* Memory I/O related functions
*/

#pragma once

#include <vector>
#include <optional>
#include "types.h"


inline constexpr size_t GB_MEM_SIZE = 0x10000;


// Memory mapping
enum Addr : Short {
  // Banks
  ROM0    = 0x0000, ROM0_end    = 0x3FFF,
  ROMnn   = 0x4000, ROMnn_end   = 0x7FFF,
  VRAM    = 0x8000, VRAM_end    = 0x9FFF,
  ExtRAM  = 0xA000, ExtRAM_end  = 0xBFFF,
  WRAM    = 0xC000, WRAM_end    = 0xDDFF,
  EchoRAM = 0xE000, EchoRAM_end = 0xFDFF,
  OAM     = 0xFE00, OAM_end     = 0xFE9F,
  // 0xFEA0 - 0xFEFF not usable

  // I/O Registers
  P1   = 0xFF00, // Joypad input
  SB   = 0xFF01, // Serial transfer data
  SC   = 0xFF02, // Serial transfer control
  DIV  = 0xFF04, // Divider register
  TIMA = 0xFF05, // Timer counter
  TMA  = 0xFF06, // Timer modulo
  TAC  = 0xFF07, // Timer control
  IF   = 0xFF0F, // Interrupt flag
  NR10 = 0xFF10, // Channel 1 sweep
  NR11 = 0xFF11, // Channel 1 length timer & duty cycle
  NR12 = 0xFF12, // Channel 1 volume & envelope
  NR13 = 0xFF13, // Channel 1 period low
  NR14 = 0xFF14, // Channel 1 period high & control
  NR21 = 0xFF16, // Channel 2 length timer & duty cycle
  NR22 = 0xFF17, // Channel 2 volume & envelope
  NR23 = 0xFF18, // Channel 2 period low
  NR24 = 0xFF19, // Channel 2 period high & control
  NR30 = 0xFF1A, // Channel 3 DAC enable
  NR31 = 0xFF1B, // Channel 3 length timer
  NR32 = 0xFF1C, // Channel 3 output level
  NR33 = 0xFF1D, // Channel 3 period low
  NR34 = 0xFF1E, // Channel 3 period high & control
  NR41 = 0xFF20, // Channel 4 length timer
  NR42 = 0xFF21, // Channel 4 volume & envelope
  NR43 = 0xFF22, // Channel 4 frequency & randomness
  NR44 = 0xFF23, // Channel 4 control
  NR50 = 0xFF24, // Master volume & VIN panning
  NR51 = 0xFF25, // Sound planning
  NR52 = 0xFF26, // Audio master control
  WPRAM = 0xFF30, WPRAM_end = 0xFF3F, // Wave pattern RAM
  LCDC = 0xFF40, // LCD Control
  STAT = 0xFF41, // LCD Status
  SCY  = 0xFF42, // Background viewport top-left Y coord
  SCX  = 0xFF43, // Background viewport top-left X coord
  LY   = 0xFF44, // LCD Y coordinate
  LYC  = 0xFF45, // LY compare
  DMA  = 0xFF46, // Transfer from ROM/RAM to OAM
  BGP  = 0xFF47, // BG and Window palette data
  OBP0 = 0xFF48, // Object palette 0
  OBP1 = 0xFF49, // Object palette 1
  WY   = 0xFF4A, // Window Y position
  WX   = 0xFF4B, // Window X position
  BANK = 0xFF50, // Boot ROM mapping control

  HRAM = 0xFF80, HRAM_end = 0xFFFE, // High RAM

  IE = 0xFFFF // Interrupt Enable
};


// TODO support unmapped RAM bank access


class Memory {
public:

  enum class CtrlType {None, MBC1, MBC2, MBC3, MBC4, MBC5, MBC6, MBC7, MMM01, HuC1, HuC3};

  struct CartHardware {
    Memory::CtrlType controller;
    int n_rom_banks = 0;
    int n_ram_banks = 0;
    bool ram = false;
    bool battery = false;
    bool timer = false;
  };

  static inline std::string ctrlTypeToStr (CtrlType type) {
    switch (type) {
      case CtrlType::None:  return "ROM Only";
      case CtrlType::MBC1:  return "MBC1";
      case CtrlType::MBC2:  return "MBC2";
      case CtrlType::MBC3:  return "MBC3";
      case CtrlType::MBC4:  return "MBC4";
      case CtrlType::MBC5:  return "MBC5";
      case CtrlType::MBC6:  return "MBC6";
      case CtrlType::MBC7:  return "MBC7";
      case CtrlType::MMM01: return "MMM01";
      case CtrlType::HuC1:  return "HuC1";
      case CtrlType::HuC3:  return "HuC3";
    }
    return "Unknown";
  }

private:

  static constexpr Byte BOOT_ROM_DATA [] = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
  };

  std::array<Byte,GB_MEM_SIZE> memory;
  std::optional<std::array<Byte,0x0100>> game_boot_rom;
  std::array<bool,0x0100> special_addr_written = {false};
  bool vram_w_enabled = false;
  bool oam_w_enabled = false;
  bool lcd_enabled = false;


  inline void performDMATransfer (Byte data) {
    // TODO select DMA bank!!!!
    Short base_addr = Short(data) << 8;
    for (Short i = 0; i < 0x00A0; i++) {
      memory[Addr::OAM + i] = memory[base_addr + i];
    }
  }

public:

  inline bool initialize (const std::vector<Byte> &game_rom, const CartHardware &hardware) {
    game_boot_rom = std::array<Byte,0x0100>();
    for (Short i = 0x0000; i < 0x0100; i++) {
      (*game_boot_rom)[i] = game_rom[i];
      memory[i] = BOOT_ROM_DATA[i];
    }
    for (Short i = 0x0100; i < 0x8000; i++) {
      memory[i] = game_rom[i];
    }
    return true;
  }


  inline void replaceBootRom () {
    if (game_boot_rom.has_value()) {
      for (Short i = 0x0000; i < 0x0100; i++) {
        memory[i] = (*game_boot_rom)[i];
      }
      game_boot_rom = std::nullopt;
    }
  }


  // Fast read/write, no checks or banks
  inline Byte& f(Short addr) {return memory[addr];}


  // Safe read
  inline Byte r(Short addr) const {return memory[addr];}


  // Safe write
  inline void w(Short addr, Byte data) {
    // Prevent writing to ROM
    if (addr < 0x8000) {
      return;
    }
    // If write in locked video memory region do not write
    if (lcd_enabled and (
        (not vram_w_enabled and addr >= 0x8000 and addr < 0xA000) or
        (not oam_w_enabled  and addr >= 0xFE00 and addr < 0xFEA0))) {
      return;
    }
    memory[addr] = data;
    // Check write on RAM, if so write both on orig and mirror
    if      (addr >= 0xC000 and addr < 0xDE00) {memory[addr-0xC000+0xE000] = data;}
    else if (addr >= 0xE000 and addr < 0xFE00) {memory[addr-0xE000+0xC000] = data;}
    else if (addr == Addr::LCDC) {lcd_enabled = ((data & 0x80) != 0);}
    else if (addr == Addr::BANK) {replaceBootRom();}
    else if (addr == Addr::DMA ) {performDMATransfer(data);}
    else if (addr >= 0xFF00) {special_addr_written[addr & 0xFF] = true;}
    // TODO optimize with a switch
  }

  inline bool specialAddrWritten (Short addr) {
    bool result = special_addr_written[addr & 0xFF];
    special_addr_written[addr & 0xFF] = false;
    return result;
  }

  inline bool isLCDEnabled () const {return lcd_enabled;}
  inline void setVRAMWriteEnabled (bool state) {vram_w_enabled = state;}
  inline void setOAMWriteEnabled (bool state) {oam_w_enabled = state;}
};