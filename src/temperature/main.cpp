#include <Arduino.h>
#include "TinyWireS.h"
#include "OneWire.h"
#include "DallasTemperature.h"

#include "shared_defs.h"

OneWire ds(1);
DallasTemperature sensors(&ds);

temperature_union temp_n;

void request()
{
    for (int idx = 0; idx < 4; idx++)
    {
        TinyWireS.send(temp_n.temp_i[idx]);
    }
}

void setup()
{
    pinMode(4, OUTPUT);

    sensors.begin();

    TinyWireS.begin(TEMPERATURE_ADDRESS);
    TinyWireS.onRequest(request);
}

uint8_t led_state = 0;

void loop()
{
    digitalWrite(4, led_state ^ 1);
    led_state = led_state ^ 1;

    delay(900);
    sensors.requestTemperatures();
    temp_n.temp = sensors.getTempCByIndex(0);
}