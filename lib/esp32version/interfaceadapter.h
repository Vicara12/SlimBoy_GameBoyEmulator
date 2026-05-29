#pragma once

#include <Arduino.h>
#include "interface.h"


class ESP32Interface : public HardwareInterface<ESP32Interface> {
public:

  inline void print (const std::string_view &s) {}

  inline void println (const std::string_view &s) {}

  inline std::string userLineInput () {return "";}

  inline void sleepMillis (uint t) {delay(t);}

  inline ulong realTimeMicros () {return micros();}

  inline void playAudio (AudioPacket&& ap) {}

  inline void informEmuRate (float r) {Serial.println(r);};
};