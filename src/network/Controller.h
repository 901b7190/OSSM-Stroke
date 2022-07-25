#pragma once

#include <exception>

#include <ArduinoJson.h>

namespace OSSMStroke {
    namespace Network {
        namespace Controller {
            void processMessage(const JsonVariant& doc, JsonDocument& result);
        }
    }
}
