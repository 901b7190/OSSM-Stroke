#pragma once
#include <functional>
#include <map>
#include <vector>

#include "OSSM_Config.h"

namespace OSSMStroke {
    namespace Model {
        class Model;

        typedef void (*SubscribeFunc)(Model&);
        typedef struct {
            float depth;
            float speed;
            float acceleration;
        } Frame;

        enum HomingStatus {
            IN_PROGRESS,
            SUCCEEDED,
            FAILED
        };

        enum MotionMode {
            STOPPED,
            PATTERN,
            STREAMING
        };

        enum Event {
            HOMING_STATUS_CHANGED,
            MOTION_MODE_CHANGED,
            SPEED_CHANGED,
            DEPTH_CHANGED,
            STROKE_CHANGED,
            SENSATION_CHANGED,
            PATTERN_CHANGED,
            SEND_FRAME
        };

        class Model {
            private:
                HomingStatus _homingStatus = HomingStatus::IN_PROGRESS;
                MotionMode _motionMode = MotionMode::STOPPED;
                float _speed = 10.;
                float _depth = 30.;
                float _stroke = 30.;
                float _sensation = 50.;
                int _pattern = 0;
                Frame _frame;

                std::map<Event, std::vector<SubscribeFunc>> _subscriptions;

                void _dispatch(Event event);

            public:
                const float MIN_SPEED = 0.;
                const float MAX_SPEED = OSSM_USER_SPEEDLIMIT;
                const float MIN_DEPTH = 0.;
                const float MAX_DEPTH = OSSM_MAX_STROKEINMM;
                const float MIN_SENSATION = -100.;
                const float MAX_SENSATION = 100.;

                void subscribe(Event event, SubscribeFunc func);
                void unsubscribe(SubscribeFunc func);

                HomingStatus getHomingStatus();
                void setHomingStatus(HomingStatus status);

                MotionMode getMotionMode();
                void setMotionMode(MotionMode motionMode);

                float getSpeed();
                void setSpeed(float speed);

                float getDepth();
                void setDepth(float depth);

                float getStroke();
                void setStroke(float stroke);

                float getSensation();
                void setSensation(float sensation);

                int getPattern();
                void setPattern(int pattern);

                const Frame& getFrame();
                void sendFrame(float depth, float speed, float acceleration);
        };
    }

    extern Model::Model model;
}
