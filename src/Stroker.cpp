#include <StrokeEngine.h>

#include "OSSM_Config.h"
#include "OSSM_Debug.h"
#include "OSSM_PinDEF.h"
#include "Model.h"
#include "Stroker.h"


namespace OSSMStroke {
    namespace Stroker {
        StrokeEngine stroker;

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
            // Real physical travel from one hard endstop to the other
            .physicalTravel = MAX_STROKEINMM,
            // Safe distance the motion is constrained to avoiding crashes
            .keepoutBoundary = STROKEBOUNDARY
        };

        // Configure Homing Procedure
        static endstopProperties endstop = {
            .homeToBack = true,                 // Endstop sits at the rear of the machine
            .activeLow = true,                  // switch is wired active low
            .endstopPin = SERVO_ENDSTOP,        // Pin number
            .pinMode = INPUT_PULLUP             // pinmode INPUT with external pull-up resistor
        };

        unsigned int getNumberOfPattern() {
            return stroker.getNumberOfPattern();
        }

        void setup() {
            LogDebug("Setting up stroker.");
            stroker.begin(&strokingMachine, &servoMotor);
            stroker.enableAndHome(
                &endstop,
                [](bool isHomed) {
                    Model::HomingStatus status = (
                        isHomed
                        ? Model::HomingStatus::SUCCEEDED
                        : Model::HomingStatus::FAILED
                    );
                    model.setHomingStatus(status);
                }
            );
            // wait for homing to complete
            while (stroker.getState() != ServoState::READY) {
                delay(100);
            }

            stroker.setSpeed(model.getSpeed(), true);
            stroker.setSensation(model.getSensation(), true);
            stroker.setDepth(model.getDepth(), true);
            stroker.setStroke(model.getStroke(), true);
            stroker.setPattern(model.getPattern(), true);

            model.subscribe(Model::Event::SPEED_CHANGED, [](Model::Model& model) {
                stroker.setSpeed(model.getSpeed(), true);
            });
            model.subscribe(Model::Event::SENSATION_CHANGED, [](Model::Model& model) {
                stroker.setSensation(model.getSensation(), true);
            });
            model.subscribe(Model::Event::DEPTH_CHANGED, [](Model::Model& model) {
                stroker.setDepth(model.getDepth(), true);
            });
            model.subscribe(Model::Event::STROKE_CHANGED, [](Model::Model& model) {
                stroker.setStroke(model.getStroke(), true);
            });
            model.subscribe(Model::Event::PATTERN_CHANGED, [](Model::Model& model) {
                stroker.setPattern(model.getPattern(), false);
            });
        }

        void loop() {}
    }
}
