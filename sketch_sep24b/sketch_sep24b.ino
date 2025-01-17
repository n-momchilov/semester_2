#include <FastLED.h>

#define NUM_LEDS 420
#define DATA_PIN 7
#define ECHO_PIN 12
#define TRIG_PIN 13

CRGB leds[NUM_LEDS];
int currentColorHue = 0; // Starting color hue for the effects
int spreadStep = 0; // Controls the spread of the power effect
bool isBreathing = true; // Flag to manage when to run breathing effect
bool isPowerEffectActive = false; // Flag to check if power effect is active

unsigned long lastUpdate = 0; // Timing control for effect updates
unsigned long interval = 50; // Reduced interval to make the effect faster
unsigned long movementLastDetected = 0; // Time when movement was last detected

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  pinMode(ECHO_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if movement is detected within a range of 15 cm
  if (checkMovement()) {
    if (!isPowerEffectActive) { // Start power effect if not already active
      isPowerEffectActive = true;
      isBreathing = false; // Stop breathing effect
      spreadStep = 0; // Reset the spread step at the start of the effect
      currentColorHue = (currentColorHue + 45) % 256; // Change hue each cycle
    }
  }

  // Run power effect if active
  if (isPowerEffectActive) {
    if (currentMillis - lastUpdate > interval) {
      powerEffect();
      lastUpdate = currentMillis;
    }
  }

  // Return to breathing effect if power effect is not active
  if (!isPowerEffectActive && isBreathing) {
    breathingEffect(currentMillis);
  }

  FastLED.show();
}

bool checkMovement() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2 < 300;
}

void powerEffect() {
  int centerIndex = NUM_LEDS / 2;

  // Light up LEDs from the center to the ends
  if (spreadStep < centerIndex) {
    leds[centerIndex + spreadStep] = CHSV(currentColorHue, 255, 255);
    leds[centerIndex - spreadStep] = CHSV(currentColorHue, 255, 255);
    spreadStep++;
  } else {
    spreadStep = 0; // Reset for the next cycle
    isPowerEffectActive = false; // Deactivate power effect
    isBreathing = true; // Reactivate breathing effect
  }
}

void breathingEffect(unsigned long currentMillis) {
  static unsigned long lastBreathUpdate = 0;
  static int breathBrightness = 128;
  static int breathDirection = 10; // Increased rate of change for a more dynamic effect

  // Faster paced update interval
  if (currentMillis - lastBreathUpdate > 30) { // Update interval is now 30ms
    lastBreathUpdate = currentMillis;
    breathBrightness += breathDirection;
    
    // Ensure breathBrightness stays within visible range
    if (breathBrightness < 80) {
      breathBrightness = 80; // Set minimum brightness to 80 to prevent 'off' state
      breathDirection = -breathDirection; // Reverse direction
    } else if (breathBrightness > 220) {
      breathBrightness = 220; // Set maximum brightness to 220
      breathDirection = -breathDirection; // Reverse direction
    }

    fill_solid(leds, NUM_LEDS, CHSV(currentColorHue, 255, breathBrightness));
  }
}
