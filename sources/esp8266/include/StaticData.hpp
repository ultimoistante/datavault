#ifndef _POWERMANAGER_STATICDATA_HPP
#define _POWERMANAGER_STATICDATA_HPP

struct activation_time_t
    {
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    unsigned char enabled;
    };

static struct activation_time_t activationTime;

static bool gotSaveAlarmTimeRequest = false;

static bool gotSetPowerRequest = false;
static bool gotSetPowerRequestStatus = false;

static bool websocketBusyWait = false;

#endif