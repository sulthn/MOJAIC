#include <Arduino.h>
#include <array>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "LittleFS.h"

#define LED 23
#define ANG 34

#define VBE 0.58f
#define slope 0.002f
#define Tref 21.0f

double val = 0.0f;
double val_history = 0.0f;
double temp_history = 0.0f;
double temp = 0.0f;
int poll = 0;

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
            ws.textAll(String(temp_history));
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
        return String(temp_history);
    }
    return String();
}

unsigned long previous = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(LED, OUTPUT);

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

const long interval = 50;

void loop()
{
    unsigned long current = millis();
    val = ((double)analogRead(ANG) / 4096.0f) * 3.30f + 0.11f;
    val_history += val;
    temp += (VBE - val) / slope + Tref;
    poll = poll + 1;

    if (current - previous >= interval)
    {
        previous = current;

        temp_history = temp / poll;
        val_history = val_history / poll;
        temp = 0.0f;
        poll = 0;

        //Serial.printf("%f (%f)\n", temp_history, val_history);
    }

    ws.cleanupClients();
}
