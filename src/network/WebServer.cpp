#include <string>

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

#include "OSSM_Debug.h"
#include "Model.h"
#include "network/Controller.h"

namespace OSSMStroke {
    namespace Network {
        namespace WebServer {
            AsyncWebServer server(80);
            AsyncCallbackJsonWebHandler ctrlHandler(
                "/ctrl",
                [](AsyncWebServerRequest *request, JsonVariant &doc) {
                    AsyncResponseStream *response = request->beginResponseStream("application/json");
                    DynamicJsonDocument result(1024);
                    Controller::processMessage(doc, result);
                    serializeJson(result, *response);
                    request->send(response);
                }
            );
            AsyncWebSocket wsHandler("/ws");

            void wsOnEvent(
                AsyncWebSocket* server,
                AsyncWebSocketClient* client,
                AwsEventType type,
                void* arg,
                uint8_t* data,
                size_t len
            ) {
                if (type == WS_EVT_DATA){
                    if (!client) return;

                    DynamicJsonDocument doc(1024);
                    DynamicJsonDocument result(1024);
                    DeserializationError err = deserializeJson(doc, data, len);
                    if (err != DeserializationError::Ok) {
                        result["code"] = 400;
                        result["error"] = "invalid_json";
                        result["message"] = "Could not deserialize JSON payload.";
                        result["json_deserialization_error"] = err.c_str();
                    } else {
                        Controller::processMessage(doc, result);
                    }

                    size_t resultLength = measureJson(result);
                    AsyncWebSocketMessageBuffer * buffer = wsHandler.makeBuffer(resultLength);
                    if (buffer) {
                        serializeJson(result, (char *)buffer->get(), resultLength + 1);
                        client->text(buffer);
                    }
                }
            }

            void registerRoutes() {
                ctrlHandler.setMethod(HTTP_POST);
                server.addHandler(&ctrlHandler);

                wsHandler.onEvent(wsOnEvent);
                server.addHandler(&wsHandler);

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

            void loop() {
                wsHandler.cleanupClients();
            }
        }
    }
}
