
#include <math.h>
#include <avr/interrupt.h>

// ========================================
// H SIN SHADOW LAMP
// SINGLE / DOUBLE / TRIPLE CLICK
// LONG PRESS / RESET
// ========================================

// RGB common-anode module
const byte RledPin = 3;
const byte GledPin = 5;
const byte BledPin = 4;

// Button: D2 to GND, INPUT_PULLUP
const byte buttonPin = 2;

// Onboard Nano L LED
const byte onboardLedPin = LED_BUILTIN;

// ========================================
// D4 SOFTWARE PWM
// ========================================

volatile byte blueOffDuty = 255;

ISR(TIMER1_OVF_vect) {
  byte duty = blueOffDuty;
  OCR1A = duty;

  if (duty == 0) {
    PORTD &= ~_BV(PD4);
  } else {
    PORTD |= _BV(PD4);
  }
}

ISR(TIMER1_COMPA_vect) {
  if (blueOffDuty > 0 && blueOffDuty < 255) {
    PORTD &= ~_BV(PD4);
  }
}

void setupBluePWM() {
  pinMode(BledPin, OUTPUT);
  digitalWrite(BledPin, HIGH);

  noInterrupts();

  TCCR1A = _BV(WGM10);
  TCCR1B = _BV(WGM12) | _BV(CS11) | _BV(CS10);

  TCNT1 = 0;
  OCR1A = 255;

  TIMSK1 = _BV(TOIE1) | _BV(OCIE1A);

  interrupts();
}

// ========================================
// BRIGHT COLOR PALETTES
// ========================================

// Deep RGB -> Glow RGB
const byte palettes[4][2][3] = {

  // 0: Astral Moonlight
  {
    {20, 75, 190},
    {35, 235, 255}
  },

  // 1: Phantom Orchid
  {
    {125, 10, 205},
    {255, 105, 255}
  },

  // 2: Celestial Jade
  {
    {15, 145, 110},
    {65, 255, 215}
  },

  // 3: Vermilion Foxfire
  {
    {205, 15, 65},
    {255, 190, 35}
  }
};

// ========================================
// MODE SETTINGS
// ========================================

enum AnimationStyle {
  BREATHING = 0,
  STEADY = 1,
  HEARTBEAT = 2
};

byte colorMode = 0;
byte animationStyle = BREATHING;

bool lampOn = true;
bool autoCycle = false;

// Breathing: one cycle every 4 seconds
const float breathingSeconds = 4.0f;

// Heartbeat: one pattern every 2.2 seconds
const float heartbeatSeconds = 2.2f;

// Automatic palette transitions
const unsigned long autoTransitionMs = 3500UL;

unsigned long autoStartTime = 0;
byte autoStartPalette = 0;

// ========================================
// BUTTON SETTINGS
// ========================================

const unsigned long debounceMs = 35;
const unsigned long multiClickMs = 380;
const unsigned long longPressMs = 1000;
const unsigned long resetPressMs = 3000;

bool rawButton = HIGH;
bool stableButton = HIGH;

unsigned long debounceStart = 0;
unsigned long pressStart = 0;
unsigned long lastRelease = 0;

byte clickCount = 0;

// 0 = normal press
// 1 = long press handled
// 2 = reset handled
byte holdState = 0;

// ========================================
// ANIMATION SETTINGS
// ========================================

float currentRGB[3] = {20, 75, 190};

float animationPhase = 0.0f;

// Approximately 60 updates per second
const unsigned long frameInterval = 16667UL;
unsigned long lastFrame = 0;

// Smooth color transition response
const float transitionSmoothness = 0.22f;

// ========================================
// RGB OUTPUT
// ========================================

void setColor(int r, int g, int b) {

  // Common-anode inversion
  analogWrite(
    RledPin,
    255 - constrain(r, 0, 255)
  );

  analogWrite(
    GledPin,
    255 - constrain(g, 0, 255)
  );

  blueOffDuty =
    255 - constrain(b, 0, 255);
}

// ========================================
// SERIAL FEEDBACK
// ========================================

