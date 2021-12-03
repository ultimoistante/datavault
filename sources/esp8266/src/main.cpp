
/*
 * DataVault esp8266-power-manager firmware
 *
 * Written by Salvatore Carotenuto (ultimoistante)
 *
 * Changelog:
 *    2021-06-12:
 *       .handled SBC power on/off from web app;
 *       .handled timer;
 *       .added notyf (css toast notifications);
 *       .added activationTime saving from webapp;
 *       .rewritten webapp with chota css;
 *       .fixed websocket status request;
 *    2021-06-06:
 *       .added ESP8266TimeAlarms library
 *       .added EEPROM handling
 *       .added timeAlarm handling
 *    2021-06-05:
 *       .fixed exception on webservice start
 *       .forked esp8266 project code from old datavault repo
 *    2021-05-17:
 *       .rebooted project under PlatformIO
 *       .started adding Vue/websocket based web app
 *       .mDNS service now works
 *       .readded NTP client code
 *    2021-01-26:
 *       .added handling of raspberryPi status led to StatusLedHandler
 *       .started adding logging and receive methods to SerialHandler
 *    2020-12-30:
 *       .rewritten WiFi connection code in event-driven mode
 *       .replaced NTP client code, adding from: https://github.com/gmag11/ESPNtpClient
 *    2020-12-27:
 *       .changed pin assignments
 *       .added status led patterns
 *       .added wifi connection
 *       .added basic webserver
 *       .started adding mDNS support
 *    2020-12-16:
 *       .added jLed library
 *       .started writing
*/

// TODO: fix mDNS: https://tttapa.github.io/ESP8266/Chap08%20-%20mDNS.html
// TODO: add wifi reconnection 

// ----------------------------------------------------------------------------
// Hardware connections:
//
//    +-------------------+
//    |A0          D0 [16]| - rPi power status LED
//    |            D1 [05]| - SoftwareSerial RX (to RaspberryPi serial TX)
//    |            D2 [04]| - SoftwareSerial TX (to RaspberryPi serial RX)
//    |            D3 [00]| - __RESERVED__
//    |            D4 [02]| - (LED_BUILTIN)
//    |                3v3| -
//    |                GND| -
//    |            D5 [14]| - Power button
//    |            D6 [12]| - WPS request button
//    |            D7 [13]| - Power Manager (this device) status LED
//    |            D8 [15]| - MOSFET module on/off
//    |       D9 / RX [03]| - 
//    |      D10 / TX [01]| - to RaspberryPi serial RX
//    |                GND| -
//    |                3v3| -
//    +-------------------+
//
//    +----+----+----+----+-----+
//    | D1 | D2 | RX | TX | GND |
//    +-__-+----+----+----+--__-+
//
// ----------------------------------------------------------------------------

#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h> // Include the mDNS library
#include <FS.h>
#include <jled.h>
#include <AceButton.h>
using namespace ace_button;
#include <ESPNtpClient.h>           // https://github.com/gmag11/ESPNtpClient
#include <EEPROM.h>
#include <ESP8266TimeAlarms.h>      // https://github.com/jhagberg/ESP8266TimeAlarms
#include <SoftwareSerial.h>
//
#include "defines.h"
#include "SerialHandler.h"
#include "WiFiHandler.h"
#include "WebService.hpp"
#include "StatusLedHandler.h"
#include "StaticData.hpp"

// ----------------------------------------------------------------------------

// user buttons
AceButton powerButton(POWER_BUTTON_PIN);
AceButton wpsButton(WPS_BUTTON_PIN);

// status leds
auto deviceStatusLed = JLed(DEVICE_STATUS_LED_PIN);
auto rpiStatusLed = JLed(RPI_STATUS_LED_PIN);

bool rpiPowered = false;
uint8_t rpiPoweredBy = SBC_POWERED_BY_NONE;

// ----------------------------------------------------------------------------

// SoftwareSerial swSerial;
SoftwareSerial swSerial;
SerialHandler serialHandler;

// handler for wifi connection
WiFiHandler wifiHandler;

StatusLedHandler statusLedHandler;

NTPEvent_t ntpEvent;            // Last triggered event
bool ntpEventTriggered = false; // True if a time even has been triggered
bool ntpSynchronized = false;

char serialResponseBuffer[64];

// ----------------------------------------------------------------------------

#define MDNS_UPDATE_TIME_INTERVAL 2000
bool mDNSStarted = false;
unsigned long nextMDNSUpdateTime = 0;

unsigned long webServerActivationTime = 0;

unsigned long timeAlarmInstanceTime = 0;
unsigned long nextAlarmsServiceTime = 0;

