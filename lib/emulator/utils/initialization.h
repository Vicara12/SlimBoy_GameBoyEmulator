/*
* Functions related to initializing state and memory
*/

#pragma once

#include "state.h"


// Load boot rom (0x0100 first bytes)
void loadBootRom (State *state);

// Load game except for 0x0100 first bytes
void loadGame (State *state, GameRom *game_rom);

// Load first 0x0100 bytes
void replaceBootRom (State *state, GameRom *game_rom);

void setPostBootState (State *state);