void printPalette() {
  Serial.print(F("Palette: "));

  switch (colorMode) {
    case 0:
      Serial.println(F("Astral Moonlight"));
      break;
    case 1:
      Serial.println(F("Phantom Orchid"));
      break;
    case 2:
      Serial.println(F("Celestial Jade"));
      break;
    case 3:
      Serial.println(F("Vermilion Foxfire"));
      break;
  }
}

void printAnimation() {
  Serial.print(F("Animation: "));

  switch (animationStyle) {
    case BREATHING:
      Serial.println(F("Breathing"));
      break;
    case STEADY:
      Serial.println(F("Steady"));
      break;
    case HEARTBEAT:
      Serial.println(F("Heartbeat"));
      break;
  }
}

// ========================================
// BUTTON ACTIONS
// ========================================

void nextPalette() {

  // Single click exits auto mode
  if (autoCycle) {
    autoCycle = false;
  }

  colorMode = (colorMode + 1) % 4;

  printPalette();
}

void togglePower() {

  lampOn = !lampOn;

  // Onboard L LED follows power state
  digitalWrite(
    onboardLedPin,
    lampOn ? HIGH : LOW
  );

  Serial.println(
    lampOn ? F("Lamp ON") : F("Lamp OFF")
  );
}

void nextAnimation() {

  animationStyle = (animationStyle + 1) % 3;

  // Restart animation phase smoothly
  animationPhase = 0.0f;

  printAnimation();
}

void toggleAutoCycle() {

  if (autoCycle) {
    // Return to the currently active palette
    unsigned long elapsed =
      millis() - autoStartTime;

    byte steps =
      (elapsed / autoTransitionMs) % 4;

    colorMode =
      (autoStartPalette + steps) % 4;

    autoCycle = false;

    Serial.println(F("Auto cycle OFF"));
    printPalette();

  } else {

    autoCycle = true;

    autoStartPalette = colorMode;
    autoStartTime = millis();

    Serial.println(F("Auto cycle ON"));
  }
}

void resetLamp() {

  colorMode = 0;
  animationStyle = BREATHING;

  autoCycle = false;
  lampOn = true;

  animationPhase = 0.0f;

  digitalWrite(onboardLedPin, HIGH);

  Serial.println(F("Lamp RESET"));
  printPalette();
  printAnimation();
}

// ========================================
// MULTI-CLICK HANDLING
// ========================================

void executeClicks() {

  switch (clickCount) {

    case 1:
      nextPalette();
      break;

    case 2:
      togglePower();
      break;

    case 3:
      nextAnimation();
      break;

    default:
      Serial.println(F("Click sequence ignored"));
      break;
  }

  clickCount = 0;
}

// ========================================
// BUTTON INPUT
// ========================================

void handleButton() {

  unsigned long now = millis();

  bool reading = digitalRead(buttonPin);

  // Debounce
  if (reading != rawButton) {
    debounceStart = now;
    rawButton = reading;
  }

  if (now - debounceStart >= debounceMs) {

    if (reading != stableButton) {

      stableButton = reading;

      if (stableButton == LOW) {

        // Button pressed
        pressStart = now;
        holdState = 0;

      } else {

        // Button released
        if (holdState == 0) {

          clickCount++;
          lastRelease = now;

        } else {

          // Long press already handled
          clickCount = 0;
        }
      }
    }
  }

  // Handle long presses
  if (stableButton == LOW) {

    unsigned long heldFor =
      now - pressStart;

    // 1 second: toggle automatic cycling
    if (heldFor >= longPressMs &&
        holdState == 0) {

      clickCount = 0;
      toggleAutoCycle();
      holdState = 1;
    }

    // 3 seconds: reset everything
    if (heldFor >= resetPressMs &&
        holdState == 1) {

      resetLamp();
      holdState = 2;
    }
  }

  // Wait for possible second or third click
  if (stableButton == HIGH &&
      clickCount > 0 &&
      now - lastRelease > multiClickMs) {

    executeClicks();
  }
}

// ========================================
// ANIMATION WAVE FUNCTIONS
// ========================================

float pulse(float position, float center, float width) {

  float distance =
    (position - center) / width;

  return exp(-distance * distance * 4.0f);
}

