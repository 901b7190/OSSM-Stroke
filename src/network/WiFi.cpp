#include <WiFiManager.h>

namespace OSSMStroke {
    namespace Network {
        namespace WiFi {
            WiFiManager wifiManager;

            void setup() {
                // reset settings - wipe credentials for testing
                // wifiManager.resetSettings();
                wifiManager.setConfigPortalBlocking(false);
                wifiManager.autoConnect();
            }

            void loop() {
                wifiManager.process();
            }
        }
    }
}
