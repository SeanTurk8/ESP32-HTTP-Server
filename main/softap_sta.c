#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "esp_mac.h"

#define EXAMPLE_AP_SSID       "ESP32-AP"
#define EXAMPLE_AP_PASS       "" // set your desired password, or leave empty for open AP
#define EXAMPLE_AP_CHANNEL    1
#define EXAMPLE_MAX_STA_CONN  4
#define EXAMPLE_STA_SSID      "" // set your router SSID
#define EXAMPLE_STA_PASS      "" // set your router password

static const char *TAG = "WiFi_APSTA";

// Wi-Fi event handler
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                esp_wifi_connect();
                ESP_LOGI(TAG, "STA started, trying to connect...");
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "STA connected to router");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGW(TAG, "STA disconnected, retrying...");
                esp_wifi_connect();
                break;
            case WIFI_EVENT_AP_STACONNECTED: {
                wifi_event_ap_staconnected_t *info = event_data;
                ESP_LOGI(TAG, "Device " MACSTR " joined SoftAP, AID=%d", MAC2STR(info->mac), info->aid);
                break;
            }
            case WIFI_EVENT_AP_STADISCONNECTED: {
                wifi_event_ap_stadisconnected_t *info = event_data;
                ESP_LOGI(TAG, "Device " MACSTR " left SoftAP, AID=%d", MAC2STR(info->mac), info->aid);
                break;
            }
            default:
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "STA got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

// Set up Wi-Fi Station mode (connects to existing router)
static void wifi_init_sta(void) {
    wifi_config_t sta_config = {
        .sta = {
            .ssid = EXAMPLE_STA_SSID,
            .password = EXAMPLE_STA_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_LOGI(TAG, "Station config set (SSID: %s)", EXAMPLE_STA_SSID);
}

// Set up Wi-Fi Access Point (ESP32 creates its own network)
static void wifi_init_softap(void) {
    wifi_config_t ap_config = {
        .ap = {
            .ssid = EXAMPLE_AP_SSID,
            .ssid_len = strlen(EXAMPLE_AP_SSID),
            .channel = EXAMPLE_AP_CHANNEL,
            .password = EXAMPLE_AP_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .required = true,
            },
        }
    };

    if (strlen(EXAMPLE_AP_PASS) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_LOGI(TAG, "SoftAP config set (SSID: %s)", EXAMPLE_AP_SSID);
}

// Initialize Wi-Fi in AP + STA mode
static void wifi_init_apsta(void) {
    esp_err_t ret = nvs_flash_init(); // Initialize non-volatile storage (NVS)
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) { 
        ESP_ERROR_CHECK(nvs_flash_erase()); // Erase NVS if it's full or incompatible
        ESP_ERROR_CHECK(nvs_flash_init());  // Re-initialize NVS after erase
    }

    ESP_ERROR_CHECK(esp_netif_init()); // Initialize TCP/IP network stack
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Create default event loop for Wi-Fi/IP events

    esp_netif_create_default_wifi_ap();  // Create default network interface for Access Point
    esp_netif_create_default_wifi_sta(); // Create default network interface for Station

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // Load default Wi-Fi driver configuration
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // Initialize Wi-Fi driver with config

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));  // Register event handler for all Wi-Fi events
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL)); // Register event handler for "got IP" event on Station interface

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA)); // dual mode

    wifi_init_softap();
    wifi_init_sta();

    ESP_ERROR_CHECK(esp_wifi_start()); // Start Wi-Fi driver

    ESP_LOGI(TAG, "Wi-Fi started in AP+STA mode");
    ESP_LOGI(TAG, "SoftAP IP: 192.168.4.1. Visit http://192.168.4.1/ to test."); 
}

// Simple GET request handler for "/"
static esp_err_t hello_get_handler(httpd_req_t *req) {
    const char *resp = "<!DOCTYPE html>"
"<html>"
"<head>"
"<title>Sean Turk | Embedded Portfolio</title>"
"</head>"
"<body style='font-family:Arial;text-align:center;background-color:#f4f4f4;'>"
"<h1 style='color:#333;'>Welcome to My Portfolio</h1>"
"<p>This page is served by the ESP32 in Access Point + Station Mode.</p>"
"<hr>"
"<h2> About Me</h2>"
"<p>Name: <strong>Sean G. Turk</strong></p>"
"<p>Degree: <strong>Computer Engineering, UMass Dartmouth</strong></p>"
"<p>Focus: <strong>Embedded Systems, Software, Firmware, and Microcontrollers</strong></p>"
"<hr>"
"<h2> Featured Projects</h2>"
"<ul style='list-style:none;'>"
"<li> Checkers Robot Arm with I2C + Stepper Motors</li>"
"<li> STM32 Data Logger w/ Flash Storage + UART Debug</li>"
"<li> STM32 ADS1246 Thermistor Reader </li>"
"<li> Line Follower Robot with IR + Ultrasonic Sensors</li>"
"<li> ESP32 HTTP Server </li>"
"</ul>"
"<hr>"
"<h2> Contact</h2>"
"<p><a href='https://github.com/SeanTurk8' target='_blank'>GitHub: github.com/SeanTurk8</a></p>"
"<p>Email: <a href='mailto:sean_turk@outlook.com'>sean_turk@outlook.com</a></p>"
"</body>"
"</html>";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Start the web server
static httpd_handle_t start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t hello_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = hello_get_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &hello_uri);
    }

    return server;
}

// Main application entry point
void app_main(void) {
    wifi_init_apsta();         // Start Wi-Fi in AP + STA mode
    start_webserver();         // Launch web server

    ESP_LOGI(TAG, "Setup complete. Connect to ESP32-AP or check STA IP to access the web server.");
}