float getAnimationWave(float dt) {

  if (animationStyle == STEADY) {
    return 1.0f;
  }

  float cycleSeconds =
    animationStyle == BREATHING
      ? breathingSeconds
      : heartbeatSeconds;

  animationPhase += dt / cycleSeconds;

  while (animationPhase >= 1.0f) {
    animationPhase -= 1.0f;
  }

  if (animationStyle == BREATHING) {

    // Smooth 0 -> 1 -> 0
    return 0.5f -
      0.5f * cos(2.0f * PI * animationPhase);
  }

  // HEARTBEAT
  // Two rounded pulses with a pause
  float first =
    pulse(animationPhase, 0.20f, 0.23f);

  float second =
    pulse(animationPhase, 0.43f, 0.18f);

  float result = max(first, second * 0.85f);

  return constrain(result, 0.0f, 1.0f);
}

// ========================================
// AUTO COLOR CALCULATION
// ========================================

void calculateAutoColor(
  float wave,
  float targetRGB[3]
) {

  unsigned long elapsed =
    millis() - autoStartTime;

  unsigned long step =
    elapsed / autoTransitionMs;

  float progress =
    (elapsed % autoTransitionMs) /
    (float)autoTransitionMs;

  byte fromPalette =
    (autoStartPalette + step) % 4;

  byte toPalette =
    (fromPalette + 1) % 4;

  // Smoothstep
  float smoothProgress =
    progress * progress *
    (3.0f - 2.0f * progress);

  for (int i = 0; i < 3; i++) {

    float fromColor =
      palettes[fromPalette][0][i] +
      (palettes[fromPalette][1][i] -
       palettes[fromPalette][0][i]) * wave;

    float toColor =
      palettes[toPalette][0][i] +
      (palettes[toPalette][1][i] -
       palettes[toPalette][0][i]) * wave;

    targetRGB[i] =
      fromColor +
      (toColor - fromColor) * smoothProgress;
  }
}

// ========================================
// SETUP
// ========================================

void setup() {

  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);

  pinMode(buttonPin, INPUT_PULLUP);

  pinMode(onboardLedPin, OUTPUT);
  digitalWrite(onboardLedPin, HIGH);

  setupBluePWM();

  Serial.begin(9600);

  Serial.println(F("HSIN SHADOW LAMP"));
  Serial.println(F("Single: Next palette"));
  Serial.println(F("Double: Power"));
  Serial.println(F("Triple: Animation"));
  Serial.println(F("Hold 1s: Auto cycle"));
  Serial.println(F("Hold 3s: Reset"));

  printPalette();
  printAnimation();

  lastFrame = micros();
}

// ========================================
// MAIN LOOP
// ========================================

void loop() {

  // Button stays responsive
  handleButton();

  unsigned long now = micros();
  unsigned long elapsed = now - lastFrame;

  if (elapsed < frameInterval) {
    return;
  }

  lastFrame = now;

  float dt = elapsed / 1000000.0f;

  if (dt > 0.1f) {
    dt = 0.1f;
  }

  // -------- ANIMATION --------

  float wave = getAnimationWave(dt);

  // -------- TARGET COLOR --------

  float targetRGB[3] = {0, 0, 0};

  if (lampOn) {

    if (autoCycle) {

      calculateAutoColor(wave, targetRGB);

    } else {

      for (int i = 0; i < 3; i++) {

        float deep =
          palettes[colorMode][0][i];

        float glow =
          palettes[colorMode][1][i];

        targetRGB[i] =
          deep + (glow - deep) * wave;
      }
    }
  }

  // -------- SMOOTH TRANSITIONS --------

  float blend =
    dt / (transitionSmoothness + dt);

  for (int i = 0; i < 3; i++) {

    currentRGB[i] +=
      (targetRGB[i] - currentRGB[i]) * blend;
  }

  // -------- OUTPUT --------

  setColor(
    (int)(currentRGB[0] + 0.5f),
    (int)(currentRGB[1] + 0.5f),
    (int)(currentRGB[2] + 0.5f)
  );
}
