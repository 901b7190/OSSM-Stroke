#include "network/WiFi.h"
#include "network/WebServer.h"

namespace OSSMStroke {
    namespace Network {
        void setup() {
            WiFi::setup();
            WebServer::setup();
        }

        void loop() {
            WiFi::loop();
            WebServer::loop();
        }
    }
}
