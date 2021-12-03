#include "WiFiHandler.h"

// -----------------------------------------------------------------------------

const uint8_t WiFiHandler::STATUS_STAMODE_NOT_CONNECTED = 0;
WiFiHandler* WiFiHandler::instance = NULL;

// -----------------------------------------------------------------------------


// void WiFiHandler::begin(StatusLedHandler* statusLedHandler, SerialHandler* serialHandler)
void WiFiHandler::begin(StatusLedHandler* statusLedHandler)
    {
    WiFiHandler::instance = this;
    //
    this->statusLedHandler = statusLedHandler;
    // this->serialHandler = serialHandler;
    // this->websocketServer = websocketServer;
    // 
    this->currentStatus = STATUS_STAMODE_NOT_CONNECTED;
    //
    WiFi.onEvent(this->onEvent);
    // WiFi.persistent( false );
    // WiFi.setAutoConnect(true);
    // WiFi.disconnect(); // first cycle which may be broken
    WiFi.mode(WIFI_STA);
    // WiFi.begin (ssid, password);
    // WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str()); // tries connection with data stored in EEPROM (from last succesful connection)
    // delay(200);
    WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str()); // tries connection with data stored in EEPROM (from last succesful connection)
    Serial.println("[I] trying connection with last stored credentials...");
    uint8_t retries = 20;
    while (WiFi.status() == WL_DISCONNECTED && retries > 0)
        {          // last saved credentials
        delay(500);
        retries--;
        // Serial.print(".");
        }
    //
    if (WiFi.status() == WL_DISCONNECTED)
        Serial.println("[W] connection timeout!");
    }

// -----------------------------------------------------------------------------

/*
typedef enum WiFiEvent
{
    WIFI_EVENT_STAMODE_CONNECTED = 0,
    WIFI_EVENT_STAMODE_DISCONNECTED,
    WIFI_EVENT_STAMODE_AUTHMODE_CHANGE,
    WIFI_EVENT_STAMODE_GOT_IP,
    WIFI_EVENT_STAMODE_DHCP_TIMEOUT,
    WIFI_EVENT_SOFTAPMODE_STACONNECTED,
    WIFI_EVENT_SOFTAPMODE_STADISCONNECTED,
    WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED,
    WIFI_EVENT_MODE_CHANGE,
    WIFI_EVENT_SOFTAPMODE_DISTRIBUTE_STA_IP,
    WIFI_EVENT_MAX,
    WIFI_EVENT_ANY = WIFI_EVENT_MAX,
} WiFiEvent_t;
*/

void WiFiHandler::onEvent(WiFiEvent_t event)
    {
    Serial.print("[D] [WiFi-event] event: ");
    // 
    switch (event)
        {
            case WIFI_EVENT_MODE_CHANGE:
                Serial.println("WIFI_EVENT_MODE_CHANGE");
                instance->statusLedHandler->setDeviceStatus(StatusLedHandler::DEVICE_STATUS_LED_MODE_NOT_CONNECTED);
                /*
                // WPS button I/O setup
                // pinMode(0,OUTPUT);         // Use GPIO0
                // digitalWrite(0,LOW);       // for hardware safe operation.
                pinMode(WPS_BUTTON_PIN, INPUT_PULLUP);  // Push Button for GPIO2 active LOW
                // Serial.printf("\nCould not connect to WiFi. state='%d'\n", status);
                Serial.printf("\nCould not connect to WiFi.\n");
                Serial.println("Please press WPS button on your router, until mode is indicated.");
                Serial.println("next press the ESP module WPS button, router WPS timeout = 2 minutes");
                //
                while(digitalRead(WPS_BUTTON_PIN) == HIGH)  // wait for WPS Button active
                    yield(); // do nothing, allow background work (WiFi) in while loops
                //
                setDeviceStatusLed(DEVICE_STATUS_LED_MODE_TRYING_WPS);
                Serial.println("WPS button pressed");

                if(!tryWPSConnection())
                    {
                    Serial.println("Failed to connect with WPS :-(");
                    }
                else
                    {
                    WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str()); // reading data from EPROM,
                    while (WiFi.status() == WL_DISCONNECTED)
                        {          // last saved credentials
                        delay(500);
                        Serial.print("."); // show wait for connect to AP
                        }
                    //   pinMode(0,INPUT);    // GPIO0, LED OFF, show WPS & connect OK
                    }*/
                break;
            case WIFI_EVENT_STAMODE_CONNECTED:
                Serial.println("[D] WIFI_EVENT_STAMODE_CONNECTED");
                Serial.printf("[I] Connected to %s. Asking for IP address.\r\n", WiFi.BSSIDstr().c_str());
                break;
            case WIFI_EVENT_STAMODE_GOT_IP:
                Serial.println("[D] WIFI_EVENT_STAMODE_GOT_IP");
                Serial.printf("[I] Got IP: %s\r\n", WiFi.localIP().toString().c_str());
                Serial.printf("[I] Connected: %s\r\n", WiFi.status() == WL_CONNECTED ? "yes" : "no");
                instance->statusLedHandler->setDeviceStatus(StatusLedHandler::DEVICE_STATUS_LED_MODE_CONNECTED);
                //
                // starts websocket server
                // instance->websocketServer->listen(WEBSOCKET_SERVER_PORT);
                if (instance->onConnectCallback)
                    instance->onConnectCallback();

                // setDeviceStatusLed(DEVICE_STATUS_LED_MODE_CONNECTED);
                // digitalWrite (ONBOARDLED, LOW); // Turn on LED
                // wifiFirstConnection = true;
                break;
            case WIFI_EVENT_STAMODE_DISCONNECTED:
                Serial.println("[D] WIFI_EVENT_STAMODE_DISCONNECTED");
                Serial.printf("[I] Disconnected from SSID: %s\n", WiFi.BSSIDstr().c_str());
                // setDeviceStatusLed(DEVICE_STATUS_LED_MODE_CONNECTING);
                //Serial.printf ("Reason: %d\n", info.disconnected.reason);
                // digitalWrite (ONBOARDLED, HIGH); // Turn off LED
                //NTP.stop(); // NTP sync can be disabled to avoid sync errors
                WiFi.reconnect();
                break;
            default:
                Serial.print("[D] [unhandled event] ");
                Serial.println(event);
                break;
        }
    }


// -----------------------------------------------------------------------------


bool WiFiHandler::tryWPSConnection()
    {
    // from https://gist.github.com/copa2/fcc718c6549721c210d614a325271389
    // wpstest.ino
    this->statusLedHandler->setDeviceStatus(StatusLedHandler::DEVICE_STATUS_LED_MODE_TRYING_WPS);
    //
    Serial.println("WPS config start");
    bool wpsSuccess = WiFi.beginWPSConfig();
    if (wpsSuccess)
        {
        // Well this means not always success :-/ in case of a timeout we have an empty ssid
        String newSSID = WiFi.SSID();
        if (newSSID.length() > 0)
            {
            // WPSConfig has already connected in STA mode successfully to the new station. 
            Serial.printf("WPS finished. Connected successfull to SSID '%s'\n", newSSID.c_str());
            }
        else
            {
            wpsSuccess = false;
            }
        }
    return wpsSuccess;
    }


// -----------------------------------------------------------------------------
