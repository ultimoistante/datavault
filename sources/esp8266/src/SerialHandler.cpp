#include "SerialHandler.h"

// -----------------------------------------------------------------------------


void SerialHandler::begin(SoftwareSerial* serial)
	{
	//strcpy(this->chip_id, chip_id);
	//this->swSerial = new SoftwareSerial(SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);
	this->serial = serial;
	this->reading = 0;
	}


// -----------------------------------------------------------------------------


void SerialHandler::doReceive()
	{
	while (this->serial->available())
		{
		//
		char inByte = this->serial->read();
		// DEBUG
		// Serial.printf("r: %c\n", inByte);
		// delay(20);
		//
		if (this->reading == 0) // still waiting for start of message
			{
			if (inByte == this->start_of_packet_char) // received start of message
				{
				this->reading = 1;
				this->rxBuffer[0] = '\0';
				this->rxBufferPtr = 0;
				}
			}
		else
			{
			if (inByte == this->end_of_packet_char) // received end of message character
				{
				this->reading = 0;
				this->processLine();
				}
			else
				{
				this->rxBuffer[this->rxBufferPtr] = inByte; // appends message character at end of buffer
				if (this->rxBufferPtr < (SERIAL_RX_BUFFER_SIZE - 1))
					{
					this->rxBufferPtr++;
					this->rxBuffer[this->rxBufferPtr] = '\0';
					}
				}
			}
		}
	}


// -----------------------------------------------------------------------------



void SerialHandler::processLine()
	{
	// splits message in tokens
	int tokenCount = this->charStringSplit(this->rxBuffer, this->separator_chars, this->bufferTokens);
	// then calls message handler callback
	this->onMessageCallback(this->bufferTokens, tokenCount);
	}



// -----------------------------------------------------------------------------


void SerialHandler::sendMessage(const char* message)
	{
	// sprintf(this->txBuffer, "%c%s%c\n", this->start_of_packet_char, message, this->end_of_packet_char);
	//
	this->serial->write(this->start_of_packet_char);
	this->serial->print(message);
	this->serial->write(this->end_of_packet_char);
	this->serial->write('\n');
	// delay(50);
	}


// -----------------------------------------------------------------------------


/*void SerialHandler::publish(const char* topic, const char* payload)
	{
	digitalWrite(LED_3, 0);
	//digitalWrite(LED_3, 1);
	this->serial->write(this->start_of_packet_char);
	this->serial->print(topic);
	this->serial->write(this->separator_chars[0]);
	this->serial->print(payload);
	this->serial->write(this->end_of_packet_char);
	delay(40);
	//this->serial->write(this->end_of_packet_char);
	//delay(20);
	digitalWrite(LED_3, 1);
	//digitalWrite(LED_3, 0);
	}*/


	// -----------------------------------------------------------------------------


uint8_t SerialHandler::charStringSplit(char* input, char* separators, char** tokens)
	{
	//char str[] ="CMD|TEST|ARGUMENT|1";
	// Serial.print("INPUT string: ");
	// Serial.println(input);
	//
	unsigned char tokens_count = 0;
	char* token;
	char* rest = input;
	while ((token = strtok_r(rest, separators, &rest)))
		{
		//printf("%s\n", token);
		*tokens++ = token;
		//Serial.println(token);
		tokens_count++;
		}
	return tokens_count;
	}




// -----------------------------------------------------------------------------



// --- Serial ports section ----------------------------------------------------


/*SoftwareSerial serial(SW_SERIAL_RX_PIN, SW_SERIAL_TX_PIN);
String         serialRxBuffer = "";
byte           serialReading = 0;
String         serialBufferTokens[5];
String         serialResponse;

// -----------------------------------------------------------------------------


void beginOnboardCommunications()
	{
	// set the data rate for the SoftwareSerial port
	serial.begin(SW_SERIAL_BAUDRATE);
	}


// -----------------------------------------------------------------------------


void SerialHandlerReceiveHandler()
	{
	if(serial.available())
		{
		char inByte = serial.read();
		//
		if(serialReading == 0) // still waiting for start of message
			{
			if(inByte == SW_SERIAL_SOL) // received start of message
				{
				serialReading = 1;
				serialRxBuffer = "";
				}
			}
		else
			{
			if(inByte == SW_SERIAL_EOL) // received end of message character
				{
				serialReading = 0;
				SerialHandlerProcessMessage();
				}
			else
				serialRxBuffer += inByte; // appends message character at end of buffer
			}
		}
	}


// -----------------------------------------------------------------------------


void SerialHandlerProcessMessage()
	{
	Serial.print("[NodeMCU] received message: ");
	Serial.println(serialRxBuffer);
	// parse the line
	int tokenCount = StringSplit(serialRxBuffer, '|', serialBufferTokens, 5);
	// print the extracted paramters
	for(int i=0; i<tokenCount; i++) {
		Serial.println(serialBufferTokens[i]);
	}
	Serial.println("");
	if(tokenCount > 1)
		{
		if(serialBufferTokens[0] == "REQ")
			{
			if(serialBufferTokens[1] == "CLIENTID")
				{
				serialResponse = "RES|CLIENTID|";
				serialResponse += chipId;
				serialSendMessage(serialResponse.c_str());
				}
			}
		}
	}


	*/
	// -----------------------------------------------------------------------------
