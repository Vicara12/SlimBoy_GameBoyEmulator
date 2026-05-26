#include <map>
#include <fstream>
#include "program.h"
#include "graphics/graphicstate.h"
#include "interfaceadapter.h"



sf::RenderWindow createWindow (int px_size) {
  const int windowWidth = px_size*SCREEN_PX_W;
  const int windowHeight = px_size*SCREEN_PX_H;
  return sf::RenderWindow(sf::VideoMode(windowWidth, windowHeight), "Game Boy");
}


bool readRom (const std::string &path, GameRom &game_rom) {
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


void drawScreen (sf::RenderWindow &window, PCInterface &interface) {
  ScreenPixels *px_intensity = interface.getLatestScreen();
  std::array<float,3> base_color = {255, 255, 255};
  int px_size = window.getSize().x/SCREEN_PX_W;

  window.clear();
  for (int i = 0; i < SCREEN_PX_H; i++) {
    for (int j = 0; j < SCREEN_PX_W; j++) {
      sf::RectangleShape square(sf::Vector2f(px_size, px_size));
      square.setFillColor(sf::Color((*px_intensity)[i].pixel[j]*base_color[0],
                                    (*px_intensity)[i].pixel[j]*base_color[1],
                                    (*px_intensity)[i].pixel[j]*base_color[2]));
      square.setPosition(j * px_size, i * px_size);
      window.draw(square);
    }
  }
  window.display();
}


bool handleInputs (sf::RenderWindow &window, PCInterface &interface) {
  sf::Event event;
  while (window.pollEvent(event)) {
    if (event.type == sf::Event::Closed) {
      return true;
    }
  }
  // GB buttons to kb keys mapping
  std::map<sf::Keyboard::Key,Button>key_binds = {
    // Dir pad
    {sf::Keyboard::W,    Button::Up},
    {sf::Keyboard::A,  Button::Left},
    {sf::Keyboard::S,  Button::Down},
    {sf::Keyboard::D, Button::Right},
    // Other buttons
    {sf::Keyboard::O,      Button::A},
    {sf::Keyboard::K,      Button::B},
    {sf::Keyboard::N, Button::Select},
    {sf::Keyboard::M,  Button::Start},
  };
  // Update GB buttons pressed
  Byte buttons_pressed = 0x00;
  for (const auto& bind : key_binds) {
    if (sf::Keyboard::isKeyPressed(bind.first)) {
      buttons_pressed |= static_cast<Byte>(bind.second);
    }
  }
  interface.setButtons(buttons_pressed);
  // User can also quit emulation by pressing Esc
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
    return true;
  }
  // User does not want to end simulation
  return false;
}


void interfaceLoop (
  PCInterface &interface,
  std::thread &emulation_thread,
  sf::RenderWindow &window
) {
  auto &audio_stream = interface.getAudioStream();
  audio_stream.play();

  // Main loop to render the window
  while (window.isOpen()) {
    bool exit = handleInputs(window, interface);
    // Audio is handled automatically by the audio stream in if_data
    drawScreen(window, interface);
    if (exit) {
      interface.requestEmulationEnd();
      if (emulation_thread.joinable()) {
        emulation_thread.join();
      }
      window.close();
      audio_stream.stop();
    }
  }
}