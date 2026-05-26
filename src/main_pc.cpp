#include <SFML/Graphics.hpp>
#include <thread>
#include "interfaceadapter.h"
#include "program.h"
#include "emulator.h"
#include "tetris.h"


#define PX_WIDTH 4


int main (int argc, char **argv) {
  GameRom game_rom;
  if (argc > 1) {
    // Read game rom, if there was an error quit emulator
    if (not readRom(std::string(argv[1]), game_rom)) {
      return 1;
    }
  } else {
    std::copy(tetris_rom.begin(), tetris_rom.end(), game_rom.begin());
  }
  bool debug = ((argc > 1 and std::string(argv[1]) == "-d") or
                (argc > 2 and std::string(argv[2]) == "-d"));
  if (debug) {
    std::cout << std::endl << "(Executing in debug mode)" << std::endl;
  }
  EmulatorConfig config = {
    .debug = debug,
    .synch_execution = true,
    .skip_boot_room = false
  };
  sf::RenderWindow window = createWindow(PX_WIDTH);
  PCInterface interface;
  std::thread emulation_thread(emulator<PCInterface>, std::ref(interface), &game_rom, config);
  emulation_thread.detach();
  interfaceLoop(interface, emulation_thread, window);
}