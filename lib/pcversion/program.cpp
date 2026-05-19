#include <map>
#include <fstream>
#include "program.h"
#include "graphics/graphicstate.h"
#include "interfaceadapter.h"



bool readRom (const std::string &path, GameRom &game_rom)
{
  // Open file at the end for size
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file.is_open()) {
      std::cerr << "Error: Unable to open file " << path << std::endl;
      return false;
  }
  std::streamsize fileSize = file.tellg();
  if (fileSize > 0x8000) {
    std::cerr << "Error: File is larger than " << 0x8000 << " bytes: " << fileSize << std::endl;
    return false;
  }
  file.seekg(0, std::ios::beg); // go back to the beginning of the file
  game_rom.fill(0x00);
  if (!file.read(reinterpret_cast<char*>(&game_rom), fileSize)) {
      std::cerr << "Error: Unable to read the file contents." << std::endl;
      return false;
  }
  file.close();
  return true;
}


void drawScreen (int px_size, sf::RenderWindow &window, InterfaceData *if_data)
{
  ScreenPixels px_intensity(SCREEN_PX_H, std::vector<float>(SCREEN_PX_W));
  getNewScreen(if_data, px_intensity);
  std::array<float,3> base_color = {255, 255, 255};

  window.clear();
  for (int i = 0; i < SCREEN_PX_H; i++) {
    for (int j = 0; j < SCREEN_PX_W; j++) {
      sf::RectangleShape square(sf::Vector2f(px_size, px_size));
      square.setFillColor(sf::Color(px_intensity[i][j]*base_color[0],
                                    px_intensity[i][j]*base_color[1],
                                    px_intensity[i][j]*base_color[2]));
      square.setPosition(j * px_size, i * px_size);
      window.draw(square);
    }
  }
  window.display();
}


bool handleInputs (sf::RenderWindow &window, InterfaceData *if_data)
{
  sf::Event event;
  while (window.pollEvent(event)) {
    if (event.type == sf::Event::Closed) {
      return true;
    }
  }
  // GB buttons to kb keys mapping
  std::map<sf::Keyboard::Key,Byte> key_binds = {
    // Dir pad
    {sf::Keyboard::W,    UP_PRESSED},
    {sf::Keyboard::A,  LEFT_PRESSED},
    {sf::Keyboard::S,  DOWN_PRESSED},
    {sf::Keyboard::D, RIGHT_PRESSED},
    // Other buttons
    {sf::Keyboard::O,      A_PRESSED},
    {sf::Keyboard::K,      B_PRESSED},
    {sf::Keyboard::N, SELECT_PRESSED},
    {sf::Keyboard::M,  START_PRESSED},
  };
  // Update GB buttons pressed
  Byte buttons_pressed = 0x00;
  for (const auto& bind : key_binds) {
    if (sf::Keyboard::isKeyPressed(bind.first)) {
      buttons_pressed |= bind.second;
    }
  }
  updateButtons(if_data, buttons_pressed);
  // User can also quit emulation by pressing Esc
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
    return true;
  }
  // User does not want to end simulation
  return false;
}


void interfaceLoop (InterfaceData *if_data, std::thread &emulation_thread)
{
  int px_size = 4;
  // Dimensions of the window and the grid
  const int windowWidth = px_size*SCREEN_PX_W;
  const int windowHeight = px_size*SCREEN_PX_H;

  // Create a window
  sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "Game Boy");

  if_data->audio_stream.play();

  // Main loop to render the window
  while (window.isOpen()) {
    bool exit = handleInputs(window, if_data);
    // Audio is handled automatically by the audio stream in if_data
    drawScreen(px_size, window, if_data);
    if (exit) {
      endEmulation(if_data);
      if (emulation_thread.joinable()) {
        emulation_thread.join();
      }
      window.close();
      if_data->audio_stream.stop();
    }
  }
}