#include <Tasker.h>
#include "chords.h"
#include <stdlib.h>

Tasker tasker(true);

const int SPEAKER_PIN = 8;
const int BUTTON_PIN = 2;
const int LED_STEP_PINS[] = {5, 6, 4, 3};
const int INCREMENT_STEP_PIN = 9;
const int INCREMENT_CHORD_PIN = 7;

const int TEMPO_POT_PIN = A1;
const int DIVISIONS_POT_PIN = A2;
const int NOTE_DURATION_POT_PIN = A7;

const int TOTAL_CHORDS = 4;

bool isPlaying = false;

int onOffButtonState;
int prevOnOffButtonState;

int incrementStepButtonState;
int prevIncrementStepButtonState;

int incrementChordButtonState;
int prevIncrementChordButtonState;

int prevBpmValue = 100;
float beatDuration;
int beatDivisions = 6;

int currentChord = 0;
int currentStep = 0;

int activeStep = 0;
float noteDurationPercent = 0.5;

// Starting chord sequence
int chords[] = {0, 4, 5, 3};

int blinkVal = LOW;

// ------------------------------------------------------------------

void setBpm(int bpm)
{
  // duration of a beat in ms
  beatDuration = 60.0 / bpm * 1000;
}

void stopSequence()
{
  noTone(SPEAKER_PIN);

  // reset position
  currentStep = 0;
  currentChord = 0;

  // Turn off LEDs
  for (int i = 0; i < TOTAL_CHORDS; i++)
  {
    digitalWrite(LED_STEP_PINS[i], LOW);
  }
}

void playNoteStep()
{
  if (!isPlaying)
  {
    noTone(SPEAKER_PIN);
    return;
  };

  // Turn on LED for current chord step
  for (int i = 0; i < TOTAL_CHORDS; i++)
  {
    if (i == currentChord)
    {
      digitalWrite(LED_STEP_PINS[i], HIGH);
    }
    else
    {
      digitalWrite(LED_STEP_PINS[i], LOW);
    }
  }

  // cycle through notes in the chord
  int currentNoteIndex = currentStep % 3;
  int note = C_MAJ_CHORDS[chords[currentChord]][currentNoteIndex];
  float noteDuration = beatDuration / beatDivisions;
  tone(SPEAKER_PIN, note, noteDuration * noteDurationPercent);

  currentStep++;
  currentStep = currentStep % beatDivisions;

  // increment chord when steps cycle
  if (currentStep == 0)
  {
    currentChord++;
    currentChord = currentChord % TOTAL_CHORDS;
  }
}

// ------------------------------------------------------------------

void blinkActiveStep()
{
  for (int i = 0; i < TOTAL_CHORDS; i++)
  {
    if (i == activeStep)
    {
      digitalWrite(LED_STEP_PINS[i], blinkVal);
      blinkVal = !blinkVal;
    }
  }
}

void resetInterval()
{
  float noteDuration = beatDuration / beatDivisions;
  tasker.setInterval(playNoteStep, noteDuration);
}

void watchForInput()
{
  // ========================== POTS ==========================
  // Changing note duration
  int noteDurationSensorVal = (analogRead(NOTE_DURATION_POT_PIN) * 10) / 10;
  float newNoteDuration = (map(noteDurationSensorVal, 464, 1024, 10, 100)) / 100.0;
  noteDurationPercent = newNoteDuration;

  // changing divisions per step
  int divisionsSensorValue = (analogRead(DIVISIONS_POT_PIN) * 10) / 10;
  int newDivisionsValue = (map(divisionsSensorValue, 464, 1024, 1, 9));
  if (newDivisionsValue != beatDivisions)
  {
    beatDivisions = newDivisionsValue;
    resetInterval();
  }

  // Changing tempo
  int tempoSensorValue = (analogRead(TEMPO_POT_PIN) * 10) / 10;
  int newBpm = (map(tempoSensorValue, 464, 1024, 50, 130));
  if (abs(newBpm - prevBpmValue) > 5)
  {
    setBpm(newBpm);
    prevBpmValue = newBpm;
    resetInterval();
  }

  // ========================== BUTTONS ==========================
  // start/stop
  onOffButtonState = digitalRead(BUTTON_PIN);
  if (onOffButtonState != prevOnOffButtonState)
  {
    if (onOffButtonState == HIGH)
    {
      Serial.println("Toggle on/off");
      isPlaying = !isPlaying;
      if (!isPlaying)
      {
        stopSequence();
      }
    }
  }
  prevOnOffButtonState = onOffButtonState;

  // Increment selected step
  incrementStepButtonState = digitalRead(INCREMENT_STEP_PIN);
  if (incrementStepButtonState != prevIncrementStepButtonState)
  {
    if (incrementStepButtonState == HIGH)
    {
      Serial.println("Select Next Step");
      // Clear current blink
      for (int i = 0; i < TOTAL_CHORDS; i++)
      {
        digitalWrite(LED_STEP_PINS[activeStep], LOW);
      }
      activeStep++;
      activeStep = activeStep % TOTAL_CHORDS;
    }
  }
  prevIncrementStepButtonState = incrementStepButtonState;

  // Increment chord
  incrementChordButtonState = digitalRead(INCREMENT_CHORD_PIN);
  if (incrementChordButtonState != prevIncrementChordButtonState)
  {
    if (incrementChordButtonState == HIGH)
    {
      Serial.println("Increment Chord");
      int nextChordIndex = chords[activeStep];
      nextChordIndex++;
      nextChordIndex = nextChordIndex % 7;
      chords[activeStep] = nextChordIndex;
    }
  }
  prevIncrementChordButtonState = incrementChordButtonState;
}

// ------------------------------------------------------------------

void setup()
{
  Serial.begin(9600);

  setBpm(prevBpmValue);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(INCREMENT_STEP_PIN, INPUT);
  pinMode(INCREMENT_CHORD_PIN, INPUT);
  pinMode(NOTE_DURATION_POT_PIN, INPUT);

  pinMode(SPEAKER_PIN, OUTPUT);

  // initialize button states
  prevOnOffButtonState = digitalRead(BUTTON_PIN);
  prevIncrementStepButtonState = digitalRead(INCREMENT_STEP_PIN);
  prevIncrementChordButtonState = digitalRead(INCREMENT_CHORD_PIN);

  // Set intervals
  tasker.setInterval(blinkActiveStep, 100);
  tasker.setInterval(watchForInput, 1);

  float noteDuration = beatDuration / beatDivisions;
  tasker.setInterval(playNoteStep, noteDuration, 0);
}

void loop()
{
  tasker.loop();
}
