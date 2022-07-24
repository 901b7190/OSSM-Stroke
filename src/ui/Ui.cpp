#include <OssmUi.h>

#include "OSSM_Debug.h"
#include "OSSM_PinDEF.h"
#include "Stroker.h"
#include "Model.h"
#include "ui/RotaryEncoder.h"
#include "ui/Ui.h"

namespace OSSMStroke {
    namespace Ui {
        MenuState menuState = MenuState::START;
        OssmUi ossmUi(REMOTE_ADDRESS, REMOTE_SDA, REMOTE_CLK);
        TaskHandle_t screenTaskHandle = nullptr;
        TaskHandle_t emergencyStopTaskHandle = nullptr;

        void onHomingChanged(Model::Model& model) {
            LogDebug("Homing status changed.");

            auto status = model.getHomingStatus();

            char const *msg = nullptr;
            if (status == Model::HomingStatus::SUCCEEDED) {
                msg = "Homed - Ready to rumble!";
            } else if (status == Model::HomingStatus::FAILED) {
                msg = "Homing failed!";
            }
            if (msg != NULL) {
                LogDebug(msg);
                ossmUi.UpdateMessage(msg);
            }
        }

        float getAnalogAverage(int pinNumber, int samples) {
            float sum = 0;
            float average = 0;
            float percentage = 0;
            for (int i = 0; i < samples; i++)
            {
                // TODO: Possibly use fancier filters?
                sum += analogRead(pinNumber);
            }
            average = sum / samples;
            // TODO: Might want to add a deadband

            percentage = 100.0 * average / 4096.0; // 12 bit resolution
            return percentage;
        }

        void screenTask(void *pvParameters)
        {
            auto& stroker = Stroker::stroker;
            RotaryEncoder encoder = RotaryEncoder();
            unsigned int patternN = Stroker::getNumberOfPattern();

            for (;;)
            {
                auto buttonState = encoder.checkButton();
                switch (menuState)
                {
                    case MenuState::M_SET_SENSATION:
                    case MenuState::OPT_SET_SENSATION:
                        break;

                    case MenuState::START:
                        if (buttonState == RotaryEncoder::ButtonState::LONG){
                            menuState = HOME;
                            ossmUi.clearLogo();
                            ossmUi.UpdateMessage("Hold Down to Start");
                            ossmUi.UpdateTitelL("Speed");
                            ossmUi.UpdateTitelR("Sensation");
                            ossmUi.UpdateStateR(map(model.getSensation(),-100,100,0,100));
                        }
                        break;

                    case MenuState::HOME:
                        if (buttonState == RotaryEncoder::ButtonState::VERY_LONG) {
                            if (stroker.getState() == ServoState::PATTERN){
                                stroker.stopMotion();
                            } else {
                                stroker.startPattern();
                            }
                        } else if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            ossmUi.UpdateMessage("Home");
                            ossmUi.UpdateTitelR("");
                            ossmUi.UpdateStateR(.0);
                            menuState = M_MENUE;
                        } else if (encoder.wasTurnedLeft()) {
                            auto sensation = model.getSensation();
                            sensation = constrain((sensation - (200/ENCODER_RESULTION)), -100, 100);
                            model.setSensation(sensation);
                            stroker.setSensation(sensation, false);
                            ossmUi.UpdateStateR(map(sensation,-100,100,0,100));
                        } else if (encoder.wasTurnedRight()) {
                            auto sensation = model.getSensation();
                            sensation = constrain((sensation + (200/ENCODER_RESULTION)), -100, 100);
                            model.setSensation(sensation);
                            stroker.setSensation(sensation, false);
                            ossmUi.UpdateStateR(map(sensation,-100,100,0,100));
                        }
                        break;

                    case M_MENUE:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            auto sensation = model.getSensation();
                            ossmUi.UpdateMessage("Hold For Start");
                            ossmUi.UpdateTitelR("Sensation");
                            ossmUi.UpdateStateR(map(sensation,-100,100,0,100));
                            menuState = HOME;
                        } else if (encoder.wasTurnedLeft()) {
                            ossmUi.UpdateMessage("Depth Fancy");
                            menuState = M_SET_DEPTH_FANCY;
                        } else if (encoder.wasTurnedRight()) {
                            ossmUi.UpdateMessage("Set Depth");
                            menuState = M_SET_DEPTH;
                        }
                        break;

                    case M_SET_DEPTH:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = OPT_SET_DEPTH;
                            ossmUi.UpdateMessage("->Set Depth<-");
                            auto depth = model.getDepth();
                            ossmUi.UpdateStateR(map(depth,0,MAX_STROKEINMM,0,100));
                            ossmUi.UpdateTitelR("Depth");
                        } else if (encoder.wasTurnedLeft()) {
                            ossmUi.UpdateMessage("Home");
                            menuState = M_MENUE;
                        } else if (encoder.wasTurnedRight()) {
                            ossmUi.UpdateMessage("Set Stroke");
                            menuState = M_SET_STROKE;
                        }
                        break;

