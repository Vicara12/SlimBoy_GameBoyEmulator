#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include "utils/debuginstr.h"
#include "interface.h"
#include "state.h"
#include "types.h"


namespace gb {

struct CartridgeInfo {
  std::string game_title = "";
  std::string manufacturer = "";
  Byte cartridge_type;
  Byte version_number;
  bool japanese;
  Memory::CartHardware hardware;
};


template<class InterfaceT>
inline void printCartridgeInfo (const CartridgeInfo &info, InterfaceT &interface) {
  interface.println("Title: " + info.game_title);
  interface.println("Version: " + info.version_number);
  interface.println("Dest.: " + std::string(info.japanese ? "Japan" : "Overseas"));
  interface.println("Manufacturer: " + info.manufacturer);
  interface.println("Num. ROM banks: " + info.hardware.n_rom_banks);
  interface.println("Num. RAM banks: " + info.hardware.n_ram_banks);
  interface.println(
    "MBC hardware: " + Memory::ctrlTypeToStr(info.hardware.controller) +
    (info.hardware.ram ? " + RAM" : "") + (info.hardware.battery ? " + BATTERY" : "") +
    (info.hardware.battery ? " + TIMER" : "")
  );
}


inline int numRomBanks (Byte val_addr_0148) {
  if (val_addr_0148 < 0x09)
    return 2 << val_addr_0148;
  if (val_addr_0148 == 0x52) return 72;
  if (val_addr_0148 == 0x53) return 80;
  if (val_addr_0148 == 0x54) return 96;
  throw std::runtime_error(
    "Unrecognized byte value for ROM size (addr 0x0148): " + formatByte(val_addr_0148)
  );
}


inline int numRamBanks (Byte val_addr_0149) {
  switch (val_addr_0149) {
    case 0x00:
    case 0x01:
      return 0;
    case 0x02:
      return 1;
    case 0x03:
      return 4;
    case 0x04:
      return 16;
    case 0x05:
      return 8;
  }
  throw std::runtime_error(
    "Unrecognized byte value for RAM size (addr 0x0149): " + formatByte(val_addr_0149)
  );
}


inline Memory::CartHardware parseType(Byte val_addr_0147) {
  switch (val_addr_0147) {
    case 0x00: return Memory::CartHardware{.controller = Memory::CtrlType::None};
    
    case 0x01: return Memory::CartHardware{.controller = Memory::CtrlType::MBC1};
    case 0x02: return Memory::CartHardware{.controller = Memory::CtrlType::MBC1, .ram = true};
    case 0x03: return Memory::CartHardware{.controller = Memory::CtrlType::MBC1, .ram = true, .battery = true};
    
    case 0x05: return Memory::CartHardware{.controller = Memory::CtrlType::MBC2};
    case 0x06: return Memory::CartHardware{.controller = Memory::CtrlType::MBC2, .battery = true};
    
    case 0x08: return Memory::CartHardware{.controller = Memory::CtrlType::None, .ram = true};
    case 0x09: return Memory::CartHardware{.controller = Memory::CtrlType::None, .ram = true, .battery = true};
    
    case 0x0B: return Memory::CartHardware{.controller = Memory::CtrlType::MMM01};
    case 0x0C: return Memory::CartHardware{.controller = Memory::CtrlType::MMM01, .ram = true};
    case 0x0D: return Memory::CartHardware{.controller = Memory::CtrlType::MMM01, .ram = true, .battery = true};
    
    case 0x0F: return Memory::CartHardware{.controller = Memory::CtrlType::MBC3, .battery = true, .timer = true};
    case 0x10: return Memory::CartHardware{.controller = Memory::CtrlType::MBC3, .ram = true, .battery = true, .timer = true};
    case 0x11: return Memory::CartHardware{.controller = Memory::CtrlType::MBC3};
    case 0x12: return Memory::CartHardware{.controller = Memory::CtrlType::MBC3, .ram = true};
    case 0x13: return Memory::CartHardware{.controller = Memory::CtrlType::MBC3, .ram = true, .battery = true};
    
    case 0x19: return Memory::CartHardware{.controller = Memory::CtrlType::MBC5};
    case 0x1A: return Memory::CartHardware{.controller = Memory::CtrlType::MBC5, .ram = true};
    case 0x1B: return Memory::CartHardware{.controller = Memory::CtrlType::MBC5, .ram = true, .battery = true};
    // These implement RUMBLE, which is not implemented
    // case 0x1C: return Memory::CartHardware{.controller = Memory::CtrlType::MBC5};
    // case 0x1D: return Memory::CartHardware{.controller = Memory::CtrlType::MBC5, .ram = true};
    // case 0x1E: return Memory::CartHardware{.controller = Memory::CtrlType::MBC5, .ram = true, .battery = true};
    
    case 0x20: return Memory::CartHardware{.controller = Memory::CtrlType::MBC6};
    
    case 0x22: return Memory::CartHardware{.controller = Memory::CtrlType::MBC7, .ram = true, .battery = true};
    
    case 0xFE: return Memory::CartHardware{.controller = Memory::CtrlType::HuC3};
    
    case 0xFF: return Memory::CartHardware{.controller = Memory::CtrlType::HuC1, .ram = true, .battery = true};
  }
  
  throw std::runtime_error(
    "Unrecognized byte value for cartridge type (addr 0x0147): 0x{:02x}" + formatByte(val_addr_0147)
  );
}


inline CartridgeInfo parseHeader (const std::vector<Byte> &cartridge) {
  CartridgeInfo info;
  for (size_t i = 0x0134; i <= 0x0143; i++) {
    if (cartridge[i] == 0x00)
      break;
    info.game_title.push_back(static_cast<char>(cartridge[i]));
  }

  if (info.game_title.length() < 0x013F - 0x0134) {
    for (size_t i = 0x013F; i <= 0x0142; i++) {
      if (cartridge[i] == 0x00)
        break;
      info.manufacturer.push_back(static_cast<char>(cartridge[i]));
    }
  }

  info.cartridge_type = cartridge[0x0147];
  info.japanese = (cartridge[0x014A] == 0x00);
  info.version_number = cartridge[0x014C];
  info.hardware = parseType(cartridge[0x0147]);
  info.hardware.n_rom_banks = numRomBanks(cartridge[0x0148]);
  info.hardware.n_ram_banks = numRamBanks(cartridge[0x0149]);
  return info;
}


inline CartridgeInfo loadGame (const std::vector<Byte> &cartridge, State &state) {
  if (cartridge.size() < 32*1024) {
    throw std::runtime_error("Cartridge is smaller than 32Kb: {} bytes" + std::to_string(cartridge.size()));
  }

  CartridgeInfo cart_info = parseHeader(cartridge);
  size_t expected_cart_size = (1 << 14) * cart_info.hardware.n_rom_banks;
  if (cartridge.size() != expected_cart_size) {
    throw std::runtime_error(
        "Expected cartridge of size " + std::to_string(expected_cart_size) + " bytes but got " +
        std::to_string(cartridge.size()) + " bytes"
    );
  }

  state.memory.initialize(cartridge, cart_info.hardware);
  return cart_info;
}

} // namespace gb