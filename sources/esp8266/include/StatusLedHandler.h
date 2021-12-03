#ifndef _POWERMANAGER_STATUSLEDHANDLER_H
#define _POWERMANAGER_STATUSLEDHANDLER_H

#include <jled.h>

// -----------------------------------------------------------------------------

class StatusLedHandler
    {
    public:
        static const uint8_t DEVICE_STATUS_LED_MODE_NOT_CONNECTED;
        static const uint8_t DEVICE_STATUS_LED_MODE_TRYING_WPS;
        static const uint8_t DEVICE_STATUS_LED_MODE_CONNECTING;
        static const uint8_t DEVICE_STATUS_LED_MODE_CONNECTED;
        //
        static const uint8_t RPI_STATUS_POWER_OFF;
        static const uint8_t RPI_STATUS_BOOTING;
        static const uint8_t RPI_STATUS_BOOTED;
        static const uint8_t RPI_STATUS_SHUTTING_DOWN;
        //
        void begin(JLed* deviceStatusLed, JLed* rPiStatusLed);
        //
        void update();
        //
        void setDeviceStatus(uint8_t mode);
        void setRaspberryPiStatus(uint8_t mode);
        // void handleWebRequest();
        //
        // void update();
        // //
        // void setOnExpiredCallback(void (* onExpiredCallback)()) { this->onExpiredCallback = onExpiredCallback; }
        //
    private:
        JLed* deviceStatusLed;
        JLed* rPiStatusLed;
    };

// -----------------------------------------------------------------------------

#endif
