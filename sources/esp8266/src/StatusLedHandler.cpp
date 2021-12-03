#include "StatusLedHandler.h"

// -----------------------------------------------------------------------------

const uint8_t StatusLedHandler::DEVICE_STATUS_LED_MODE_NOT_CONNECTED = 0;
const uint8_t StatusLedHandler::DEVICE_STATUS_LED_MODE_TRYING_WPS = 1;
const uint8_t StatusLedHandler::DEVICE_STATUS_LED_MODE_CONNECTING = 2;
const uint8_t StatusLedHandler::DEVICE_STATUS_LED_MODE_CONNECTED = 3;

const uint8_t StatusLedHandler::RPI_STATUS_POWER_OFF = 10;
const uint8_t StatusLedHandler::RPI_STATUS_BOOTING = 11;
const uint8_t StatusLedHandler::RPI_STATUS_BOOTED = 12;
const uint8_t StatusLedHandler::RPI_STATUS_SHUTTING_DOWN = 13;

// -----------------------------------------------------------------------------

void StatusLedHandler::begin(JLed* deviceStatusLed, JLed* rPiStatusLed)
    {
    this->deviceStatusLed = deviceStatusLed;
    this->rPiStatusLed = rPiStatusLed;
    this->setDeviceStatus(DEVICE_STATUS_LED_MODE_NOT_CONNECTED);
    }


// -----------------------------------------------------------------------------

void StatusLedHandler::update()
    {
    this->deviceStatusLed->Update();
    this->rPiStatusLed->Update();
    }

// -----------------------------------------------------------------------------


void StatusLedHandler::setDeviceStatus(uint8_t mode)
    {
    switch (mode)
        {
            case StatusLedHandler::DEVICE_STATUS_LED_MODE_NOT_CONNECTED:
                this->deviceStatusLed->Blink(25, 250).Forever();
                break;
            case StatusLedHandler::DEVICE_STATUS_LED_MODE_TRYING_WPS:
                this->deviceStatusLed->Blink(25, 100).Forever();
                break;
            case StatusLedHandler::DEVICE_STATUS_LED_MODE_CONNECTED:
                this->deviceStatusLed->Breathe(2000).Forever();
                break;
        }
    }


// -----------------------------------------------------------------------------


void StatusLedHandler::setRaspberryPiStatus(uint8_t mode)
    {
    switch (mode)
        {
            case StatusLedHandler::RPI_STATUS_POWER_OFF:
                // this->rPiStatusLed->Off();
                this->rPiStatusLed->FadeOff(500).Repeat(1);
                break;
            case StatusLedHandler::RPI_STATUS_BOOTING:
                this->rPiStatusLed->Blink(25, 500).Forever();
                break;
            case StatusLedHandler::RPI_STATUS_BOOTED:
                this->rPiStatusLed->Breathe(2000).Forever();
                break;
            case StatusLedHandler::RPI_STATUS_SHUTTING_DOWN:
                this->rPiStatusLed->Blink(25, 250).Forever();
                break;
        }
    }


// -----------------------------------------------------------------------------
