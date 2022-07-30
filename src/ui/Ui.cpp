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
        OssmUi ossmUi(OSSM_REMOTE_ADDRESS, OSSM_REMOTE_SDA, OSSM_REMOTE_CLK);
        TaskHandle_t remoteControlTaskHandle = nullptr;
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


        void remoteControlTask(void *pvParameters)
        {
            RotaryEncoder encoder = RotaryEncoder();
            auto encoderSign = [&]() {
                return (
                    encoder.wasTurnedLeft() ? -1
                    : encoder.wasTurnedRight() ? 1
                    : 0
                );
            };
            const std::vector<MenuState> carousel = {
                MenuState::MAIN_MENU,
                MenuState::M_SET_DEPTH,
                MenuState::M_SET_STROKE,
                MenuState::M_SET_PATTERN,
            };
            int currentCarrouselIndex = 0;
            auto nextCarouselIndex = [&](int direction) {
                currentCarrouselIndex = (currentCarrouselIndex + direction) % carousel.size();
                return currentCarrouselIndex;
            };
            float previousSpeed = -1.;

            struct ScreenPayload {
                bool apply = true;
                String message = "";
                String titleLeft = "";
                String titleRight = "";
                float stateLeft = 0.;
                float stateRight = 0.;
            };

            for (;;)
            {
                auto buttonState = encoder.checkButton();
                ScreenPayload screenPayload;
                switch (menuState)
                {
                    case MenuState::START:
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = MenuState::HOME;
                            ossmUi.clearLogo();
                        } else {
                            screenPayload.apply = false;
                        }
                        break;

                    case MenuState::HOME: {
                        auto motionMode = model.getMotionMode();
                        if (motionMode == Model::MotionMode::STREAMING) {
                            screenPayload.message = "[STREAMING MODE]";
                        } else {
                            String action = motionMode == Model::MotionMode::STOPPED ? "Start" : "Stop";
                            screenPayload.message = "Hold Down to " + action;
                        }
                        screenPayload.titleLeft = "Speed";
                        screenPayload.titleRight = "Sensation";
                        screenPayload.stateRight = MAP(
                            model.getSensation(),
                            model.MIN_SENSATION,
                            model.MAX_SENSATION,
                            0.,
                            100.
                        );
                        if (buttonState == RotaryEncoder::ButtonState::VERY_LONG) {
                            switch (motionMode) {
                                case Model::MotionMode::STOPPED:
                                    model.setMotionMode(Model::MotionMode::PATTERN);
                                    break;
                                default:
                                    model.setMotionMode(Model::MotionMode::STOPPED);
                                    break;
                            }
                        } else if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = MenuState::MAIN_MENU;
                        } else if (auto sign = encoderSign()) {
                            auto sensation = model.getSensation() + sign * (200 / OSSM_ENCODER_RESULTION);
                            model.setSensation(sensation);
                            continue;
                        }
                        break;
                    }

                    case MenuState::MAIN_MENU: {
                        screenPayload.message = "Home";
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = MenuState::HOME;
                        } else if (auto sign = encoderSign()) {
                            menuState = carousel[nextCarouselIndex(sign)];
                        }
                        break;
                    }

                    case M_SET_DEPTH:
                        screenPayload.message = "Set Depth";
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = MenuState::OPT_SET_DEPTH;
                        } else if (auto sign = encoderSign()) {
                            menuState = carousel[nextCarouselIndex(sign)];
                        }
                        break;

                    case OPT_SET_DEPTH:
                        screenPayload.message = "->Set Depth<-";
                        screenPayload.titleRight = "Depth";
                        screenPayload.stateRight = MAP(model.getDepth(), model.MIN_DEPTH, model.MAX_DEPTH, 0., 100.);
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = MenuState::M_SET_DEPTH;
                        } else if (auto sign = encoderSign()) {
                            auto depth = model.getDepth() + sign * OSSM_DEPTH_RESULTION;
                            model.setDepth(depth);
                            continue;
                        }
                        break;

                    case M_SET_STROKE:
                        screenPayload.message = "Set Stroke";
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = MenuState::OPT_SET_STROKE;
                        } else if (auto sign = encoderSign()) {
                            menuState = carousel[nextCarouselIndex(sign)];
                        }
                        break;

                    case OPT_SET_STROKE:
                        screenPayload.message = "->Set Stroke<-";
                        screenPayload.titleRight = "Stroke";
                        screenPayload.stateRight = MAP(model.getStroke(), model.MIN_DEPTH, model.MAX_DEPTH, 0., 100.);
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = MenuState::M_SET_STROKE;
                        } else if (auto sign = encoderSign()) {
                            auto depth = model.getStroke() + sign * OSSM_STROKE_RESULTION;
                            model.setStroke(depth);
                            continue;
                        }
                        break;

                    case M_SET_PATTERN:
                        screenPayload.message = "Set Pattern";
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = MenuState::OPT_SET_PATTERN;
                        } else if (auto sign = encoderSign()) {
                            menuState = carousel[nextCarouselIndex(sign)];
                        }
                        break;

                    case OPT_SET_PATTERN:
                        screenPayload.message = Stroker::getPatternName(model.getPattern());
                        if (buttonState == RotaryEncoder::ButtonState::LONG) {
                            menuState = MenuState::M_SET_PATTERN;
                        } else if (auto sign = encoderSign()) {
                            auto pattern = (model.getPattern() + sign) % Stroker::getNumberOfPattern();
                            model.setPattern(pattern);
                            continue;
                        }
                        break;
                }

                screenPayload.titleLeft = "Speed";
                auto speedPercent = getAnalogAverage(OSSM_SPEED_POT_PIN, 200);
                auto newSpeed = MAP(
                    speedPercent,
                    OSSM_SPEED_POT_DEADZONE_PERCENT,
                    100. - OSSM_SPEED_POT_DEADZONE_PERCENT,
                    model.MIN_SPEED,
                    model.MAX_SPEED
                );
                if (1. < abs(newSpeed - previousSpeed)) {
                    // This condition prevents spamming setSpeed at each iteration.
                    // It makes the speed overridable by some other task (e.g. API).
                    model.setSpeed(newSpeed);
                    previousSpeed = newSpeed;
                }
                // Get the speed from model and not from "speedPercent" to
                // update screen in case of external change (e.g. API).
                screenPayload.stateLeft = MAP(
                    model.getSpeed(),
                    model.MIN_SPEED,
                    model.MAX_SPEED,
                    0,
                    100.
                );

                if (screenPayload.apply) {
                    ossmUi.UpdateMessage(screenPayload.message);
                    ossmUi.UpdateTitelL(screenPayload.titleLeft);
                    ossmUi.UpdateTitelR(screenPayload.titleRight);
                    ossmUi.UpdateStateL(screenPayload.stateLeft);
                    ossmUi.UpdateStateR(screenPayload.stateRight);
                }

                delay(100);
            }
        }

        void emergencyStopTask(void *pvParameters)
        {
            // Currently uses remote control ethernet cable as kill-switch.

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
                    vTaskResume(remoteControlTaskHandle);
                } else if (isConnected && !ossmUi.DisplayIsConnected()) {
                    LogDebug("Display Disconnected");
                    isConnected = false;
                    model.setMotionMode(Model::MotionMode::STOPPED);
                    vTaskSuspend(remoteControlTaskHandle);
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
            xTaskCreate(
                remoteControlTask,          // Task function.
                "remoteControlTask",        // name of task.
                4096,                       // Stack size of task
                NULL,                       // parameter of the task
                2,                          // priority of the task
                &remoteControlTaskHandle    // Task handle to keep track of created task
            );
            xTaskCreate(
                emergencyStopTask,          // Task function.
                "emergencyStopTask",        // name of task.
                2048,                       // Stack size of task
                NULL,                       // parameter of the task
                2,                          // priority of the task
                &emergencyStopTaskHandle    // Task handle to keep track of created task
            );
        }

        void loop() {
            ossmUi.UpdateScreen();
        }
    }
}
