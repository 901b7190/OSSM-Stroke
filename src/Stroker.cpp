#include <deque>

#include <esp_timer.h>
#include <StrokeEngine.h>

#include "OSSM_Config.h"
#include "OSSM_Debug.h"
#include "OSSM_PinDEF.h"
#include "Model.h"
#include "Stroker.h"


namespace OSSMStroke {
    namespace Stroker {
        StrokeEngine stroker;

        TaskHandle_t streamingTaskHandle = nullptr;
        QueueHandle_t framesBuffer = xQueueCreate(
            OSSM_MAX_STREAMING_FRAME_BUFFER_SIZE,
            sizeof(Model::Frame)
        );
        unsigned short framesBufferId = 0;

        static motorProperties servoMotor {
            .maxSpeed = OSSM_MAX_SPEED,                // Maximum speed the system can go in mm/s
            .maxAcceleration = OSSM_MAX_ACCELERATION,  // Maximum linear acceleration in mm/s²
            .stepsPerMillimeter = OSSM_STEP_PER_MM,    // Steps per millimeter
            .invertDirection = true,              // One of many ways to change the direction,
                                                  // should things move the wrong way
            .enableActiveLow = true,              // Polarity of the enable signal
            .stepPin = OSSM_SERVO_PULSE,               // Pin of the STEP signal
            .directionPin = OSSM_SERVO_DIR,            // Pin of the DIR signal
            .enablePin = OSSM_SERVO_ENABLE             // Pin of the enable signal
        };

        static machineGeometry strokingMachine = {
            // Real physical travel from one hard endstop to the other
            .physicalTravel = OSSM_MAX_STROKEINMM,
            // Safe distance the motion is constrained to avoiding crashes
            .keepoutBoundary = OSSM_STROKEBOUNDARY
        };

        // Configure Homing Procedure
        static endstopProperties endstop = {
            .homeToBack = true,                 // Endstop sits at the rear of the machine
            .activeLow = true,                  // switch is wired active low
            .endstopPin = OSSM_SERVO_ENDSTOP,   // Pin number
            .pinMode = INPUT_PULLUP             // pinmode INPUT with external pull-up resistor
        };

        unsigned int getNumberOfPattern() {
            return stroker.getNumberOfPattern();
        }

        String getPatternName(int index) {
            return stroker.getPatternName(index);
        }


        void streamingTask(void *pvParameters) {
            auto currentTimeMs = [](){ return static_cast<unsigned long>(esp_timer_get_time() / 1000); };
            auto beginningOfTime = currentTimeMs();
            auto currentFramesBufferId = framesBufferId;
            Model::Frame frame;
            Model::Frame nextFrame;

            for (;;) {
                if (xQueueReceive(framesBuffer, &frame, 100000 / portTICK_PERIOD_MS)) {
                    currentFramesBufferId = framesBufferId;

                    if (frame.time == 0) {
                        beginningOfTime = currentTimeMs();
                    }

                    auto currentTime = currentTimeMs() - beginningOfTime;
                    if (currentTime < frame.time) {
                        auto delay = frame.time - currentTime;
                        LogDebugFormatted("[OSSM STREAMING] Scheduling frame in %dms.\n", delay);
                        vTaskDelay(delay / portTICK_PERIOD_MS);
                    } else if (
                        xQueuePeek(framesBuffer, &nextFrame, 0)
                        && nextFrame.time < currentTime
                    ) {
                        // Just check that the next frame time isn't also
                        // expired to avoid spamming frames under high load.
                        LogDebug("[OSSM STREAMING] Skipping frame. Probably lagging.");
                        continue;
                    }

                    if (currentFramesBufferId != framesBufferId) {
                        // The buffer was cleared while we were waiting.
                        continue;
                    }

                    LogDebugFormatted(
                        "[OSSM STREAMING] Sending frame. drift=%dms\n",
                        currentTimeMs() - beginningOfTime - frame.time
                    );
                    stroker.setFrame(frame.depth, frame.speed, frame.acceleration);
                }
            }
        }