unsigned long powerOffTime = 0;

// ----------------------------------------------------------------------------

// millisecond timer object
os_timer_t msecTimer;
uint8_t timerTickCounter = 0;

// ----------------------------------------------------------------------------

AlarmId activationTimerAlarmId = 0;

// ----------------------------------------------------------------------------


void msecTimerCallback(void* pArg)
    {
    statusLedHandler.update();
    //
    powerButton.check();
    wpsButton.check();
    }


// -----------------------------------------------------------------------------


void setMosfetStatus(bool status)
    {
    digitalWrite(MOSFET_CONTROL_PIN, status ? HIGH : LOW);
    }


// ----------------------------------------------------------------------------


void onConnectCallback()
    {
    // Serial.println(">>> onConnectCallback");
    //
    // starts the mDNS responder for <devicename>.local
    if (!MDNS.begin(DEVICE_MDNS_HOSTNAME))
        {
        Serial.println("[E] Error setting up MDNS responder!");
        }
    else
        {
        MDNS.addService("http", "tcp", 80);
        Serial.println("[I] MDNS responder started");
        mDNSStarted = true;
        }
    //
    // defers webserver activation time, at 1 second from now
    webServerActivationTime = millis() + 1000;
    //
    // starts NTP service
    NTP.setTimeZone(TZ_Europe_Rome);
    NTP.onNTPSyncEvent([](NTPEvent_t event)
        {
        ntpEvent = event;
        ntpEventTriggered = true;
        });
    NTP.begin();
    }


// -----------------------------------------------------------------------------


void processNtpEvent(NTPEvent_t ntpEvent)
    {
    switch (ntpEvent.event)
        {
            case timeSyncd:
            case partlySync:
            case syncNotNeeded:
            case accuracyError:
                Serial.printf("[I] [NTP-event] %s\n", NTP.ntpEvent2str(ntpEvent));
                ntpSynchronized = true;
                //
                // defers timeAlarm instance to 0.5 second from now
                timeAlarmInstanceTime = millis() + 500;
                //
                break;
            default:
                break;
        }
    }


// -----------------------------------------------------------------------------


void handleButtonEvent(AceButton* button, uint8_t eventType, uint8_t /*buttonState*/)
    {
    uint8_t pin = button->getPin();
    //
    if (pin == POWER_BUTTON_PIN)
        {
        switch (eventType)
            {
            // Interpret a Released event as a Pressed event, to distiguish it
            // from a LongPressed event.
                case AceButton::kEventReleased:
                    if (!rpiPowered)
                        {
                        setMosfetStatus(true);
                        rpiPowered = true;
                        rpiPoweredBy = SBC_POWERED_BY_USER;
                        statusLedHandler.setRaspberryPiStatus(StatusLedHandler::RPI_STATUS_BOOTING);
                        }
                    break;

                    // LongPressed goes in and out of edit mode.
                case AceButton::kEventLongPressed:
                    if (rpiPowered)
                        {
                        setMosfetStatus(false);
                        rpiPowered = false;
                        rpiPoweredBy = SBC_POWERED_BY_NONE;
                        rpiStatusLed.FadeOff(500).Repeat(1);
                        }
                    break;
            }
        }
    //
    else if (pin == WPS_BUTTON_PIN && eventType == AceButton::kEventReleased)
        {
        if (wifiHandler.getCurrentStatus() == WiFiHandler::STATUS_STAMODE_NOT_CONNECTED)
            {
            wifiHandler.tryWPSConnection();
            }
        }
    }


// -----------------------------------------------------------------------------


void serialReceiveMessageHandler(char** tokens, int tokenCount)
    {
    // unsigned char config_changed = 0;
    // String msg = "";
    //
    // prints the extracted parameters
    // Serial.printf("[serialReceiveMessageHandler] Token count: %d\n", tokenCount);
    // for (int i = 0; i < tokenCount; i++)
    //     Serial.printf("   tokens[%d]: '%s'\n", i, tokens[i]);
    //
    if (tokenCount)
        {
        //
        // powerBy? request
        if (strcmp(tokens[0], "powerBy?") == 0)
            {
            sprintf(serialResponseBuffer, "powerBy=%s", (rpiPoweredBy == SBC_POWERED_BY_TIMER) ? "timer" : ((rpiPoweredBy == SBC_POWERED_BY_USER) ? "user" : "none"));
            serialHandler.sendMessage(serialResponseBuffer);
            // swSerial.println(serialResponseBuffer);
            }
        //
        // bootComplete! event
        else if (strcmp(tokens[0], "bootComplete!") == 0)
            {
            statusLedHandler.setRaspberryPiStatus(StatusLedHandler::RPI_STATUS_BOOTED);
            // sprintf(serialResponseBuffer, "{powerBy=%s}", (rpiPoweredBy == SBC_POWERED_BY_TIMER) ? "timer" : ((rpiPoweredBy == SBC_POWERED_BY_USER) ? "user" : "none"));
            // swSerial.println(serialResponseBuffer);
            }
        //
        // powerOff request
        if (tokenCount >= 2 && strcmp(tokens[0], "powerOff") == 0)
            {
            unsigned int delay = atoi(tokens[1]);
            if (delay > 0)
                {
                Serial.printf("[I] powerOff request received. Shutting down RaspberryPi in %d seconds\n", delay);
                statusLedHandler.setRaspberryPiStatus(StatusLedHandler::RPI_STATUS_SHUTTING_DOWN);
                powerOffTime = millis() + (delay * 1000);
                }
            }
        }
    }


