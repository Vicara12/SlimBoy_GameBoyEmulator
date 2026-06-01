#pragma once

#include <functional>
#include <string>
#include <utility>
#include <mutex>
#include <vector>
#include "types.h"
#include "graphics/graphicstate.h"
#include "audio/audiostate.h"



// This template class contains the function declarations that get called whenever interaction with
// the underlying hardware is needed (for example for button input or timing).
// Each target platform needs to implement a class specifying the behavior following the
// Curiously Recurring Template Pattern (CRTP) for some of the emulator side functions.
// They should be as fast as possible (prefer inline) as they are executed sequentially with the
// emulator
template<class Derived>
class HardwareInterface {
  std::mutex screen_mutex;
  std::array<ScreenPixels*, 3> screen_frames;
  std::unique_ptr<std::vector<Byte>> ram_copy;
  float emu_rate = 1.f;
  Byte buttons = 0x00;
  bool end_emulation = false;
  bool emulation_ended = false;
  bool new_frame = false;

  inline Derived& impl() {return *static_cast<Derived*>(this);};

public:

  // EMULATOR SIDE FUNCTIONS

  // IMPLEMENT: This function takes a string and prints it (either on console, serial, etc)
  inline void print (const std::string_view &s) {impl().print(s);}

  // IMPLEMENT: Same as print, but adds end of line
  inline void println (const std::string_view &s) {impl().println(s);}

  // IMPLEMENT: This function should return a line introduced by the user
  inline std::string userLineInput () {return impl().userLineInput();}

  // IMPLEMENT: This function should pause the program for a given amount of milliseconds
  inline void sleepMillis (uint t) {impl().sleepMillis(t);}

  // IMPLEMENT: This function should return the current real time in microseconds
  inline ulong realTimeMicros () {return impl().realTimeMicros();}

  // IMPLEMENT: Called every 32 times a second, receives as argument the left and right audio buffers, respectively
  inline void playAudio (AudioPacket&& ap) {impl().playAudio(std::move(ap));}

  inline ScreenPixels* updateScreen () {
    std::lock_guard<std::mutex> lock(screen_mutex);
    new_frame = true;
    std::swap(screen_frames[1], screen_frames[2]);
    return screen_frames[2];
  }

  inline bool endEmulation () {return end_emulation;}

  inline void informEmulationEnded () {emulation_ended = true;}

  inline void informEmuRate (float r) {emu_rate = r;}

  inline Byte readButtons () {return buttons;}

  inline void saveRAM (std::unique_ptr<std::vector<Byte>> ram) {ram_copy = std::move(ram);}


  // PLATFORM SIDE FUNCTIONS

  // This function returns a ScreenFrame, which contains the value of each pixel in an intensity
  // scale from 0 to 3, where 0 is white and 3 black. NEVER call delete or free on the returned ptr.
  inline ScreenPixels* getLatestScreen () {
    std::lock_guard<std::mutex> lock(screen_mutex);
    // Swapping when there is no new frame available can lead to printing an old frame
    if (new_frame) {
      std::swap(screen_frames[0], screen_frames[1]);
    }
    new_frame = false;
    return screen_frames[0];
  }

  // This function receives a Byte in which a 1 corresponds to some button being pressed. The bit to
  // button mapping is defined in the Buttons enum
  void setButtons (Byte buttons_pressed) {buttons = buttons_pressed;}

  void requestEmulationEnd () {end_emulation = true;}

  bool emulationEnded () const {return emulation_ended;}

  // Updated every 0.5s; provides the ratio t_emulation/t_real
  float emuRate () const {return emu_rate;};

  std::unique_ptr<std::vector<Byte>> getRAM () const {return std::move(ram_copy);}

protected:

  HardwareInterface() {
    for (auto &sf : screen_frames) {
      sf = new ScreenPixels;
    }
  }

  ~HardwareInterface() {
    for (auto &sf : screen_frames) {
      delete sf;
      sf = nullptr;
    }
  }
};