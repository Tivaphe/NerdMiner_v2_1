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
static bool nextColorIsRed = true; // To alternate Red/Blue, true = Red, false = Blue
static int previousNerdStatusLed = -1; // To detect state changes for resetting LED

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

    if (mMonitor.NerdStatus != NM_hashing && previousNerdStatusLed == NM_hashing) {
        hashingLedState = false;      // Ensure LED is marked OFF
        nextColorIsRed = true;        // Reset color sequence for next time
        leds = CRGB::Black;           // Explicitly turn LED off
        FastLED.show();               // Update LED
    }
    previousNerdStatusLed = mMonitor.NerdStatus;

    if (!ledOn)
    {
        FastLED.clear(true);
        return;
    }

    if (mMonitor.NerdStatus == NM_hashing) {
        float currentKHs = mMonitor.currentHashRateKHs;
        unsigned long blinkDelayMs = 750;

        if (currentKHs > 0.1) {
            blinkDelayMs = (unsigned long)(1000.0 / (currentKHs * 0.2 + 0.8));
        }
        blinkDelayMs = constrain(blinkDelayMs, 75, 750);

        if (millis() - previousLedToggleMillis >= blinkDelayMs) {
            hashingLedState = !hashingLedState;
            previousLedToggleMillis = millis();

            if (hashingLedState) {
                if (nextColorIsRed) {
                    leds.setRGB(255, 0, 0); // Red
                } else {
                    leds.setRGB(0, 0, 255); // Blue
                }
                nextColorIsRed = !nextColorIsRed;
            } else {
                leds.setRGB(0, 0, 0); // Off
            }
        }

        FastLED.show();
        return;
    }

    // Logic for other states (NM_waitingConfig, NM_Connecting)
    switch (mMonitor.NerdStatus)
    {
    case NM_waitingConfig:
        brightness = MAX_BRIGHTNESS;
        leds.setRGB(255, 255, 0); // Yellow
        fadeAmount = 0;
        break;

    case NM_Connecting:
        leds.setRGB(0, 0, 255); // Blue
        fadeAmount = SLOW_FADE;
        break;

    // NM_hashing is handled above and returns
    default:
        // Potentially handle other unknown states or do nothing
        break;
    }

    // Apply fading for states other than NM_hashing
    // This block is now only reached if not NM_hashing
    leds.fadeLightBy(0xFF - brightness);
    brightness = brightness + (fadeDirection * fadeAmount);
    if (brightness <= 0 || brightness >= MAX_BRIGHTNESS)
    {
        fadeDirection = -fadeDirection;
    }
    brightness = constrain(brightness, 0, MAX_BRIGHTNESS);

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