// -----------------------------------------------------------------------------


void eepromWriteSignature()
    {
    // writes default signature: "DVCTRL10" (DataVault Controller 1.0)
    EEPROM.write(0, 'D');
    EEPROM.write(1, 'V');
    EEPROM.write(2, 'C');
    EEPROM.write(3, 'T');
    EEPROM.write(4, 'R');
    EEPROM.write(5, 'L');
    EEPROM.write(6, '1');
    EEPROM.write(7, '0');
    EEPROM.commit(); // stores data to EEPROM
    }


// -----------------------------------------------------------------------------


void eepromWriteActivationTime()
    {
    EEPROM.put(EEPROM_DATA_START_ADDR, activationTime);
    EEPROM.commit(); // stores data to EEPROM
    }


// -----------------------------------------------------------------------------


void eepromInit()
    {
    uint16_t start_addr;
    // expected signature: "DVCTRL10" (DataVault Controller 1.0)
    char signature[8];
    for (byte i = 0; i < 8; i++)
        signature[i] = EEPROM.read(i);
    if (memcmp(signature, "DVCTRL10", 8) == 0)
        {
        // good signature, reads structures from eeprom
        start_addr = 8; // data start address is after signature
        EEPROM.get(start_addr, activationTime);
        Serial.println("[I] found valid eeprom signature");
        }
    else
        {
        // uninitialized eeprom, or bad signature
        Serial.print("[W] no valid eeprom signature found. Initializing... ");
        // stores default values
        activationTime.hour = 0;
        activationTime.minute = 0;
        activationTime.second = 0;
        activationTime.enabled = 0;
        //
        eepromWriteSignature();
        eepromWriteActivationTime();
        Serial.println("done.");
        }
    }


// -----------------------------------------------------------------------------


void onActivationTimerAlarm()
    {
    Serial.printf("[I] timerAlarm triggered at %s\n", NTP.getTimeDateString(time(NULL), "%04Y-%02m-%02d %02H:%02M:%02S"));
    //
    websocket.textAll("{\"event\":\"timerTriggered\"}");
    //
    if (!rpiPowered)
        {
        Serial.println("[I] activated SBC power");
        setMosfetStatus(true);
        rpiPoweredBy = SBC_POWERED_BY_TIMER;
        statusLedHandler.setRaspberryPiStatus(StatusLedHandler::RPI_STATUS_BOOTING);
        rpiPowered = true;
        // TODO: also put a pin HIGH to signal activation by timer
        }
    else
        Serial.println("[I] SBC power already active, skipping");
    }


// -----------------------------------------------------------------------------


void setup()
    {
    //
    // sets button pins as input, with pullup
    pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
    pinMode(WPS_BUTTON_PIN, INPUT_PULLUP);
    // sets mosfet control pin mode
    pinMode(MOSFET_CONTROL_PIN, OUTPUT);
    //
    setMosfetStatus(false);
    //
    Serial.begin(HW_SERIAL_BAUDRATE);
    //
    swSerial.begin(SW_SERIAL_BAUDRATE, SWSERIAL_8N1, SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);
    serialHandler.begin(&swSerial);
    serialHandler.setPacketDelimiterChars('{', '}');
    serialHandler.setTokenCharSeparators("|,");
    serialHandler.setMessageHandler(serialReceiveMessageHandler);
    // serialHandler.begin(SERIAL_BAUDRATE);
    // serialHandler.setRxTokenCharSeparators("|,");
    // serialHandler.setMessageHandler(serialReceiveMessageHandler);
    // delay(250);
    // serialHandler.logInfo("Ultimoistante's DataVault Power Manager");
    //
    SPIFFS.begin();
    //
    EEPROM.begin(512);
    delay(500);
    eepromInit();
    //
    // starts millisecond timer
    os_timer_setfn(&msecTimer, msecTimerCallback, NULL);
    os_timer_arm(&msecTimer, 1, true);

    // Configure the ButtonConfig with the event handler.
    ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
    buttonConfig->setEventHandler(handleButtonEvent);
    buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
    buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);
    buttonConfig->setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
    //
    statusLedHandler.begin(&deviceStatusLed, &rpiStatusLed);
    //
    delay(500);
    wifiHandler.setOnConnectCallback(onConnectCallback);
    // wifiHandler.begin(&statusLedHandler, &serialHandler);
    wifiHandler.begin(&statusLedHandler);
    delay(1000);
    }


