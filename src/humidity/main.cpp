#include <Arduino.h>
#include "TinyWireS.h"
#include "DHT.h"

#include "shared_defs.h"

DHT dht(1, DHT22);

dht_rh rh_n;
temperature_union temp_n;

void request()
{
    for (int idx = 0; idx < 4; idx++)
    {
        TinyWireS.send(rh_n.rh_i[idx]);
    }

    for (int idx = 0; idx < 4; idx++)
    {
        TinyWireS.send(temp_n.temp_i[idx]);
    }
}

void setup()
{
    pinMode(1, INPUT);
    pinMode(4, OUTPUT);

    dht.begin();
    
    rh_n.rh = dht.readHumidity();
    temp_n.temp = dht.readTemperature();

    TinyWireS.begin(HUMIDITY_ADDRESS);
    TinyWireS.onRequest(request);
}

void loop()
{
    digitalWrite(4, 1);
    delay(1000);
    digitalWrite(4, 0);
    delay(1000);

    rh_n.rh = dht.readHumidity();
    temp_n.temp = dht.readTemperature();
}