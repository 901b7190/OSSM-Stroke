#include <mdns.h>
#include <WiFiManager.h>

#include "OSSM_Debug.h"

namespace OSSMStroke {
    namespace Network {
        namespace WiFi {
            WiFiManager wifiManager;

            void setupMdns() {
                esp_err_t err = mdns_init();
                if (err) {
                    LogDebug("Failed to initialize MDNS service.");
                    return;
                }

                mdns_hostname_set(OSSM_MDNS_HOSTNAME);
                mdns_service_add(NULL, "_http", "_tcp", OSSM_HTTP_SERVER_PORT, NULL, 0);
                mdns_service_instance_name_set("_http", "_tcp", OSSM_MDNS_HTTP_INSTANCE_NAME);
            }

            void setup() {
                // reset settings - wipe credentials for testing
                // wifiManager.resetSettings();
                wifiManager.setConfigPortalBlocking(false);
                wifiManager.autoConnect();

                setupMdns();
            }

            void loop() {
                wifiManager.process();
            }
        }
    }
}