                    case OPT_SET_DEPTH:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = M_SET_DEPTH;
                            ossmUi.UpdateMessage("Set Depth");
                            ossmUi.UpdateStateR(.0);
                            ossmUi.UpdateTitelR("");
                        } else if (encoder.wasTurnedLeft()) {
                            auto depth = model.getDepth();
                            depth = constrain((depth - DEPTH_RESULTION) , 0, MAX_STROKEINMM);
                            model.setDepth(depth);
                            stroker.setDepth(depth, false);
                            ossmUi.UpdateStateR(map(depth,0,MAX_STROKEINMM,0,100));
                            LogDebug(depth);
                        } else if (encoder.wasTurnedRight()) {
                            auto depth = model.getDepth();
                            depth = constrain((depth + DEPTH_RESULTION) , 0, MAX_STROKEINMM);
                            model.setDepth(depth);
                            stroker.setDepth(depth, false);
                            ossmUi.UpdateStateR(map(depth,0,MAX_STROKEINMM,0,100));
                            LogDebug(depth);
                        }
                        break;

                    case M_SET_STROKE:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            auto stroke = model.getStroke();
                            menuState = OPT_SET_STROKE;
                            ossmUi.UpdateMessage("->Set Stroke<-");
                            ossmUi.UpdateStateR(map(stroke,0,MAX_STROKEINMM,0,100));
                            ossmUi.UpdateTitelR("Stroke");
                        } else if (encoder.wasTurnedLeft()) {
                            ossmUi.UpdateMessage("Set Depth");
                            menuState = M_SET_DEPTH;
                        } else if (encoder.wasTurnedRight()) {
                            ossmUi.UpdateMessage("Set Pattern");
                            menuState = M_SET_PATTERN;
                        }
                        break;

                    case OPT_SET_STROKE:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = M_SET_STROKE;
                            ossmUi.UpdateMessage("Set Stroke");
                            ossmUi.UpdateStateR(.0);
                            ossmUi.UpdateTitelR("");
                        } else if (encoder.wasTurnedLeft()) {
                            auto stroke = model.getStroke();
                            stroke = constrain((stroke - STROKE_RESULTION) , 0, MAX_STROKEINMM);
                            model.setStroke(stroke);
                            stroker.setStroke(stroke, false);
                            ossmUi.UpdateStateR(map(stroke,0,MAX_STROKEINMM,0,100));
                            LogDebug(stroke);
                        } else if (encoder.wasTurnedRight()) {
                            auto stroke = model.getStroke();
                            stroke = constrain((stroke + STROKE_RESULTION) , 0, MAX_STROKEINMM);
                            model.setStroke(stroke);
                            stroker.setStroke(stroke, false);
                            ossmUi.UpdateStateR(map(stroke,0,MAX_STROKEINMM,0,100));
                            LogDebug(stroke);
                        }
                        break;

                    case M_SET_PATTERN:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            auto pattern = model.getPattern();
                            menuState = OPT_SET_PATTERN;
                            ossmUi.UpdateMessage("->Select Pattern<-");
                            ossmUi.UpdateMessage(stroker.getPatternName(pattern));
                            ossmUi.UpdateTitelR("Pattern");
                        } else if (encoder.wasTurnedLeft()) {
                            ossmUi.UpdateMessage("Set Stroke");
                            menuState = M_SET_STROKE;
                        } else if (encoder.wasTurnedRight()) {
                            ossmUi.UpdateMessage("Inter. Depth");
                            menuState = M_SET_DEPTH_INT;
                        }
                        break;

                    case OPT_SET_PATTERN:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            auto pattern = model.getPattern();
                            menuState = M_SET_PATTERN;
                            ossmUi.UpdateMessage("Select Pattern");
                            ossmUi.UpdateStateR(.0);
                            ossmUi.UpdateTitelR("");
                            model.setPattern(pattern);
                            stroker.setPattern(pattern, false);
                        } else if (encoder.wasTurnedLeft()) {
                            auto pattern = model.getPattern();
                            pattern = constrain((pattern - 1), 0, patternN);
                            ossmUi.UpdateMessage(stroker.getPatternName(pattern));
                        } else if (encoder.wasTurnedRight()) {
                            auto pattern = model.getPattern();
                            pattern = constrain((pattern + 1), 0, patternN);
                            ossmUi.UpdateMessage(stroker.getPatternName(pattern));
                        }
                        break;

                    case M_SET_DEPTH_INT:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            float defaultDepth = 10.;
                            menuState = OPT_SET_DEPTH_INT;
                            model.setDepth(defaultDepth);
                            stroker.setupDepth(defaultDepth, false);
                            ossmUi.UpdateTitelR("Depth");
                            ossmUi.UpdateStateR(map(defaultDepth,0,MAX_STROKEINMM,0,100));
                            ossmUi.UpdateMessage("->Inter. Depth<-");
                        } else if (encoder.wasTurnedLeft()) {
                            ossmUi.UpdateMessage("Set Pattern");
                            menuState = M_SET_PATTERN;
                        } else if (encoder.wasTurnedRight()) {
                            ossmUi.UpdateMessage("Depth Fancy");
                            menuState = M_SET_DEPTH_FANCY;
                        }
                        break;

                    case OPT_SET_DEPTH_INT:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = M_SET_DEPTH_INT;
                            ossmUi.UpdateMessage("Inter. Depth");
                            ossmUi.UpdateStateR(.0);
                            ossmUi.UpdateTitelR("");
                        } else if (encoder.wasTurnedLeft()) {
                            auto depth = model.getDepth();
                            depth = constrain((depth - DEPTH_RESULTION) , 0, MAX_STROKEINMM);
                            model.setDepth(depth);
                            stroker.setDepth(depth, true);
                            ossmUi.UpdateStateR(map(depth,0,MAX_STROKEINMM,0,100));
                            LogDebug(depth);
                        } else if (encoder.wasTurnedRight()) {
                            auto depth = model.getDepth();
                            depth = constrain((depth + DEPTH_RESULTION) , 0, MAX_STROKEINMM);
                            model.setDepth(depth);
                            stroker.setDepth(depth, true);
                            ossmUi.UpdateStateR(map(depth,0,MAX_STROKEINMM,0,100));
                            LogDebug(depth);
                            break;
                        }
                        break;

                    case M_SET_DEPTH_FANCY:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            float defaultDepth = 10.;
                            menuState = OPT_SET_DEPTH_FANCY;
                            ossmUi.UpdateMessage("->Depth Fancy<-");
                            model.setDepth(defaultDepth);
                            stroker.setupDepth(defaultDepth, true);
                            ossmUi.UpdateTitelR("Depth");
                            ossmUi.UpdateStateR(map(defaultDepth,0,MAX_STROKEINMM,0,100));
                        } else if (encoder.wasTurnedLeft()) {
                            ossmUi.UpdateMessage("Inter. Depth");
                            menuState = M_SET_DEPTH_INT;
                        } else if (encoder.wasTurnedRight()) {
                            ossmUi.UpdateMessage("Home");
                            menuState = M_MENUE;
                        }
                        break;

                    case OPT_SET_DEPTH_FANCY:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = M_SET_DEPTH_FANCY;
                            ossmUi.UpdateMessage("Depth Fancy");
                            ossmUi.UpdateStateR(.0);
                            ossmUi.UpdateTitelR("");
                        } else if (encoder.wasTurnedLeft()) {
                            float depth = model.getDepth();
                            depth = constrain((depth - DEPTH_RESULTION) , 0, MAX_STROKEINMM);
                            model.setDepth(depth);
                            stroker.setDepth(depth, true);
                            ossmUi.UpdateStateR(map(depth,0,MAX_STROKEINMM,0,100));
                            LogDebug(depth);
                        } else if (encoder.wasTurnedRight()) {
                            float depth = model.getDepth();
                            depth = constrain((depth + DEPTH_RESULTION) , 0, MAX_STROKEINMM);
                            model.setDepth(depth);
                            stroker.setDepth(depth, true);
                            ossmUi.UpdateStateR(map(depth,0,MAX_STROKEINMM,0,100));
                            LogDebug(depth);
                        }
                        break;
                }

                // get average analog reading, function takes pin and # samples
                auto speed = getAnalogAverage(SPEED_POT_PIN, 200);
                ossmUi.UpdateStateL(speed);
                //LogDebug(speed);
                speed = fscale(0.00, 99.98, 0.5, USER_SPEEDLIMIT, speed, -1);
                model.setSpeed(speed);
                stroker.setSpeed(speed, true);
                delay(100);
            }
        }

        void emergencyStopTask(void *pvParameters)
        {
            bool isConnected = false;
            for (;;)
            {
                // bool alm = digitalRead(SERVO_ALM_PIN);
                // bool ped = digitalRead(SERVO_PED_PIN);
                //LogDebugFormatted("ALM: %ld \n", static_cast<long int>(alm));
                //LogDebugFormatted("PED: %ld \n", static_cast<long int>(ped));
                if (!isConnected && ossmUi.DisplayIsConnected()) {
                    LogDebug("Display Connected");
                    isConnected = true;
                    vTaskResume(screenTaskHandle);
                } else if (isConnected && !ossmUi.DisplayIsConnected()) {
                    LogDebug("Display Disconnected");
                    isConnected = false;
                    // if(Stroker.getState() == PATTERN){
                    Stroker::stroker.stopMotion();
                    // }
                    vTaskSuspend(screenTaskHandle);
                }
                vTaskDelay(200);
            }
        }

        void setup() {
            model.subscribe(
              Model::Event::HOMING_STATUS_CHANGED,
              onHomingChanged
            );

            ossmUi.Setup();
            ossmUi.UpdateOnly();
            xTaskCreatePinnedToCore(
                screenTask,         // Task function.
                "screenTask",       // name of task.
                4096,               // Stack size of task
                NULL,               // parameter of the task
                5,                  // priority of the task
                &screenTaskHandle,  // Task handle to keep track of created task
                0                   // pin task to core 0
            );
            xTaskCreatePinnedToCore(
                emergencyStopTask,          // Task function.
                "emergencyStopTask",        // name of task.
                2048,                       // Stack size of task
                NULL,                       // parameter of the task
                1,                          // priority of the task
                &emergencyStopTaskHandle,   // Task handle to keep track of created task
                0                           // pin task to core 0
            );
        }

        void loop() {
            ossmUi.UpdateScreen();
        }
    }
}
