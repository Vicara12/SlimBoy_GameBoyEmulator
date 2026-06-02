
// #define PROFILE // TODO remove

#include <Arduino.h>
#include <vector>
#include "SPIFFS.h"
#include "interfaceadapter.h"
#include "emulator.h"

float er;
ESP32Interface *interface;
GameRom game_rom;
auto emu_cfg = EmulatorConfig{
  .synch_execution = false,
  .skip_boot_room = false
};
const std::string game_path_ = "/Tetris.gb";
// const std::string game_path_ = "/Dr_Mario.gb";
// const std::string game_path_ = "/Zelda.gb";


// void launchEmulator () {
//   emulator(&interface, &game_rom, emu_cfg);
// }


void readGameRom(const std::string &game_path) {
  if (not SPIFFS.begin(true)) {
    Serial.println("Some error occurred while mounting SPIFFS");
    while (true) delay(1000);
  }
  
  File file = SPIFFS.open(game_path.c_str(), FILE_READ);
  if (not file || file.isDirectory())
    throw std::runtime_error("Could not open file: " + game_path);

  size_t file_size = file.size();
  game_rom.resize(file_size);
  file.read(game_rom.data(), file_size);
  file.close();
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nBeginning setup...");
  interface = new ESP32Interface;
  Serial.println("Interface loaded");
  readGameRom(game_path_);
  Serial.println("Done reading game ROM");
  
  emulator<ESP32Interface, false>(*interface, game_rom, emu_cfg);
}

void loop() {
  Serial.println("WTF");
  delay(1000);
}