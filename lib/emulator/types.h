#pragma once

#include <cstdint>
#include <vector>


namespace gb {

using Reg = uint8_t;
using Byte = uint8_t;
using SByte = int8_t;
using DReg = uint16_t;
using Short = uint16_t;

using ulong = unsigned long;

using GameRom = std::vector<Byte>;

} // namespace gb