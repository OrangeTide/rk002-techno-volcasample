/*
 * CC11: Set tempo from 100 to 140 BPM.
 */
#include <RK002.h>

#define NUM_PARTS 10
#define NUM_PATTERNS 5
#define PPQN 24
#define NOTE_OFF 0x90
#define NOTE_ON 0x80
#define PATTERN_END 0xF4

RK002_DECLARE_INFO("TECHNO_VSAMPLE","Wouter Hisschemöller","0.1","22aef8e0-383f-47ef-a85d-4c74141e59ce")

typedef struct event {
  byte type;
  byte data0;
  byte data1;
  unsigned short tick;
} event_t;

typedef struct part {
  event_t* pattern;
  byte eventIndex;
  unsigned int patternStartTick;
} part_t;

typedef struct pattern_data {
  byte pattern_length;
  event_t* pattern;
} pattern_data_t;

event_t pattern_kick0[] = {
  {NOTE_ON, 60, 100, PPQN * 0},
  {NOTE_OFF, 60, 0, PPQN * 0.25},
  {NOTE_ON, 60, 100, PPQN * 1},
  {NOTE_OFF, 60, 0, PPQN * 1.25},
  {NOTE_ON, 60, 100, PPQN * 2},
  {NOTE_OFF, 60, 0, PPQN * 2.25},
  {NOTE_ON, 60, 100, PPQN * 3},
  {NOTE_OFF, 60, 0, PPQN * 3.25},
  {PATTERN_END, 0, 0, PPQN * 4}
};

event_t pattern_kick1[] = {
  {NOTE_ON, 60, 100, PPQN * 0},
  {NOTE_OFF, 60, 0, PPQN * 0.25},
  {NOTE_ON, 60, 60, PPQN * 0.75},
  {NOTE_OFF, 60, 0, PPQN * 0.99},
  {NOTE_ON, 60, 100, PPQN * 1},
  {NOTE_OFF, 60, 0, PPQN * 1.25},
  {NOTE_ON, 60, 100, PPQN * 2},
  {NOTE_OFF, 60, 0, PPQN * 2.25},
  {NOTE_ON, 60, 100, PPQN * 3},
  {NOTE_OFF, 60, 0, PPQN * 3.25},
  {NOTE_ON, 60, 61, PPQN * 3.5},
  {NOTE_OFF, 60, 0, PPQN * 3.75},
  {PATTERN_END, 0, 0, PPQN * 4}
};

event_t pattern_kick2[] = {
  {NOTE_ON, 60, 99, PPQN * 0},
  {NOTE_OFF, 60, 0, PPQN * 0.25},
  {PATTERN_END, 0, 0, PPQN * 4}
};

event_t pattern3[3];
event_t pattern4[3];

pattern_data_t patterns[NUM_PATTERNS] = {
  { 9, pattern_kick0 },
  { 13, pattern_kick1 },
  { 3, pattern_kick2 },
  { 3, pattern3 },
  { 3, pattern4 }
};

part_t parts[NUM_PARTS] = {{ 0 }};
unsigned int tick = 0;
byte partIndex = 0;
byte patternIndex = 0;

bool RK002_onControlChange(byte channel, byte nr, byte value) {
  switch (nr) {
    case 0: {
        partIndex = (byte)(NUM_PARTS * (value / 127.0));
        RK002_printf("partIndex %d\n", partIndex);
      }
      break;
    
    case 1: {
        byte newIndex = (byte)(NUM_PATTERNS * (value / 127.0));
        RK002_printf("Pattern new index %d\n", newIndex);
        if (newIndex != patternIndex) {
          patternIndex = newIndex;
          setPattern();
        }
      }
      break;

    case 10: {
        RK002_clockSetTempo(1000 + (400 * (value / 127.0)));
        RK002_printf("RK002_clockSetTempo %d\n", RK002_clockGetTempo());
      }
      break;
  }
  return false;
}

bool RK002_onClock() {
  for (int i = 0; i < NUM_PARTS; i++) {
    unsigned int patternTick = tick - parts[i].patternStartTick;
    event_t e = *(parts[i].pattern + parts[i].eventIndex);
    if (e.tick == patternTick) {
      parts[i].eventIndex++;
      switch (e.type) {
        case NOTE_ON:
          RK002_sendNoteOn(i, e.data0, e.data1);
          break;
 
        case NOTE_OFF:
          RK002_sendNoteOff(i, e.data0, e.data1);
          break;

        case PATTERN_END:
          parts[i].eventIndex = 0;
          parts[i].patternStartTick = tick + 1;
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

  // create the random patterns
  for (int i = 3; i < NUM_PATTERNS; i++) {
    byte numEvents = patterns[i].pattern_length / 2;
    for (int j = 0; j < numEvents; j++) {
      addNoteEvent((patterns[i].pattern + j), 0, 6, 60, 98);
    }
    addPatternEndEvent((patterns[i].pattern + (patterns[i].pattern_length - 1)), PPQN * 4);
  }
  
  patternIndex = 3;
  setPattern();
}

void loop() {}

/**
 * Add note to pattern.
 */
void addNoteEvent(event_t* event_ptr, unsigned short tick, byte duration, byte pitch, byte velocity) {
  event_ptr->type = NOTE_ON;
  event_ptr->data0 = pitch;
  event_ptr->data1 = velocity;
  event_ptr->tick = tick;
  (event_ptr + 1)->type = NOTE_OFF;
  (event_ptr + 1)->data0 = pitch;
  (event_ptr + 1)->data1 = 0;
  (event_ptr + 1)->tick = tick + duration;
}

/**
 * Add end of pattern event to pattern.
 */
void addPatternEndEvent(event_t* event_ptr, unsigned short tick) {
  event_ptr->type = PATTERN_END;
  event_ptr->tick = tick;
}

/**
 * Set pattern of current part.
 */
void setPattern() {
  event_t* pattern = patterns[patternIndex].pattern;
  parts[partIndex].pattern = pattern;
  parts[partIndex].eventIndex = 0;
  parts[partIndex].patternStartTick = tick + 1;
}
