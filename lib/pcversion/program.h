/*
* Functions for executing the PC version of the emulator
*/

#pragma once

#include <SFML/Graphics.hpp>
#include <thread>
#include "interfaceadapter.h"
#include "types.h"


bool readRom (const std::string &path, gb::GameRom &game_rom);

void interfaceLoop (PCInterface &if_data, std::thread &emulation_thread, sf::RenderWindow &window);

// Returns wether the user wants to terminate the program
bool handleInputs (sf::RenderWindow &window, PCInterface &interface);

void drawScreen (sf::RenderWindow &window, PCInterface &interface);

sf::RenderWindow createWindow (int px_size);