#ifndef __MOJAIC_SHARED_DEFS_H__
#define __MOJAIC_SHARED_DEFS_H__

#define HUMIDITY_ADDRESS    0x22
#define TEMPERATURE_ADDRESS 0x42

union dht_rh
{
    float rh;
    char rh_i[4];
};

union temperature_union
{
    float temp;
    char temp_i[4];
};

#endif