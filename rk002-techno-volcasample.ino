#include <RK002.h>

RK002_DECLARE_INFO("TECHNO_VSAMPLE","Wouter Hisschemöller","0.1","22aef8e0-383f-47ef-a85d-4c74141e59ce")

int beatCount = 0;

bool RK002_onClock() {
  switch (beatCount) {
    case 0:
      RK002_sendNoteOn(0, 60, 100);
      break;
   
    case 8:
      RK002_sendNoteOff(0, 60, 0);
      break;
  }
  beatCount = (beatCount + 1) % 24;
  return false;
}

void setup() {
  RK002_clockSetTempo(1200);
  RK002_clockSetMode(1);
}

void loop() {
}
