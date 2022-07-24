#include <Arduino.h>          // Basic Needs
#include <Wire.h>
#include <SPI.h>
#include <StrokeEngine.h>     // Include Stroke Engine

#include "OSSM_Config.h"      // START HERE FOR Configuration
#include "OSSM_PinDEF.h"      // This is where you set pins specific for your board
#include "OSSM_Debug.h"
#include "FastLED.h"          // Used for the LED on the Reference Board (or any other pixel LEDS you may add)
#include "Network.h"
#include "Stroker.h"
#include "Model.h"
#include "ui/Ui.h"

#define LED_BRIGHTNESS 150
#define LED_TYPE WS2811
#define LED_COLOR_ORDER GRB
#define LED_PIN 25
#define LED_COUNT 1
CRGB leds[LED_COUNT];

void setLedRainbow(CRGB leds[])
{
    // int power = 250;

    for (int hueShift = 0; hueShift < 350; hueShift++)
    {
        int gHue = hueShift % 255;
        fill_rainbow(leds, LED_COUNT, gHue, 25);
        FastLED.show();
        delay(4);
    }
}

void setup() {
    Serial.begin(115200);         // Start Serial.
    LogDebug("\n Starting");      // Start LogDebug

    pinMode(OSSM_SERVO_ALM_PIN, INPUT);
    pinMode(OSSM_SERVO_PED_PIN, INPUT);

    pinMode(OSSM_SPEED_POT_PIN, INPUT);
    adcAttachPin(OSSM_SPEED_POT_PIN);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db); // allows us to read almost full 3.3V range

    // LEDs setup
    FastLED.addLeds<LED_TYPE, LED_PIN, LED_COLOR_ORDER>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(LED_BRIGHTNESS);
    setLedRainbow(leds);
    FastLED.show();

    OSSMStroke::Network::setup();
    OSSMStroke::Ui::setup();
    OSSMStroke::Stroker::setup();
}

void loop() {
  OSSMStroke::Network::loop();
  OSSMStroke::Ui::loop();
  OSSMStroke::Stroker::loop();
}
