#include <string>

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

#include "OSSM_Debug.h"
#include "Model.h"

namespace OSSMStroke {
    namespace Network {
        namespace WebServer {
            AsyncWebServer server(80);
            AsyncCallbackJsonWebHandler ctrlHandler(
                "/ctrl",
                [](AsyncWebServerRequest *request, JsonVariant &doc) {
                    if (!doc.is<JsonObject>()) {
                        request->send(400, "application/json", R"({"error": "not_an_object"})");
                        return;
                    }

                    if (!doc["speed"].is<float_t>()) {
                        request->send(400, "application/json", R"({"error": "invalid_speed"})");
                        return;
                    }

                    model.setSpeed(doc["speed"].as<float_t>());
                    request->send(200, "application/json", R"({"result": "ok"})");
                }
            );

            void registerRoutes() {
                ctrlHandler.setMethod(HTTP_POST);
                server.addHandler(&ctrlHandler);

                server.onNotFound([](AsyncWebServerRequest *request) {
                    request->send(404);
                });
            }
            void startWebserver() { server.begin(); }
            void stopWebserver() { server.end(); }

            void setup() {
                registerRoutes();

                auto startWebserverHandler = [](
                    void* arg,
                    esp_event_base_t event_base,
                    int32_t event_id,
                    void* event_data
                ) { startWebserver(); };
                auto stopWebserverHandler = [](
                    void* arg,
                    esp_event_base_t event_base,
                    int32_t event_id,
                    void* event_data
                ) { stopWebserver(); };
                if (
                    esp_err_t err = esp_event_handler_register(
                        IP_EVENT,
                        IP_EVENT_STA_GOT_IP,
                        startWebserverHandler,
                        NULL
                    ) != ESP_OK
                ) {
                    LogDebugFormatted(
                        "Failed to register server connection handler: %s.",
                        esp_err_to_name(err)
                    );
                } else if (
                    esp_err_t err = esp_event_handler_register(
                        WIFI_EVENT,
                        WIFI_EVENT_STA_DISCONNECTED,
                        stopWebserverHandler,
                        NULL
                    ) != ESP_OK
                ) {
                    LogDebugFormatted(
                        "Failed to register server disconnection handler: %s.",
                        esp_err_to_name(err)
                    );
                }
                if (::WiFi.status() == WL_CONNECTED) {
                    startWebserver();
                }
            }

            void loop() {}
        }
    }
}
