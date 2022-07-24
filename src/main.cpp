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


static motorProperties servoMotor {
  .maxSpeed = MAX_SPEED,                // Maximum speed the system can go in mm/s
  .maxAcceleration = MAX_ACCELERATION,  // Maximum linear acceleration in mm/s²
  .stepsPerMillimeter = STEP_PER_MM,    // Steps per millimeter
  .invertDirection = true,              // One of many ways to change the direction,
                                        // should things move the wrong way
  .enableActiveLow = true,              // Polarity of the enable signal
  .stepPin = SERVO_PULSE,               // Pin of the STEP signal
  .directionPin = SERVO_DIR,            // Pin of the DIR signal
  .enablePin = SERVO_ENABLE             // Pin of the enable signal
};

static machineGeometry strokingMachine = {
  .physicalTravel = MAX_STROKEINMM,            // Real physical travel from one hard endstop to the other
  .keepoutBoundary = STROKEBOUNDARY              // Safe distance the motion is constrained to avoiding crashes
};

// Configure Homing Procedure
static endstopProperties endstop = {
  .homeToBack = true,                // Endstop sits at the rear of the machine
  .activeLow = true,                  // switch is wired active low
  .endstopPin = SERVO_ENDSTOP,        // Pin number
  .pinMode = INPUT_PULLUP             // pinmode INPUT with external pull-up resistor
};

// Create tasks for checking pot input or web server control, and task to handle
// planning the motion profile (this task is high level only and does not pulse
// the stepper!)

TaskHandle_t estop_T    = nullptr;  // Estop Taks for Emergency

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
    delay(200);

    // LEDs setup
    FastLED.addLeds<LED_TYPE, LED_PIN, LED_COLOR_ORDER>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(LED_BRIGHTNESS);
    setLedRainbow(leds);
    FastLED.show();

    // Network setup
    networkSetup();

    // OLED SETUP
    OSSMStroke::Ui::setup();

    // Setup Stroke Engine
    LogDebug("Setting up stroker.");
    OSSMStroke::Stroker::stroker.begin(&strokingMachine, &servoMotor);
    // pointer to the homing config struct
    OSSMStroke::Stroker::stroker.enableAndHome(
        &endstop,
        [](bool isHomed) {
            OSSMStroke::Model::HomingStatus status = (
              isHomed
              ? OSSMStroke::Model::HomingStatus::SUCCEEDED
              : OSSMStroke::Model::HomingStatus::FAILED
            );
            OSSMStroke::model.setHomingStatus(status);
        }
    );

    delay(100);

    pinMode(SERVO_ALM_PIN, INPUT);
    pinMode(SERVO_PED_PIN, INPUT);

    pinMode(SPEED_POT_PIN, INPUT);
    adcAttachPin(SPEED_POT_PIN);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db); // allows us to read almost full 3.3V range

    // wait for homing to complete
    while (OSSMStroke::Stroker::stroker.getState() != READY) {
        delay(100);
    }
}

void loop() {
  networkLoop();
  OSSMStroke::Ui::loop();
}
