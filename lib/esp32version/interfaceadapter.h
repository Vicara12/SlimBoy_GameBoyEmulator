#pragma once

#include <Arduino.h>
#include "interface.h"


class ESP32Interface : public gb::HardwareInterface<ESP32Interface> {
public:

  inline void print (const std::string_view &s) {Serial.print(s.data());}

  inline void println (const std::string_view &s) {Serial.println(s.data());}

  inline std::string userLineInput () {return "";}

  inline void sleepMillis (uint t) {delay(t);}

  inline ulong realTimeMicros () {return micros();}

  inline void playAudio (const gb::AudioPacket &ap) {}

  inline void informEmuRate (float r) {Serial.println(r);};
};