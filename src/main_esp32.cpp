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

  if (file.size() != game_rom.size()) {
    Serial.println("ROM and buffer size mismatch");
    while (true) delay(1000);
  }

  size_t counter = 0;
  while (file.available())
    game_rom[counter++] = file.read();

  file.close();

  if (counter != game_rom.size()) {
    Serial.println("Could not read full content of ROM");
    while (true) delay(1000);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nBeginning setup...");
  interface = new ESP32Interface;
  Serial.println("Interface loaded");
  readGameRom(game_path_);
  Serial.println("Done reading game ROM");
  
  emulator<ESP32Interface, false>(*interface, &game_rom, emu_cfg);
}

void loop() {
  Serial.println("WTF");
  delay(1000);
}