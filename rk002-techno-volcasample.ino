/*
 * CC11: Set tempo from 100 to 140 BPM.
 */
#include <RK002.h>

#define NUM_PARTS 10
#define PPQN 24
#define NOTE_OFF 0x90
#define NOTE_ON 0x80
#define PATTERN_END 0xF4

RK002_DECLARE_INFO("TECHNO_VSAMPLE","Wouter Hisschemöller","0.1","22aef8e0-383f-47ef-a85d-4c74141e59ce")

struct event {
  byte type;
  byte channel;
  byte data0;
  byte data1;
  unsigned short tick;
};

struct event pattern_kick0[] = {
  {NOTE_ON, 0, 60, 100, PPQN * 0},
  {NOTE_OFF, 0, 60, 0, PPQN * 0.25},
  {NOTE_ON, 0, 60, 100, PPQN * 1},
  {NOTE_OFF, 0, 60, 0, PPQN * 1.25},
  {NOTE_ON, 0, 60, 100, PPQN * 2},
  {NOTE_OFF, 0, 60, 0, PPQN * 2.25},
  {NOTE_ON, 0, 60, 100, PPQN * 3},
  {NOTE_OFF, 0, 60, 0, PPQN * 3.25},
  {PATTERN_END, 0, 0, 0, PPQN * 4}
};

struct event pattern_kick1[] = {
  {NOTE_ON, 0, 60, 100, PPQN * 0},
  {NOTE_OFF, 0, 60, 0, PPQN * 0.25},
  {NOTE_ON, 0, 60, 60, PPQN * 0.75},
  {NOTE_OFF, 0, 60, 0, PPQN * 0.99},
  {NOTE_ON, 0, 60, 100, PPQN * 1},
  {NOTE_OFF, 0, 60, 0, PPQN * 1.25},
  {NOTE_ON, 0, 60, 100, PPQN * 2},
  {NOTE_OFF, 0, 60, 0, PPQN * 2.25},
  {NOTE_ON, 0, 60, 100, PPQN * 3},
  {NOTE_OFF, 0, 60, 0, PPQN * 3.25},
  {NOTE_ON, 0, 60, 60, PPQN * 3.5},
  {NOTE_OFF, 0, 60, 0, PPQN * 3.75},
  {PATTERN_END, 0, 0, 0, PPQN * 4}
};

struct part {
  struct event* pattern;
  byte patternIndex;
  unsigned int patternDuration;
};

struct part parts[NUM_PARTS] = {{ 0 }};
unsigned int tick = 0;

bool RK002_onControlChange(byte channel, byte nr, byte value) {
  switch (nr) {
    case 11:
      RK002_clockSetTempo(1000 + (400 * (value / 127.0)));
      break;
  }
  return false;
}

bool RK002_onClock() {
  for (int i = 0; i < NUM_PARTS; i++) {
    unsigned int patternTick = tick % parts[i].patternDuration;
    
    // reset pattern after duration
    if (patternTick == 0) {
      parts[i].patternIndex = 0;
    }
    
    // handle pattern events that should happen on this tick
    struct event e = parts[i].pattern[parts[i].patternIndex];
    if (e.tick == patternTick) {
      parts[i].patternIndex++;
      switch (e.type) {
        case NOTE_ON:
          RK002_sendNoteOn(i, e.data0, e.data1);
          break;
 
        case NOTE_OFF:
          RK002_sendNoteOff(i, e.data0, e.data1);
          break;

        default:
          break;
      }
    }
  }

  tick++;
  return false;
}

void setup() {
  RK002_clockSetTempo(1200);
  RK002_clockSetMode(1);
  int patternLength = sizeof(pattern_kick0) / sizeof(struct event);
  parts[0].pattern = pattern_kick0;
  parts[0].patternDuration = pattern_kick0[patternLength - 1].tick;
}

void loop() {}
