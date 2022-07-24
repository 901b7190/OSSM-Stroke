#include <algorithm>

#include "Model.h"
#include "OSSM_Debug.h"

namespace OSSMStroke {
    Model::Model model;

    namespace Model {
        void Model::_dispatch(Event event) {
            for (SubscribeFunc& fn : _subscriptions[event]) {
                fn(*this);
            }
        }

        void Model::subscribe(Event event, SubscribeFunc func) {
            _subscriptions[event].push_back(func);
        }

        void Model::unsubscribe(SubscribeFunc func) {
            for (auto& kv : _subscriptions) {
                auto& funcs = kv.second;
                funcs.erase(std::remove(funcs.begin(), funcs.end(), func), funcs.end());
            }
        }

        HomingStatus Model::getHomingStatus() {
            return _homingStatus;
        }
        void Model::setHomingStatus(HomingStatus status) {
            if (_homingStatus == status) return;
            _homingStatus = status;
            _dispatch(Event::HOMING_STATUS_CHANGED);
        }

        float Model::getSpeed() {
            return _speed;
        }
        void Model::setSpeed(float speed) {
            if (speed == _speed) return;
            _speed = speed;
            _dispatch(Event::SPEED_CHANGED);
        }

        float Model::getDepth() {
            return _depth;
        }
        void Model::setDepth(float depth) {
            if (depth == _depth) return;
            _depth = depth;
            _dispatch(Event::DEPTH_CHANGED);
        }

        float Model::getStroke() {
            return _stroke;
        }
        void Model::setStroke(float stroke) {
            if (stroke == _stroke) return;
            _stroke = stroke;
            _dispatch(Event::STROKE_CHANGED);
        }

        float Model::getSensation() {
            return _sensation;
        }
        void Model::setSensation(float sensation) {
            if (sensation == _sensation) return;
            _sensation = sensation;
            _dispatch(Event::SENSATION_CHANGED);
        }

        float Model::getPattern() {
            return _pattern;
        }
        void Model::setPattern(int pattern) {
            if (pattern == _pattern) return;
            _pattern = pattern;
            _dispatch(Event::PATTERN_CHANGED);
        }
    }
}