// ----------------------------------------------------------------------------


void loop()
    {
    unsigned long now = millis();
    //
    serialHandler.doReceive();
    /*while (swSerial.available())
        {
        char inByte = swSerial.read();
        // DEBUG
        Serial.printf("r: %c\n", inByte);
        }*/
        //
    //
    // checks if it's time to start webserver
    if (webServerActivationTime && now >= webServerActivationTime)
        {
        webServerActivationTime = 0;
        initWebService(&server, &websocket);
        }
    //
    // checks if it's time to instance defined alarmTime
    if (timeAlarmInstanceTime && now >= timeAlarmInstanceTime)
        {
        timeAlarmInstanceTime = 0;
        //
        // disable/free previously created timer, if any
        if (activationTimerAlarmId)
            {
            Alarm.disable(activationTimerAlarmId);
            Alarm.free(activationTimerAlarmId);
            }
        //
        if (activationTime.enabled)
            {
            Serial.printf("[I] setting timerAlarm at %02d:%02d:%02d\n", activationTime.hour, activationTime.minute, activationTime.second);
            activationTimerAlarmId = Alarm.alarmRepeat(activationTime.hour, activationTime.minute, activationTime.second, onActivationTimerAlarm);
            }
        else
            {
            Serial.println("[W] timerAlarm disabled");
            }
        }
    //
    // checks if raspberryPi has requested power off and it's time to switch power
    if (powerOffTime != 0 && now >= powerOffTime)
        {
        powerOffTime = 0;
        setMosfetStatus(false);
        rpiPowered = false;
        rpiPoweredBy = SBC_POWERED_BY_NONE;
        rpiStatusLed.FadeOff(500).Repeat(1);
        }
    //
    // updates Alarms service
    if (activationTimerAlarmId && now >= nextAlarmsServiceTime)
        {
        nextAlarmsServiceTime = now + 500;
        Alarm.delay(5);
        }
    //
    // updates mDNS service
    if (mDNSStarted && now >= nextMDNSUpdateTime)
        {
        nextMDNSUpdateTime = now + MDNS_UPDATE_TIME_INTERVAL;
        MDNS.update();
        //
        }

    // checks for NTP events
    if (ntpEventTriggered)
        {
        ntpEventTriggered = false;
        processNtpEvent(ntpEvent);
        }

    // checks if got a request to save activationTime
    if (gotSaveAlarmTimeRequest)
        {
        gotSaveAlarmTimeRequest = false;
        websocketBusyWait = true;
        // saves activation time structure to eeprom
        Serial.printf("[I] Saving activation time: %02d:%02d:%02d, enabled: %d\n", activationTime.hour, activationTime.minute, activationTime.second, activationTime.enabled);
        eepromWriteActivationTime();
        //
        // requests reinstancing of timer
        timeAlarmInstanceTime = now + 100;
        //
        websocketBusyWait = false;
        websocket.textAll("{\"event\":\"savedActivationTime\"}");
        }

    // checks if got a request to set SBC power on/off (from web interface)
    if (gotSetPowerRequest)
        {
        gotSetPowerRequest = false;
        //
        setMosfetStatus(gotSetPowerRequestStatus);
        rpiPoweredBy = gotSetPowerRequestStatus ? SBC_POWERED_BY_USER : SBC_POWERED_BY_NONE;
        if (gotSetPowerRequestStatus)
            statusLedHandler.setRaspberryPiStatus(StatusLedHandler::RPI_STATUS_BOOTING);
        else
            rpiStatusLed.FadeOff(500).Repeat(1);
        //
        String eventPayload = "{\"event\":\"setPower\", \"status\":";
        eventPayload += gotSetPowerRequestStatus ? "true }" : "false }";
        websocket.textAll(eventPayload.c_str());
        }
    //
    }