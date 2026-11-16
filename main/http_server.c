#include "http_server.h"
#include "esp_http_server.h"
#include "relay.h"
#include "automation.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------- */
/* Embedded web UI (no SPIFFS needed - served directly from flash/RAM) */
/* ------------------------------------------------------------------- */
static const char *INDEX_HTML =
"<!DOCTYPE html><html><head><meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width, initial-scale=1'>"
"<title>ESP32 Smart Home</title>"
"<style>"
"body{font-family:Arial,sans-serif;background:#1e1e2f;color:#eee;text-align:center;padding:20px;margin:0;}"
"h1{color:#4fc3f7;margin-bottom:4px;}"
".sub{color:#888;font-size:13px;margin-top:0;}"
".card{background:#2a2a40;border-radius:12px;padding:16px;margin:14px auto;max-width:380px;box-shadow:0 4px 10px rgba(0,0,0,0.4);}"
".row{display:flex;justify-content:space-between;padding:4px 8px;}"
".btn{padding:10px 18px;margin:5px;border:none;border-radius:8px;font-size:15px;cursor:pointer;}"
".on{background:#4caf50;color:white;}"
".off{background:#e53935;color:white;}"
".mode{background:#4fc3f7;color:#111;}"
".val{font-weight:bold;color:#ffca28;}"
".state{font-weight:bold;}"
"</style></head><body>"
"<h1>ESP32 Smart Home</h1>"
"<p class='sub'>DHT11 + LDR + HC-SR04 &middot; Wi-Fi / HTTP</p>"

"<div class='card'>"
"<div class='row'><span>Temperature</span><span class='val' id='temp'>--</span></div>"
"<div class='row'><span>Humidity</span><span class='val' id='hum'>--</span></div>"
"<div class='row'><span>Light Level</span><span class='val' id='light'>--</span></div>"
"<div class='row'><span>Distance</span><span class='val' id='dist'>--</span></div>"
"</div>"

"<div class='card'>"
"<h3>Light</h3>"
"<button class='btn on' onclick=\"send('/light/on')\">ON</button>"
"<button class='btn off' onclick=\"send('/light/off')\">OFF</button>"
"<p>State: <span class='state' id='lightState'>--</span></p>"
"</div>"

"<div class='card'>"
"<h3>Fan</h3>"
"<button class='btn on' onclick=\"send('/fan/on')\">ON</button>"
"<button class='btn off' onclick=\"send('/fan/off')\">OFF</button>"
"<p>State: <span class='state' id='fanState'>--</span></p>"
"</div>"

"<div class='card'>"
"<h3>Mode</h3>"
"<button class='btn mode' onclick=\"send('/mode/auto')\">AUTOMATIC</button>"
"<button class='btn mode' onclick=\"send('/mode/manual')\">MANUAL</button>"
"<p>Current: <span class='state' id='mode'>--</span></p>"
"</div>"

"<script>"
"function send(url){fetch(url).then(update);}"
"function update(){"
"fetch('/status').then(function(r){return r.json();}).then(function(d){"
"document.getElementById('temp').innerText=d.temperature.toFixed(1)+' \\u00B0C';"
"document.getElementById('hum').innerText=d.humidity.toFixed(0)+' %';"
"document.getElementById('light').innerText=d.light;"
"document.getElementById('dist').innerText=(d.distance>=0?d.distance.toFixed(1)+' cm':'n/a');"
"document.getElementById('lightState').innerText=d.light_state?'ON':'OFF';"
"document.getElementById('fanState').innerText=d.fan_state?'ON':'OFF';"
"document.getElementById('mode').innerText=d.auto_mode?'AUTOMATIC':'MANUAL';"
"});}"
"setInterval(update,2000);"
"window.onload=update;"
"</script>"
"</body></html>";

/* ------------------------------------------------------------------- */
/* HTTP handlers                                                       */
/* ------------------------------------------------------------------- */
static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t status_handler(httpd_req_t *req)
{
    char buf[220];
    snprintf(buf, sizeof(buf),
             "{\"temperature\":%.1f,\"humidity\":%.1f,\"light\":%d,"
             "\"distance\":%.1f,\"light_state\":%s,\"fan_state\":%s,"
             "\"auto_mode\":%s}",
             g_system_status.temperature,
             g_system_status.humidity,
             g_system_status.light_level,
             g_system_status.distance,
             g_system_status.light_state ? "true" : "false",
             g_system_status.fan_state ? "true" : "false",
             g_system_status.auto_mode ? "true" : "false");

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t light_on_handler(httpd_req_t *req)
{
    automation_set_mode(false);
    relay_set_light(true);
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t light_off_handler(httpd_req_t *req)
{
    automation_set_mode(false);
    relay_set_light(false);
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t fan_on_handler(httpd_req_t *req)
{
    automation_set_mode(false);
    relay_set_fan(true);
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t fan_off_handler(httpd_req_t *req)
{
    automation_set_mode(false);
    relay_set_fan(false);
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t mode_auto_handler(httpd_req_t *req)
{
    automation_set_mode(true);
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

static esp_err_t mode_manual_handler(httpd_req_t *req)
{
    automation_set_mode(false);
    return httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
}

/* ------------------------------------------------------------------- */
/* Server bring-up                                                     */
/* ------------------------------------------------------------------- */
void http_server_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) != ESP_OK) {
        return;
    }

    static const httpd_uri_t uris[] = {
        { .uri = "/",             .method = HTTP_GET, .handler = index_handler },
        { .uri = "/status",       .method = HTTP_GET, .handler = status_handler },
        { .uri = "/light/on",     .method = HTTP_GET, .handler = light_on_handler },
        { .uri = "/light/off",    .method = HTTP_GET, .handler = light_off_handler },
        { .uri = "/fan/on",       .method = HTTP_GET, .handler = fan_on_handler },
        { .uri = "/fan/off",      .method = HTTP_GET, .handler = fan_off_handler },
        { .uri = "/mode/auto",    .method = HTTP_GET, .handler = mode_auto_handler },
        { .uri = "/mode/manual",  .method = HTTP_GET, .handler = mode_manual_handler },
    };

    for (size_t i = 0; i < sizeof(uris) / sizeof(uris[0]); i++) {
        httpd_register_uri_handler(server, &uris[i]);
    }
}