        BaseType_t startStreamingTask() {
            return xTaskCreate(
                streamingTask,              // Task function.
                "ossmStreamingTask",        // name of task.
                2048,                       // Stack size of task
                NULL,                       // parameter of the task
                3,                          // priority of the task
                &streamingTaskHandle        // Task handle to keep track of created task
            );
        }

        void clearFramesBuffer() {
            xQueueReset(framesBuffer);
            framesBufferId++;
            xTaskAbortDelay(streamingTaskHandle);
        }

        void suspendStreamingTask() {
            clearFramesBuffer();
            vTaskSuspend(streamingTaskHandle);
        }

        void resumeStreamingTask() {
            vTaskResume(streamingTaskHandle);
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

            if (startStreamingTask()) {
                suspendStreamingTask();
            } else {
                LogDebug("ERROR! Failed to start OSSM streaming task.");
            }

            stroker.setSpeed(model.getSpeed(), true);
            stroker.setSensation(model.getSensation(), true);
            stroker.setDepth(model.getDepth(), true);
            stroker.setStroke(model.getStroke(), true);
            stroker.setPattern(model.getPattern(), true);

            model.subscribe(Model::Event::SPEED_CHANGED, [](Model::Model& model) {
                if (model.getMotionMode() == Model::MotionMode::PATTERN) {
                    stroker.setSpeed(model.getSpeed(), true);
                }
            });
            model.subscribe(Model::Event::SENSATION_CHANGED, [](Model::Model& model) {
                if (model.getMotionMode() == Model::MotionMode::PATTERN) {
                    stroker.setSensation(model.getSensation(), true);
                }
            });
            model.subscribe(Model::Event::DEPTH_CHANGED, [](Model::Model& model) {
                if (model.getMotionMode() == Model::MotionMode::PATTERN) {
                    stroker.setDepth(model.getDepth(), true);
                }
            });
            model.subscribe(Model::Event::STROKE_CHANGED, [](Model::Model& model) {
                if (model.getMotionMode() == Model::MotionMode::PATTERN) {
                    stroker.setStroke(model.getStroke(), true);
                }
            });
            model.subscribe(Model::Event::PATTERN_CHANGED, [](Model::Model& model) {
                auto pattern = model.getPattern();
                if (!(0 < pattern || pattern < stroker.getNumberOfPattern())) {
                    pattern = 0;
                }
                stroker.setPattern(model.getPattern(), false);
            });
            model.subscribe(Model::Event::SEND_FRAME, [](Model::Model& model) {
                if (model.getMotionMode() == Model::MotionMode::STREAMING) {
                    auto frame = model.getFrame();
                    if (!xQueueSend(framesBuffer, (void*)&frame, 0)) {
                        LogDebug("ERROR! Maximum frame buffer size reached. Frame skipped. Consider throttling.");
                        return;
                    }
                }
            });
            model.subscribe(Model::Event::CLEAR_FRAMES, [](Model::Model& model) {
                clearFramesBuffer();
            });

            model.subscribe(Model::Event::MOTION_MODE_CHANGED, [](Model::Model& model) {
                Model::MotionMode motionMode = model.getMotionMode();

                if (motionMode != Model::MotionMode::STREAMING) {
                    suspendStreamingTask();
                }

                switch (motionMode) {
                    case Model::MotionMode::PATTERN:
                        if (!stroker.startPattern()) {
                            LogDebug("Failed to start pattern. Stopping motion.");
                            model.setMotionMode(Model::MotionMode::STOPPED);
                        }
                        break;
                    case Model::MotionMode::STREAMING:
                        if (stroker.startStreaming()) {
                            resumeStreamingTask();
                        } else {
                            LogDebug("Failed to start StrokeEngine streaming mode.");
                            model.setMotionMode(Model::MotionMode::STOPPED);
                        }
                        break;
                    default:
                        stroker.stopMotion();
                        break;
                }
            });
        }

        void loop() {}
    }
}
