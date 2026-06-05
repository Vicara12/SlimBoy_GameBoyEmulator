/*
* Functions for instruction printing and instruction-related debugging.
*/

#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include "types.h"
#include "instructions/instructiondefines.h"
#include "instructions/instruction.h"


namespace gb {

struct Touched {
  bool AF = false;
  bool BC = false;
  bool DE = false;
  bool HL = false;
  bool SP = false;
  bool flags = false;
  Short memory = 0x0000; // 0 = no memory touched
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



inline std::string formatInt (int n)
{
  if (n >= 0 and n < 10) {
    return "   " + std::to_string(n);
  }
  else if (n > -10 and n < 100) {
    return "  " +  std::to_string(n);
  }
  return " " + std::to_string(n);
}


inline std::string regName (Short addr)
{
  switch (addr)
  {
    case 0xFF00:
      return "    P1";
    case 0xFF04:
      return "   DIV";
    case 0xFF05:
      return "  TIMA";
    case 0xFF06:
      return "   TMA";
    case 0xFF07:
      return "   TMA";
    case 0xFF0F:
      return "    IF";
    case 0xFF40:
      return "  LCDC";
    case 0xFF41:
      return "  STAT";
    case 0xFF42:
      return "   SCY";
    case 0xFF43:
      return "   SCX";
    case 0xFF44:
      return "    LY";
    case 0xFF45:
      return "   LYC";
    case 0xFF46:
      return "   DMA";
    case 0xFF47:
      return "   BGP";
    case 0xFF48:
      return "  OBP0";
    case 0xFF49:
      return "  OBP1";
    case 0xFFFF:
      return "    IE";
    default:
      return formatShort(addr);
  }
}


inline std::string instrStr (Byte opcode, Byte data0, Byte data1, Touched &touched)
{
  const std::string reg_space = "    ";
  const std::string two_reg_space = reg_space + " " + reg_space;
  // Non CB prefix, opcode < 0x40 and >= 0xC0 or HALT
  switch (opcode)
  {
  // 0x00 - 0x3F opcode block
  case 0x00:
    return "NOP " + two_reg_space;
  case 0x01:
    touched.BC = true;
    return "LD    BC," + formatShort(JOIN_REGS(data1, data0));
  case 0x02:
    touched.AF = true;
    touched.BC = true;
    return "LD  (BC),   A";
  case 0x03:
    touched.BC = true;
    return "INC   BC     ";
  case 0x04:
    touched.BC = true;
    return "INC    B     ";
  case 0x05:
    touched.BC = true;
    return "DEC    B     ";
  case 0x06:
    touched.BC = true;
    return "LD     B,  " + formatByte(data0);
  case 0x07:
    return "RLCA         ";
  case 0x08:
    return "LD  " + formatShort(JOIN_REGS(data1, data0)) + ",  SP";
  case 0x09:
    return "ADD   HL,  BC";
  case 0x0A:
    return "LD     A,(BC)";
  case 0x0B:
    return "DEC   BC " + reg_space;
  case 0x0C:
    return "INC    C     ";
  case 0x0D:
    return "DEC    C     ";
  case 0x0E:
    return "LD     C,  " + formatByte(data0);
  case 0x0F:
    return "RRCA         ";
  case 0x10:
    return "STOP" + two_reg_space;
  case 0x11:
    return "LD    DE," + formatShort(JOIN_REGS(data1, data0));
  case 0x12:
    return "LD  (DE),   A";
  case 0x13:
    return "INC   DE     ";
  case 0x14:
    return "INC    D     ";
  case 0x15:
    return "DEC    D     ";
  case 0x16:
    return "LD     D,  " + formatByte(data0);
  case 0x17:
    return "RLA          ";
  case 0x18:
    return "JR  " + formatInt(int8_t(data0)) + " " + reg_space;
  case 0x19:
    return "ADD   HL,  DE";
  case 0x1A:
    return "LD     A,(DE)";
  case 0x1B:
    return "DEC   DE " + reg_space;
  case 0x1C:
    return "INC    E     ";
  case 0x1D:
    return "DEC    E     ";
  case 0x1E:
    return "LD     E,  " + formatByte(data0);
  case 0x1F:
    return "RRA          ";
  case 0x20:
    return "JR    NZ," + formatInt(int8_t(data0));
  case 0x21:
    return "LD    HL," + formatShort(JOIN_REGS(data1, data0));
  case 0x22:
    return "LD  (HL+),  A";
  case 0x23:
    return "INC   HL     ";
  case 0x24:
    return "INC    H     ";
  case 0x25:
    return "DEC    H     ";
  case 0x26:
    return "LD     H,  " + formatByte(data0);
  case 0x27:
    return "DAA " + two_reg_space;
  case 0x28:
    return "JR     Z," + formatInt(int8_t(data0));
  case 0x29:
    return "ADD   HL,  HL";
  case 0x2A:
    return "LD    A,(HL+)";
  case 0x2B:
    return "DEC   HL " + reg_space;
  case 0x2C:
    return "INC    L     ";
  case 0x2D:
    return "DEC    L     ";
  case 0x2E:
    return "LD     L,  " + formatByte(data0);
  case 0x2F:
    return "CPL " + two_reg_space;
  case 0x30:
    return "JR    NC," + formatInt(int8_t(data0));
  case 0x31:
    return "LD    SP," + formatShort(JOIN_REGS(data1, data0));
  case 0x32:
    return "LD  (HL-),  A";
  case 0x33:
    return "INC   SP     ";
  case 0x34:
    return "INC (HL)     ";
  case 0x35:
    return "DEC (HL)     ";
  case 0x36:
    return "LD  (HL),  " + formatByte(data0);
  case 0x37:
    return "SCF " + two_reg_space;
  case 0x38:
    return "JR     C," + formatInt(int8_t(data0));
  case 0x39:
    return "ADD   HL,  SP";
  case 0x3A:
    return "LD    A,(HL-)";
  case 0x3B:
    return "DEC   SP " + reg_space;
  case 0x3C:
    return "INC    A     ";
  case 0x3D:
    return "DEC    A     ";
  case 0x3E:
    return "LD     A,  " + formatByte(data0);
  case 0x3F:
    return "CCF " + two_reg_space;
  // Special case: HALT
  case 0x76:
    return "HALT" + two_reg_space;
  // 0xC0 - 0xFF opcode block
  case 0xC0:
    return "RET   NZ " + reg_space;
  case 0xC1:
    return "POP   BC " + reg_space;
  case 0xC2:
    return "JP    NZ," + formatShort(JOIN_REGS(data1, data0));
  case 0xC3:
    return "JP    " + formatShort(JOIN_REGS(data1, data0)) + "   ";
  case 0xC4:
    return "CALL  NZ," + formatShort(JOIN_REGS(data1, data0));
  case 0xC5:
    return "PUSH  BC " + reg_space;
  case 0xC6:
    return "ADD    A,  " + formatByte(data0);
  case 0xC7:
    return "RST 0x00 " + reg_space;
  case 0xC8:
    return "RET    Z " + reg_space;
  case 0xC9:
    return "RET      " + reg_space;
  case 0xCA:
    return "JP     Z," + formatShort(JOIN_REGS(data1, data0));
  case 0xCC:
    return "CALL   Z," + formatShort(JOIN_REGS(data1, data0));
  case 0xCD:
    return "CALL  " + formatShort(JOIN_REGS(data1, data0)) + "   ";
  case 0xCE:
    return "ADC    A,  " + formatByte(data0);
  case 0xCF:
    return "RST 0x08 " + reg_space;
  case 0xD0:
    return "RET   NC " + reg_space;
  case 0xD1:
    return "POP   DE " + reg_space;
  case 0xD2:
    return "JP    NC," + formatShort(JOIN_REGS(data1, data0));
  case 0xD3:
    return "ILLEGAL  " + reg_space;
  case 0xD4:
    return "CALL NC," + formatShort(JOIN_REGS(data1, data0));
  case 0xD5:
    return "PUSH  DE " + reg_space;
  case 0xD6:
    return "SUB   " + formatByte(data0) + "," + reg_space;
  case 0xD7:
    return "RST 0x10 " + reg_space;
  case 0xD8:
    return "RET    C " + reg_space;
  case 0xD9:
    return "RETI     " + reg_space;
  case 0xDA:
    return "JP     C," + formatShort(JOIN_REGS(data1, data0));
  case 0xDB:
    return "ILLEGAL  " + reg_space;
  case 0xDC:
    return "CALL   C," + formatShort(JOIN_REGS(data1, data0));
  case 0xDD:
    return "ILLEGAL  " + reg_space;
  case 0xDE:
    return "SBC    A,  " + formatByte(data0);
  case 0xDF:
    return "RST 0x18 " + reg_space;
  case 0xE0:
    return "LDH (FF00+" + formatByte(data0) + "), A";
  case 0xE1:
    return "POP   HL " + reg_space;
  case 0xE2:
    return "LD (0xFF00+C),A";
  case 0xE3:
    return "ILLEGAL  " + reg_space;
  case 0xE4:
    return "ILLEGAL  " + reg_space;
  case 0xE5:
    return "PUSH  HL " + reg_space;
  case 0xE6:
    return "AND   " + formatByte(data0) + "," + reg_space;
  case 0xE7:
    return "RST 0x20 " + reg_space;
  case 0xE8:
    return "ADD   SP," + formatInt(int8_t(data0));
  case 0xE9:
    return "JP  (HL) " + reg_space;
  case 0xEA:
    return "LD (" + formatShort(JOIN_REGS(data1, data0)) + "),  A";
  case 0xEB:
    return "ILLEGAL  " + reg_space;
  case 0xEC:
    return "ILLEGAL  " + reg_space;
  case 0xED:
    return "ILLEGAL  " + reg_space;
  case 0xEE:
    return "XOR   " + formatByte(data0) + " " + reg_space;
  case 0xEF:
    return "RST 0x28 " + reg_space;
  case 0xF0:
    return "LDH  A,(FF00+" + formatByte(data0) + ")";
  case 0xF1:
    return "POP   AF " + reg_space;
  case 0xF2:
    return "LD A,(0xFF00+C)";
  case 0xF3:
    return "DI  " + two_reg_space;
  case 0xF4:
    return "ILLEGAL  " + reg_space;
  case 0xF5:
    return "PUSH  AF " + reg_space;
  case 0xF6:
    return "OR    " + formatByte(data0) + "," + reg_space;
  case 0xF7:
    return "RST 0x30 " + reg_space;
  case 0xF8:
    return "LD HL,SP+" + formatInt(int8_t(data0));
  case 0xF9:
    return "LD    SP,  HL";
  case 0xFA:
    return "LD   A,(" + formatShort(JOIN_REGS(data1, data0)) + ")";
  case 0xFB:
    return "EI  " + two_reg_space;
  case 0xFC:
    return "ILLEGAL  " + reg_space;
  case 0xFD:
    return "ILLEGAL  " + reg_space;
  case 0xFE:
    return "CP    " + formatByte(data0) + " " + reg_space;
  case 0xFF:
    return "RST 0x38 " + reg_space;
  default:
    break;
  }
  // CB instrs or opcode >= 0x40 and < 0xC0 excluding HALT
  std::string opname;
  Byte opcode_msb = ((opcode == 0xCB ? data0 : opcode) >> 4);
  Byte opcode_lsb = ((opcode == 0xCB ? data0 : opcode) & 0x0F);
  if (opcode != 0xCB) {
    if (opcode_msb >= 0x4 and opcode_msb < 0x8) {
      opname = "LD  ";
    } else {
      switch (opcode_msb)
      {
      case 0x8:
        opname = (opcode_lsb < 0x8 ? "ADD " : "ADC ");
        break;
      case 0x9:
        opname = (opcode_lsb < 0x8 ? "SUB " : "SBC ");
        break;
      case 0xA:
        opname = (opcode_lsb < 0x8 ? "AND " : "XOR ");
        break;
      case 0xB:
        opname = (opcode_lsb < 0x8 ? "OR  " : "CP  ");
        break;
      }
    }
  } else {
    // Set touched flags and regs
    if (data0 < 0x80) {
      touched.flags = true;
    }
    switch (opcode_msb)
    {
    case 0x0:
      opname = (opcode_lsb < 0x8 ? "RLC " : "RRC ");
      break;
    case 0x1:
      opname = (opcode_lsb < 0x8 ? "RL  " : "RR  ");
      break;
    case 0x2:
      opname = (opcode_lsb < 0x8 ? "SLA " : "SRA ");
      break;
    case 0x3:
      opname = (opcode_lsb < 0x8 ? "SWAP" : "SRL ");
      break;
    case 0x4:
      opname = (opcode_lsb < 0x8 ? "BIT0" : "BIT1");
      break;
    case 0x5:
      opname = (opcode_lsb < 0x8 ? "BIT2" : "BIT3");
      break;
    case 0x6:
      opname = (opcode_lsb < 0x8 ? "BIT4" : "BIT5");
      break;
    case 0x7:
      opname = (opcode_lsb < 0x8 ? "BIT6" : "BIT7");
      break;
    case 0x8:
      opname = (opcode_lsb < 0x8 ? "RES0" : "RES1");
      break;
    case 0x9:
      opname = (opcode_lsb < 0x8 ? "RES2" : "RES3");
      break;
    case 0xA:
      opname = (opcode_lsb < 0x8 ? "RES4" : "RES5");
      break;
    case 0xB:
      opname = (opcode_lsb < 0x8 ? "RES6" : "RES7");
      break;
    case 0xC:
      opname = (opcode_lsb < 0x8 ? "SET0" : "SET1");
      break;
    case 0xD:
      opname = (opcode_lsb < 0x8 ? "SET2" : "SET3");
      break;
    case 0xE:
      opname = (opcode_lsb < 0x8 ? "SET4" : "SET5");
      break;
    case 0xF:
      opname = (opcode_lsb < 0x8 ? "SET6" : "SET7");
      break;
    }
  }
  std::string secondary_reg;
  if (opcode_msb >= 0x8) {
    secondary_reg = "   A";
  } else {
    switch (opcode_msb)
    {
    case 0x4:
      secondary_reg = (opcode_lsb < 0x8 ? "   B" : "   C");
      break;
    case 0x5:
      secondary_reg = (opcode_lsb < 0x8 ? "   D" : "   E");
      break;
    case 0x6:
      secondary_reg = (opcode_lsb < 0x8 ? "   H" : "   L");
      break;
    case 0x7:
      secondary_reg = (opcode_lsb < 0x8 ? "(HL)" : "   A");
      break;
    }
  }
  std::string main_reg;
  switch (opcode_lsb)
  {
  case 0x0:
    main_reg = "   B";
    break;
  case 0x1:
    main_reg = "   C";
    break;
  case 0x2:
    main_reg = "   D";
    break;
  case 0x3:
    main_reg = "   E";
    break;
  case 0x4:
    main_reg = "   H";
    break;
  case 0x5:
    main_reg = "   L";
    break;
  case 0x6:
    main_reg = "(HL)";
    break;
  case 0x7:
    main_reg = "   A";
    break;
  case 0x8:
    main_reg = "   B";
    break;
  case 0x9:
    main_reg = "   C";
    break;
  case 0xA:
    main_reg = "   D";
    break;
  case 0xB:
    main_reg = "   E";
    break;
  case 0xC:
    main_reg = "   H";
    break;
  case 0xD:
    main_reg = "   L";
    break;
  case 0xE:
    main_reg = "(HL)";
    break;
  case 0xF:
    main_reg = "   A";
    break;
  }

  // Single reg ops
  if (opcode == 0xCB or opcode_msb >= 0xA or (opcode_msb == 0x9 and opcode_lsb < 0x8)) {
    return opname + main_reg + " " + reg_space;
  }
  // Two reg ops
  else {
    return opname + secondary_reg + "," + main_reg;
  }
}

inline std::string cycleStr (Byte opcode, Byte data0, Byte data1, State &state)
{
  // Prepare instruction preamble (PC and instr bytes)
  int instr_len = instrLen(opcode);
  std::string preamble = formatShort(state.PC) + " [" + formatByte(opcode);
  switch (instr_len)
  {
  case 1:
    preamble += "]      ";
    break;
  case 2:
    preamble += "," + formatByte(data0) + "]   ";
    break;
  case 3:
    preamble += "," + formatByte(data0) + "," + formatByte(data1) + "]";
  default:
    break;
  }
  
  // Prepare instruction str
  Touched touched;
  std::string instr = instrStr(opcode, data0, data1, touched);

  // Prepare instr appendix (affected state)
  std::stringstream state_ss;
  state_ss << " | " <<  "AF:" << formatShort(REG_AF(state))
                  << " BC:" << formatShort(REG_BC(state))
                  << " DE:" << formatShort(REG_DE(state))
                  << " HL:" << formatShort(REG_HL(state))
                  << " SP:" << formatShort(state.SP)
                  << " IME" << (state.ime ? "1" : "0");

  return preamble + " " + instr + " " + state_ss.str();
}

} // namespace gb