/*
 * CC11: Set tempo from 100 to 140 BPM.
 */
#include <RK002.h>

#define NUM_PARTS 10
#define PPQN 24
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
  {NOTE_ON, 0, 60, 100, 0},
  {NOTE_ON, 0, 60, 100, PPQN},
  {NOTE_ON, 0, 60, 100, PPQN * 2},
  {NOTE_ON, 0, 60, 100, PPQN * 3},
  {PATTERN_END, 0, 0, 0, (PPQN * 4) - 1}
};

struct part {
  byte pitch;
  unsigned int endTick;
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
    
    // search for notes that should end on this tick
    if (parts[i].endTick == tick) {
      stopNote(i, parts[i].pitch);
    }

    // search for pattern events that should happen on this tick
    struct event patternEvent = parts[i].pattern[parts[i].patternIndex];
    if (patternEvent.tick == tick % parts[i].patternDuration) {
      switch (patternEvent.type) {
        case NOTE_ON:
          startNote(i, patternEvent.data0, patternEvent.data1);
          parts[i].patternIndex++;
          break;

        case PATTERN_END:
          parts[i].patternIndex = 0;
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
  parts[0].patternDuration = pattern_kick0[patternLength - 1].tick + 1;
}

void loop() {}

void startNote(byte channel, byte pitch, byte velocity) {
  // if a note is still playing then first stop it
  if (parts[channel].endTick > 0) {
    stopNote(0, pitch);
  }
  RK002_sendNoteOn(channel, pitch, velocity);
  parts[channel].pitch = pitch;
  parts[channel].endTick = tick + 8;
}

void stopNote(byte channel, byte pitch) {
  RK002_sendNoteOff(channel, pitch, 0);
  parts[channel].endTick = 0;
}
