#pragma once
#include <functional>
#include <map>
#include <vector>

namespace OSSMStroke {
    namespace Model {
        class Model;

        typedef void (*SubscribeFunc)(Model&);

        enum HomingStatus {
            IN_PROGRESS,
            SUCCEEDED,
            FAILED
        };

        enum MotionMode {
            STOPPED,
            PATTERN
        };

        enum Event {
            HOMING_STATUS_CHANGED,
            MOTION_MODE_CHANGED,
            SPEED_CHANGED,
            DEPTH_CHANGED,
            STROKE_CHANGED,
            SENSATION_CHANGED,
            PATTERN_CHANGED,
        };

        class Model {
            private:
                HomingStatus _homingStatus = HomingStatus::IN_PROGRESS;
                MotionMode _motionMode = MotionMode::STOPPED;
                float _speed = .0;
                float _depth = .0;
                float _stroke = .0;
                float _sensation = .0;
                int _pattern = 0;

                std::map<Event, std::vector<SubscribeFunc>> _subscriptions;

                void _dispatch(Event event);

            public:
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

                float getPattern();
                void setPattern(int pattern);
        };
    }

    extern Model::Model model;
}
