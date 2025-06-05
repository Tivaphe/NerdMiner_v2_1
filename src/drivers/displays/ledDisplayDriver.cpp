#include "displayDriver.h"

#ifdef LED_DISPLAY

#include <Arduino.h>
#include "monitor.h"
#include "wManager.h"

#ifdef USE_LED
#include <FastLED.h>
#endif

#ifdef USE_LED
#define MAX_BRIGHTNESS 16
#define SLOW_FADE 1;
#define FAST_FADE 4;

CRGB leds(0, 0, 0);
int brightness = 0;
int fadeDirection = 1;
int fadeAmount = 0;
#endif // USE_LED

// Static variables for hashing LED blinking
static unsigned long previousLedToggleMillis = 0;
static bool hashingLedState = false;

bool ledOn = false;
extern monitor_data mMonitor;

void ledDisplay_Init(void)
{
  Serial.println("Led display driver initialized");
  #ifdef USE_LED
  FastLED.addLeds<RGB_LED_CLASS, RGB_LED_PIN, RGB_LED_ORDER>(&leds, 1);
  FastLED.show();
  #endif // USE_LED
}

void ledDisplay_AlternateScreenState(void)
{
  Serial.println("Switching display state");
  ledOn = !ledOn;
}

void ledDisplay_AlternateRotation(void)
{
}

void ledDisplay_NoScreen(unsigned long mElapsed)
{
  mining_data data = getMiningData(mElapsed);

  // Print hashrate to serial
  Serial.printf(">>> Completed %s share(s), %s Khashes, avg. hashrate %s KH/s\n",
                data.completedShares.c_str(), data.totalKHashes.c_str(), data.currentHashRate.c_str());

  // Print extended data to serial for no display devices
  Serial.printf(">>> Valid blocks: %s\n", data.valids.c_str());
  Serial.printf(">>> Block templates: %s\n", data.templates.c_str());
  Serial.printf(">>> Best difficulty: %s\n", data.bestDiff.c_str());
  Serial.printf(">>> 32Bit shares: %s\n", data.completedShares.c_str());
  Serial.printf(">>> Temperature: %s\n", data.temp.c_str());
  Serial.printf(">>> Total MHashes: %s\n", data.totalMHashes.c_str());
  Serial.printf(">>> Time mining: %s\n", data.timeMining.c_str());
}
void ledDisplay_LoadingScreen(void)
{
  Serial.println("Initializing...");
}

void ledDisplay_SetupScreen(void)
{
  Serial.println("Setup...");
}

// Variables para controlar el parpadeo con millis()
unsigned long previousMillis = 0;

void ledDisplay_DoLedStuff(unsigned long frame)
{

#ifdef USE_LED

    if (!ledOn)
  {
    FastLED.clear(true);
    return;
  }

  switch (mMonitor.NerdStatus)
  {
  case NM_waitingConfig:
    brightness = MAX_BRIGHTNESS;
    leds.setRGB(255, 255, 0);
    fadeAmount = 0;
    break;

  case NM_Connecting:
    leds.setRGB(0, 0, 255);
    fadeAmount = SLOW_FADE;
    break;

  case NM_hashing:
    // Change LED color to Green
    // leds.setRGB(0, 255, 0); // This will be set based on hashingLedState
    // Disable fading
    fadeAmount = 0;
    brightness = MAX_BRIGHTNESS; // Keep LED at full brightness when ON

    // Implement blinking pattern
    if (millis() - previousLedToggleMillis >= 500) {
      previousLedToggleMillis = millis();
      hashingLedState = !hashingLedState;
    }

    if (hashingLedState) {
      leds.setRGB(0, 255, 0); // Green
    } else {
      leds.setRGB(0, 0, 0); // Off
    }
    break;
  }

  // Apply fading or direct LED state for NM_hashing
  if (mMonitor.NerdStatus != NM_hashing) {
    leds.fadeLightBy(0xFF - brightness);
    brightness = brightness + (fadeDirection * fadeAmount);
    if (brightness <= 0 || brightness >= MAX_BRIGHTNESS)
    {
      fadeDirection = -fadeDirection;
    }
    brightness = constrain(brightness, 0, MAX_BRIGHTNESS);
  }
  // For NM_hashing, brightness is handled by turning LED on/off directly.
  // If fadeAmount is 0 (as in NM_waitingConfig or NM_hashing), brightness logic is skipped for those.

  FastLED.show();
#endif
}

void ledDisplay_AnimateCurrentScreen(unsigned long frame)
{
}

CyclicScreenFunction ledDisplayCyclicScreens[] = {ledDisplay_NoScreen};

DisplayDriver ledDisplayDriver = {
    ledDisplay_Init,
    ledDisplay_AlternateScreenState,
    ledDisplay_AlternateRotation,
    ledDisplay_LoadingScreen,
    ledDisplay_SetupScreen,
    ledDisplayCyclicScreens,
    ledDisplay_AnimateCurrentScreen,
    ledDisplay_DoLedStuff,
    SCREENS_ARRAY_SIZE(ledDisplayCyclicScreens),
    0,
    0,
    0,
};
#endif
