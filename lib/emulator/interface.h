#pragma once

#include <functional>
#include <string>
#include "types.h"
#include "graphics/graphicstate.h"
#include "audio/audiostate.h"


// Button defines for readButtons function
#define RIGHT_PRESSED   0x01
#define LEFT_PRESSED    0x02
#define UP_PRESSED      0x04
#define DOWN_PRESSED    0x08
#define A_PRESSED       0x10
#define B_PRESSED       0x20
#define SELECT_PRESSED  0x40
#define START_PRESSED   0x80

using AudioBuffer = std::vector<uint8_t>;


// This structure contains the functions that get called whenever interaction with the underlying
// hardware is needed (for example for button input or timing). They should be as fast as possible,
// as they are executed sequentially with the emulator
struct Interface {
  // This function returns a Byte in which a 1 in bit 0 corresponds to R button pressed, in bit 1 to
  // L, in bit 2 to U, in bit 3 to D, in bit 4 to A, int bit 5 to B, in bit 6 to Sel and in bit 7 to
  // start
  std::function<Byte()> readButtons = nullptr;

  // This function takes a string and prints it (either on console, serial, etc)
  std::function<void(std::string)> print = nullptr;

  // This function should return a line introduced by the user
  std::function<std::string()> userLineInput = nullptr;

  // This function should pause the program for a given amount of milliseconds
  std::function<void(int)> sleepMillis = nullptr;

  // This function should return the current real time in microseconds
  std::function<ulong()> realTimeMicros = nullptr;

  // This function receives a ScreenFrame, which contains the value of each pixel in an intensity
  // scale from 0 to 3, where 0 is white and 3 black, and prints it to screen
  std::function<void(ScreenFrame*)> updateScreen = nullptr;

  // This function is called periodically to check if the emulation has to be ended
  std::function<bool()> endEmulation = nullptr;

  // This function is called every 0.5 seconds and provides t_emulation/t_real
  std::function<void(float)> informEmuRate = nullptr;

  // Called every 32 times a second, receives as argument the left and right audio buffers, respectively
  std::function<void(AudioPacket&&)> playAudio = nullptr;
};