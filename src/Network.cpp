#include <exception>
#include <string>

#include <esp_http_server.h>
#include <Stream.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <WiFi.h>

#include "OSSM_Debug.h"
#include "Stroker.h"

WiFiManager wifiManager;
httpd_handle_t server = NULL;

class NetworkError : public std::exception {
    public:
        NetworkError(const char* msg) : _msg(String(msg)) {}
        NetworkError(String msg) : _msg(msg) {}

        const char * what () const throw () {
            return (String("Network error: ") + _msg).c_str();
        }

    private:
        String _msg;
};

struct HttpdReqReader {
    public:
        HttpdReqReader(httpd_req_t* req) : _req(req), _remaining(req->content_len) {}

        int read() {
            char c;
            return readBytes(&c, 1) ? static_cast<unsigned char>(c) : -1;
        }

        size_t readBytes(char* buffer, size_t length) {
            int ret;

            if ((ret = httpd_req_recv(_req, buffer, length)) <= 0) {
                return ret;
            }

            _remaining -= ret;
            return ret;
        }

    private:
        httpd_req_t* _req;
        size_t _remaining;
};

esp_err_t writeHttpBadRequest(httpd_req_t *req, const char* err) {
    httpd_resp_set_status(req, HTTPD_400);
    httpd_resp_send_chunk(req, "{\"error\": \"", HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, err, HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, "\"}", HTTPD_RESP_USE_STRLEN);
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t postControlHandler(httpd_req_t *req) {
    httpd_resp_set_type(req, "application/json");

    LogDebug("Received POST /ctrl.");
    HttpdReqReader stream = HttpdReqReader(req);

    DynamicJsonDocument doc(1024);
    DeserializationError deserializationErr = deserializeJson(doc, stream);
    if (deserializationErr.code() != DeserializationError::Ok) {
        LogDebugFormatted("Failed to deserialize input: %s.\n", deserializationErr.c_str());
        return writeHttpBadRequest(req, "deserialization_error");
    }

    if (!doc["speed"].is<float_t>()) {
        return writeHttpBadRequest(req, "invalid_speed");
        return ESP_OK;
    }

    // FIXME hardcoded safeguard. We need model.
    float_t speed = MIN(300.f, doc["speed"].as<float_t>());
    LogDebugFormatted("Speed: %f\n", speed);
    Stroker.setSpeed(MIN(300.f, speed), true);

    httpd_resp_send(req, "{\"result\": \"ok\"}", 0);
    return ESP_OK;
}

void startWebserver() {
    if (server != NULL) {
        LogDebug("Webserver already started.");

    }
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    LogDebugFormatted("Starting webserver on port: '%d'.", config.server_port);
    if (esp_err_t err = httpd_start(&server, &config) != ESP_OK) {
        LogDebugFormatted("Error starting webserver: %s.", esp_err_to_name(err));
        return;
    }

    LogDebug("Registering weberver's URI handlers.");

    const httpd_uri_t postCtrlUri = {
        .uri       = "/ctrl",
        .method    = HTTP_POST,
        .handler   = postControlHandler,
        .user_ctx  = NULL
    };
    if (esp_err_t err = httpd_register_uri_handler(server, &postCtrlUri) != ESP_OK) {
        LogDebugFormatted("Error starting webserver: %s.", esp_err_to_name(err));
        return;
    }
}

void stopWebserver() {
    if (server == NULL) {
        LogDebug("Webserver already stopped.");
        return;
    }

    LogDebug("Stopping webserver");
    if (esp_err_t err = httpd_stop(&server) != ESP_OK) {
        LogDebugFormatted("Failed to stop webserver: %s.", esp_err_to_name(err));
        return;
    }

    server = NULL;
}

void disconnectHandler(
    void* arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data
) {
    stopWebserver();
}

void connectHandler(
    void* arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data
) {
    startWebserver();
}


void networkSetup() {
    /////////////////
    // Wi-Fi stuff //
    /////////////////
    // reset settings - wipe credentials for testing
    // wifiManager.resetSettings();
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.autoConnect();

    //////////////////////
    // Web Server stuff //
    //////////////////////

    // FIXME not working right now.
    // if (
    //     esp_err_t err = esp_event_handler_register(
    //         IP_EVENT,
    //         IP_EVENT_STA_GOT_IP,
    //         &connectHandler,
    //         NULL
    //     ) != ESP_OK
    // ) {
    //     LogDebugFormatted(
    //         "Failed to register server connection handler: %s.",
    //         esp_err_to_name(err),
    //     );
    // } else if (
    //     esp_err_t err = esp_event_handler_register(
    //         WIFI_EVENT,
    //         WIFI_EVENT_STA_DISCONNECTED,
    //         &disconnectHandler,
    //         NULL
    //     ) != ESP_OK
    // ) {
    //     LogDebugFormatted(
    //         "Failed to register server disconnection handler: %s.",
    //         esp_err_to_name(err),
    //     );
    // }
    if (server == NULL && WiFi.status() == WL_CONNECTED) {
        startWebserver();
    }
}

void networkLoop() {
    /////////////////
    // Wi-Fi stuff //
    /////////////////
    wifiManager.process();

    // FIXME put this in setup when events handlers are usable
    if (server == NULL && WiFi.status() == WL_CONNECTED) {
        startWebserver();
    }
}
