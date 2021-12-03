#ifndef _POWERMANAGER_WIFIHANDLER_H
#define _POWERMANAGER_WIFIHANDLER_H

#include <ESP8266WiFi.h>
// #include <ArduinoWebsockets.h>
// using namespace websockets;
#include "StatusLedHandler.h"
#include "SerialHandler.h"
#include "defines.h"

// -----------------------------------------------------------------------------

class WiFiHandler
	{
	public:
		static const uint8_t STATUS_STAMODE_NOT_CONNECTED;
		//
		static WiFiHandler* instance;
		//
		WiFiHandler() { this->onConnectCallback = NULL; }
		//
		// void begin(StatusLedHandler* statusLedHandler, SerialHandler* serialHandler);
		void begin(StatusLedHandler* statusLedHandler);
		//
		bool tryWPSConnection();
		//
		uint8_t getCurrentStatus() { return this->currentStatus; }
		//
		void setOnConnectCallback(void (*onConnectCallback)()) { this->onConnectCallback = onConnectCallback; }

		// void handleWebRequest();
		//
		// void update();
		// //
		// void setOnExpiredCallback(void (* onExpiredCallback)()) { this->onExpiredCallback = onExpiredCallback; }
		//
	private:
		StatusLedHandler* statusLedHandler;
		// SerialHandler* serialHandler;
		// WebsocketsServer* websocketServer;
		//
		uint8_t currentStatus;
		// //
		// unsigned long currentTime;
		// unsigned long previousTime;
		// unsigned long timeoutMsec;
		// //
		// // Variable to store the HTTP request
		// String header;

		// unsigned long milliseconds;
		// unsigned long remaining;
		// //
		// void (* onExpiredCallback)();
		WiFiEvent_t lastEvent;

		static void onEvent(WiFiEvent_t event);

		void (*onConnectCallback)();

	};

// -----------------------------------------------------------------------------

#endif
