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

  enum class CtrlType {None, MBC1, MBC2, MBC3, MBC5, MBC6, MBC7, MMM01, HuC1, HuC3};

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

  static constexpr size_t MEM_BLOCK_SIZE = 8*1024;
  static constexpr size_t NUM_MEM_BLOCKS = 0x10000/MEM_BLOCK_SIZE;
  static constexpr size_t MEM_BLOCK_MASK = MEM_BLOCK_SIZE-1;

  using MemBlock = std::array<Byte,MEM_BLOCK_SIZE>;


  std::array<MemBlock*,NUM_MEM_BLOCKS> memory;
  std::optional<std::array<Byte,0x0100>> game_boot_rom;
  std::array<bool,0x0100> special_addr_written = {false};
  std::vector<MemBlock> rom_bank;
  std::vector<MemBlock> ram_bank;
  Byte rom_bank_idx_low = 1; // Lower 5 bits of rom bank
  Byte rom_ram_bank_idx = 0; // Upper 2 bits of rom bank or ram bank idx
  bool ram_mode_select = false;
  bool ram_enabled = false;
  bool vram_w_enabled = false;
  bool oam_w_enabled = false;
  bool lcd_enabled = false;
  CartHardware hardware;


  inline void performDMATransfer (Byte data) {
    Short base_addr = Short(data) << 8;
    for (Short i = 0; i < 0x00A0; i++) {
      f(Addr::OAM + i) = f(base_addr + i);
    }
  }


  inline void changeMemoryBanks (Short addr, Byte data) {
    // RAM enable
    if (addr < 0x2000) {
      ram_enabled = ((data & 0x0F) == 0x0A);
    }
    // ROM bank number
    else if (addr < 0x4000) {
      data &= 0x1F;
      data += (data == 0); // If 0 increment by one
      rom_bank_idx_low = data;
    }
    // RAM bank number or upper bits of ROM bank number
    else if (addr < 0x6000) {
      rom_ram_bank_idx = (data & 0x03);
    }
    // ROM/RAM mode select
    else {
      ram_mode_select = (data & 0x01);
    }

    Byte first_rom_bank_idx = 0, second_rom_bank_idx, ram_bank_idx = 0;
    second_rom_bank_idx = (rom_ram_bank_idx << 5) | rom_bank_idx_low;
    second_rom_bank_idx = second_rom_bank_idx % hardware.n_rom_banks;
    if (ram_mode_select) {
      first_rom_bank_idx = rom_ram_bank_idx << 5;
      first_rom_bank_idx = first_rom_bank_idx % hardware.n_rom_banks;
      ram_bank_idx = rom_ram_bank_idx;
    }

    if (hardware.n_ram_banks != 0) {
      ram_bank_idx = (ram_bank_idx) % hardware.n_ram_banks;
    } else {
      ram_bank_idx = 0;
    }
    
    // Hook low ROM bank
    memory[0] = &rom_bank[2*first_rom_bank_idx];
    memory[1] = &rom_bank[2*first_rom_bank_idx + 1];
    // Hook high ROM bank
    memory[2] = &rom_bank[2*second_rom_bank_idx];
    memory[3] = &rom_bank[2*second_rom_bank_idx + 1];
    // Hook RAM bank
    memory[Addr::ExtRAM/MEM_BLOCK_SIZE] = &ram_bank[ram_bank_idx];
  }


public:

  inline ~Memory () {
    for (size_t idx = 4; idx < NUM_MEM_BLOCKS; idx++) {
      if (idx != Addr::ExtRAM/MEM_BLOCK_SIZE) {
        delete memory[idx];
      }
    }
  }

  inline void initialize (const std::vector<Byte> &game_rom, const CartHardware &hardware) {
    if (
      hardware.controller != CtrlType::None and
      hardware.controller != CtrlType::MBC1
    ) {
      throw std::runtime_error(std::format(
        "Bank controller type {} is not implemented",
        ctrlTypeToStr(hardware.controller)
        )
      );
    }

    game_boot_rom = std::array<Byte,0x0100>();
    rom_bank.resize(2*hardware.n_rom_banks); // A ROM bank is 16kb, but a memory block is 8kb
    ram_bank.resize(std::max(hardware.n_ram_banks,1));

    // ROM bank 0
    memory[0] = &rom_bank[0];
    memory[1] = &rom_bank[1];
    // R0M bank 1
    memory[2] = &rom_bank[2];
    memory[3] = &rom_bank[3];

    // External RAM
    memory[Addr::ExtRAM/MEM_BLOCK_SIZE] = &ram_bank[0];

    for (size_t idx = 4; idx < NUM_MEM_BLOCKS; idx++) {
      if (idx != Addr::ExtRAM/MEM_BLOCK_SIZE) {
        memory[idx] = new MemBlock;
      }
    }

    size_t game_rom_idx = 0;
    for (Short i = 0x0000; i < 0x0100; i++) {
      (*game_boot_rom)[i] = game_rom[game_rom_idx++];
      f(i) = BOOT_ROM_DATA[i];
    }
    for (Short i = 0x0100; i < MEM_BLOCK_SIZE; i++) {
      (*memory[0])[i] = game_rom[game_rom_idx++];
    }
    for (size_t block_idx = 1; block_idx < rom_bank.size(); block_idx++) {
      for (size_t addr = 0; addr < MEM_BLOCK_SIZE; addr++) {
        rom_bank[block_idx][addr] = game_rom[game_rom_idx++];
      }
    }
    this->hardware = hardware;
  }


  inline void replaceBootRom () {
    if (game_boot_rom.has_value()) {
      for (Short i = 0x0000; i < 0x0100; i++) {
        f(i) = (*game_boot_rom)[i];
      }
      game_boot_rom = std::nullopt;
    }
  }


  // Fast read/write, no checks or banks
  inline const Byte& f(Short addr) const {return (*memory[addr/MEM_BLOCK_SIZE])[addr&MEM_BLOCK_MASK];}

  inline Byte& f(Short addr) {return const_cast<Byte&>(std::as_const(*this).f(addr));}


  // Safe read
  inline Byte r(Short addr) const {return f(addr);}


  // Safe write
  inline void w(Short addr, Byte data) {
    // Prevent writing to ROM
    if (addr < 0x8000) {
      changeMemoryBanks(addr, data);
      return;
    }
    // If write in locked video memory region do not write
    if (lcd_enabled and (
        (not vram_w_enabled and addr/MEM_BLOCK_SIZE == 4) or // VRAM is the 4th memory block
        (not oam_w_enabled  and addr >= 0xFE00 and addr < 0xFEA0))) {
      return;
    }
    // Check write on external RAM (5th memory block)
    else if (not ram_enabled and addr/MEM_BLOCK_SIZE == 5) {
      return;
    }

    f(addr) = data;
    if      (addr == Addr::LCDC) {lcd_enabled = ((data & 0x80) != 0);}
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