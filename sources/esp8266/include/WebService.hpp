
#ifndef _POWERMANAGER_WEBSERVICE_HPP
#define _POWERMANAGER_WEBSERVICE_HPP

// -----------------------------------------------------------------------------

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "AsyncJson.h"
#include "StaticData.hpp"

char wsResponseBuffer[512];

AsyncWebServer server(80);
AsyncWebSocket websocket("/ws");

// -----------------------------------------------------------------------------

void processWebsocketRequest(AsyncWebSocketClient* client, String message)
	{
	if (websocketBusyWait)
		return;
	//
	StaticJsonDocument<200> jsonPayload;
	DeserializationError error = deserializeJson(jsonPayload, message);
	if (error)
		{
		client->text("{\"error\":\"malformed json request\"}");
		return;
		}
	//
	// StaticJsonDocument jsonBuffer;
	// Serial.printf("processWebsocketRequest: %s\n", message.c_str());
	// TODO: handle request here
	JsonVariant request = jsonPayload["request"];
	if (!request.isNull())
		{
		StaticJsonDocument<512> jsonResp;
		//
		// Serial.printf("   +> handling request: %s\n", request.as<const char *>());
		if (strcmp(request.as<const char*>(), "status") == 0)
			{
			jsonResp["response"] = "status";
			JsonObject data = jsonResp.createNestedObject("data");
			// data["sbcPowered"] = digitalRead(MOSFET_CONTROL_PIN) ? "on" : "off";
			data["sbcPowered"] = digitalRead(MOSFET_CONTROL_PIN) ? true : false;
			//
			JsonObject timeData = data.createNestedObject("time");
			if (NTP.syncStatus() == syncd)
				{
				timeData["value"] = NTP.getTimeDateString(time(NULL), "%04Y-%02m-%02d %02H:%02M:%02S");
				}
			else
				{
				timeData["value"] = "";
				timeData["err"] = "time not syncronized yet";
				}
			//
			JsonObject activationTimeData = data.createNestedObject("activationTime");
			activationTimeData["hour"] = activationTime.hour;
			activationTimeData["minute"] = activationTime.minute;
			activationTimeData["second"] = activationTime.second;
			activationTimeData["enabled"] = activationTime.enabled ? true : false;
			}
		else if (strcmp(request.as<const char*>(), "saveActivationTime") == 0)
			{
			JsonVariant newActivationTime = jsonPayload["activationTime"];
			if (!newActivationTime.isNull())
				{
				JsonVariant hour = newActivationTime["hour"];
				if (!hour.isNull())
					{
					JsonVariant minute = newActivationTime["minute"];
					if (!minute.isNull())
						{
						JsonVariant second = newActivationTime["second"];
						if (!second.isNull())
							{
							JsonVariant enabled = newActivationTime["enabled"];
							if (!enabled.isNull())
								{
								activationTime.hour = hour.as<unsigned char>();
								activationTime.minute = minute.as<unsigned char>();
								activationTime.second = second.as<unsigned char>();
								bool enabledBoolValue = enabled.as<bool>();
								activationTime.enabled = enabledBoolValue ? 1 : 0;
								//
								gotSaveAlarmTimeRequest = true;
								}
							}
						}
					}
				}
			//
			jsonResp["event"] = "savingActivationTime";
			}
		else if (strcmp(request.as<const char*>(), "setPower") == 0)
			{
			JsonVariant requestedStatus = jsonPayload["status"];
			if (!requestedStatus.isNull())
				{
				gotSetPowerRequest = true;
				gotSetPowerRequestStatus = requestedStatus.as<bool>();
				//
				jsonResp["event"] = "requestedSetPower";
				}
			}

		// else if (strcmp(request.as<const char *>(), "powerOnTime") == 0)
		// {
		// 	jsonResp["response"] = "activationTime";
		// 	JsonObject data = jsonResp.createNestedObject("data");
		// 	data["hour"] = activationTime.hour;
		// 	data["minute"] = 2;
		// 	data["second"] = 3;
		// }
		//
		serializeJson(jsonResp, wsResponseBuffer);
		// Serial.printf("   +> sending response: %s\n", wsResponseBuffer);
		websocket.textAll(wsResponseBuffer);
		}
	//
	// String response = String(millis(), DEC);
	// client->text(response);
	}

// ----------------------------------------------------------------------------

void onWebsocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len)
	{
	if (type == WS_EVT_CONNECT)
		{
		//Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
		// client->printf("Hello Client %u :)", client->id());
		client->ping();
		}
	else if (type == WS_EVT_DISCONNECT)
		{
		//Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
		}
	else if (type == WS_EVT_ERROR)
		{
		//Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
		}
	else if (type == WS_EVT_PONG)
		{
		//Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
		}
	else if (type == WS_EVT_DATA)
		{
		AwsFrameInfo* info = (AwsFrameInfo*)arg;
		String msg = "";
		if (info->final && info->index == 0 && info->len == len)
			{
			if (info->opcode == WS_TEXT)
				{
				for (size_t i = 0; i < info->len; i++)
					{
					msg += (char)data[i];
					}
				}
			else
				{
				char buff[3];
				for (size_t i = 0; i < info->len; i++)
					{
					sprintf(buff, "%02x ", (uint8_t)data[i]);
					msg += buff;
					}
				}

			if (info->opcode == WS_TEXT)
				processWebsocketRequest(client, msg);
			}
		else
			{
			//message is comprised of multiple frames or the frame is split into multiple packets
			if (info->opcode == WS_TEXT)
				{
				for (size_t i = 0; i < len; i++)
					{
					msg += (char)data[i];
					}
				}
			else
				{
				char buff[3];
				for (size_t i = 0; i < len; i++)
					{
					sprintf(buff, "%02x ", (uint8_t)data[i]);
					msg += buff;
					}
				}
			// Serial.printf("%s\n", msg.c_str());

			if ((info->index + len) == info->len)
				{
				if (info->final)
					{
					if (info->message_opcode == WS_TEXT)
						processWebsocketRequest(client, msg);
					}
				}
			}
		}
	}

// -----------------------------------------------------------------------------

/*String GetContentType(String filename)
	{
	if(filename.endsWith(".htm")) return "text/html";
	else if(filename.endsWith(".html")) return "text/html";
	else if(filename.endsWith(".css")) return "text/css";
	else if(filename.endsWith(".js")) return "application/javascript";
	else if(filename.endsWith(".png")) return "image/png";
	else if(filename.endsWith(".gif")) return "image/gif";
	else if(filename.endsWith(".jpg")) return "image/jpeg";
	else if(filename.endsWith(".ico")) return "image/x-icon";
	else if(filename.endsWith(".xml")) return "text/xml";
	else if(filename.endsWith(".pdf")) return "application/x-pdf";
	else if(filename.endsWith(".zip")) return "application/x-zip";
	else if(filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
	}


void ServeFile(AsyncWebServerRequest* webserver, String path, String contentType)
	{
	File file = SPIFFS.open(path, "r");
	size_t sent = webserver->streamFile(file, contentType);
	file.close();
	}


bool handleFileReadGzip(AsyncWebServer* webserver, String path)
	{
	if (path.endsWith("/")) path += "index.html";
		Serial.println("handleFileRead: " + path);

	if (SPIFFS.exists(path))
		{
		ServeFile(webserver, path, GetContentType(path));
		return true;
		}
	else
		{
		String pathWithGz = path + ".gz";
		if (SPIFFS.exists(pathWithGz))
			{
			ServeFile(webserver, pathWithGz, GetContentType(path));
			return true;
			}
		}
	Serial.println("\tFile Not Found");
	return false;
	}*/

void initWebService(AsyncWebServer* webserver, AsyncWebSocket* websocket)
	{
	//
	// initializes websocket
	websocket->onEvent(&onWebsocketEvent);
	webserver->addHandler(websocket);
	Serial.println("[I] WebSocket server started");
	//
	// initializes webserver
	webserver->serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
	webserver->onNotFound([](AsyncWebServerRequest* request)
		{ request->send(404, "text/plain", "Not found"); });
	//
	// gzip service tryout
	// webserver->onNotFound([](AsyncWebServerRequest* request) {                    // If the client requests any URI
	// 	if (!handleFileReadGzip(request, webserver->uri()))   // send it if it exists
	// 		//  handleNotFound();  				   // otherwise, respond with a 404 (Not Found) error
	// 		request->send(404, "text/plain", "Not found");
	// 	});
	//
	webserver->begin();
	Serial.println("[I] HTTP server started");
	}

// -----------------------------------------------------------------------------

#endif
