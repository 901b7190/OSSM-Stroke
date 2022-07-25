#include <string>

#include <ArduinoJson.h>

#include "network/Controller.h"
#include "Model.h"

namespace OSSMStroke {
    namespace Network {
        namespace Controller {
            typedef void (*FloatHandler)(float value);
            void processMappableMutation(
                const JsonVariant& doc,
                JsonDocument& result,
                String key,
                float minOut,
                float maxOut,
                FloatHandler handler
            ) {
                if (!doc[key].is<float>()) {
                    result["code"] = 400;
                    result["error"] = "invalid_" + key;
                    result["message"] = "\"" + key + "\" must be of float type.";
                    return;
                }

                float value = doc[key].as<float>();
                if (doc.containsKey("unit")) {
                    if (!doc["unit"].is<String>()) {
                        result["code"] = 400;
                        result["error"] = "invalid_unit";
                        result["message"] = "unit must be a string (e.g. \"percent\").";
                        return;
                    }
                    String unit = doc["unit"].as<String>();
                    if (unit == "percent") {
                        value = MAP(value, 0., 100., minOut, maxOut);
                    }
                }

                handler(value);
                result["type"] = "ack";
            }

            void processMappableQuery(
                const JsonVariant& doc,
                JsonDocument& result,
                String key,
                float value,
                float minIn,
                float maxIn
            ) {
                if (doc.containsKey("unit")) {
                    if (!doc["unit"].is<String>()) {
                        result["code"] = 400;
                        result["error"] = "invalid_unit";
                        result["message"] = "unit must be a string (e.g. \"percent\").";
                        return;
                    }
                    String unit = doc["unit"].as<String>();
                    if (unit == "percent") {
                        value = MAP(value, minIn, maxIn, 0., 100.);
                    }
                }

                result["type"] = "get_" + key;
                result[key] = value;
            }

            void processMessage(const JsonVariant& doc, JsonDocument& result) {
                if (!doc.is<JsonObject>()) {
                    result["code"] = 400;
                    result["error"] = "not_json_object";
                    result["message"] = "Input data should be a JSON object.";
                    return;
                }

                if (!doc["type"].is<String>()) {
                    result["code"] = 400;
                    result["error"] = "missing_type";
                    result["message"] = "You must specify a message type.";
                    return;
                }

                String type = doc["type"].as<String>();
                if (type == "set_speed") {
                    processMappableMutation(
                        doc,
                        result,
                        "speed",
                        model.MIN_SPEED,
                        model.MAX_SPEED,
                        [](float speed) { model.setSpeed(speed); }
                    );
                } else if (type == "set_depth") {
                    processMappableMutation(
                        doc,
                        result,
                        "depth",
                        model.MIN_DEPTH,
                        model.MAX_DEPTH,
                        [](float depth) { model.setDepth(depth); }
                    );
                } else if (type == "set_stroke") {
                    processMappableMutation(
                        doc,
                        result,
                        "stroke",
                        model.MIN_DEPTH,
                        model.MAX_DEPTH,
                        [](float stroke) { model.setStroke(stroke); }
                    );
                } else if (type == "set_sensation") {
                    processMappableMutation(
                        doc,
                        result,
                        "sensation",
                        model.MIN_SENSATION,
                        model.MAX_SENSATION,
                        [](float stroke) { model.setStroke(stroke); }
                    );
                } else if (type == "set_pattern") {
                    if (!doc["pattern"].is<int>()) {
                        result["code"] = 400;
                        result["error"] = "invalid_pattern";
                        result["message"] = "\"pattern\" must be of integer type.";
                        return;
                    }

                    int pattern = doc["pattern"].as<int>();
                    model.setPattern(pattern);
                    result["type"] = "ack";
                } else if (type == "start_pattern") {
                    model.setMotionMode(Model::MotionMode::PATTERN);
                    result["type"] = "ack";
                } else if (type == "stop") {
                    model.setMotionMode(Model::MotionMode::STOPPED);
                    result["type"] = "ack";
                } else if (type == "get_speed") {
                    processMappableQuery(
                        doc,
                        result,
                        "speed",
                        model.getSpeed(),
                        model.MIN_DEPTH,
                        model.MAX_SPEED
                    );
                } else if (type == "get_depth") {
                    processMappableQuery(
                        doc,
                        result,
                        "depth",
                        model.getDepth(),
                        model.MIN_DEPTH,
                        model.MAX_DEPTH
                    );
                } else if (type == "get_stroke") {
                    processMappableQuery(
                        doc,
                        result,
                        "stroke",
                        model.getStroke(),
                        model.MIN_DEPTH,
                        model.MAX_DEPTH
                    );
                } else if (type == "get_sensation") {
                    processMappableQuery(
                        doc,
                        result,
                        "sensation",
                        model.getSensation(),
                        model.MIN_SENSATION,
                        model.MAX_SENSATION
                    );
                } else if (type == "get_pattern") {
                    auto pattern = model.getPattern();
                    result["type"] = "get_pattern";
                    result["pattern"] = pattern;
                } else if (type == "get_motion_mode") {
                    auto motionMode = model.getMotionMode();
                    result["type"] = "get_motion_mode";
                    switch (motionMode) {
                        case Model::MotionMode::STOPPED:
                            result["motion_mode"] = "STOPPED";
                            break;
                        case Model::MotionMode::PATTERN:
                            result["motion_mode"] = "PATTERN";
                            break;
                    }
                } else {
                    result["code"] = 400;
                    result["error"] = "unknown type";
                    result["message"] = "Unknown type \"" + type + "\".";
                }

                if (!result.containsKey("code")) {
                    result["code"] = 200;
                }
            }
        }
    }
}
