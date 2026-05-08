#include <Arduino.h>
#include <array>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "Wire.h"
#include "LittleFS.h"

#include "shared_defs.h"

#define LED_PIN 23
#define ANALOG_PIN 34

#define VBE 0.58f
#define slope 0.002f
#define Tref 21.0f

double temperature = 0.0f;

const char *ssid = "martabak telor";
const char *password = "11072008";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;
        if (strcmp((char *)data, "toggle") == 0)
        {
            ws.textAll(String(temperature));
        }
    }
}

void eventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);

        Serial.printf("");
        // digitalWrite(ledPin, ledState);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

String processor(const String &var)
{
    if (var == "STATE")
    {
        return String(temperature);
    }
    return String();
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);

    Wire.begin();
    //Wire.setClock(100000);
    Wire.setTimeout(5);

    if (!LittleFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting LittleFS");
        return;
    }

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Connected..!");
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());

    ws.onEvent(eventHandler);
    server.addHandler(&ws);

    server.serveStatic("/index.js", LittleFS, "/index.js");
    server.serveStatic("/graph.js", LittleFS, "/graph.js");
    server.serveStatic("/style.css", LittleFS, "/style.css");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html", false, processor); });

    server.begin();
}

void requestDHT()
{
    temperature_union dht_t;
    dht_rh dht_rhh;

    dht_t.temp = 0.0f;
    dht_rhh.rh = 0.0f;

    int idx = 0;

    Wire.requestFrom(HUMIDITY_ADDRESS, 8);

    while (Wire.available())
    {
        if (idx < 4)
        {
            dht_rhh.rh_i[idx] = Wire.read();
        }
        else
        {
            dht_t.temp_i[idx % 4] = Wire.read();
        }
        idx++;
    }

    Serial.printf("DHT = %.2f%%, %.2f°C\n", dht_rhh.rh, dht_t.temp);
}

void requestDS()
{
    static temperature_union temp_r;
    float temp_t = temp_r.temp;
    int idx = 0;

    Wire.requestFrom(TEMPERATURE_ADDRESS, 4);

    while (Wire.available())
    {
        temp_r.temp_i[idx] = Wire.read();
        idx++;
    }

    if (!Wire.getTimeout())
    {
        temp_r.temp = temp_t;
    }

    temperature = temp_r.temp;

    Serial.printf("DS = %.2f°C\n", temp_r.temp);    
}

const long interval = 200;
unsigned long previous = 0;

void loop()
{
    unsigned long current = millis();

    if (current - previous >= interval)
    {
        previous = current;

        requestDHT();
        requestDS();

        // Serial.printf("%f (%f)\n", temp_history, val_history);
    }

    ws.cleanupClients();
}
