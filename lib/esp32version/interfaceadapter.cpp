#include <Arduino.h>
#include "interfaceadapter.h"



Interface getESP32Interface (float &emu_rate) {
  return Interface{
    .readButtons = [] () -> Byte {return 0x00;},
    .print = [] (std::string s) {},
    .userLineInput = [] () -> std::string {return "";},
    .sleepMillis = [] (int millis) {sleep(millis);},
    .realTimeMicros = [] () -> ulong {return micros();},
    .updateScreen = [] (ScreenFrame *) {},
    .endEmulation = [] () -> bool {return false;},
    .informEmuRate = [&] (float rate) {Serial.println(rate);},
    .playAudio = [] (AudioPacket &&) {}
  };
}