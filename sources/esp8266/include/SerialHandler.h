#ifndef _SERIAL_HANDLER_H
#define _SERIAL_HANDLER_H

// -----------------------------------------------------------------------------

#include <Arduino.h>
#include <SoftwareSerial.h>

// -----------------------------------------------------------------------------

#define SERIAL_RX_BUFFER_SIZE	128
// #define SERIAL_TX_BUFFER_SIZE	128

// -----------------------------------------------------------------------------


class SerialHandler
    {
    public:
        void begin(SoftwareSerial* serial);
        //
        void doReceive();
        //
        void sendMessage(const char* message);
        //void sendMessage(const char* message);
        // void publish(const char* topic, const char* payload);
        //
        void setPacketDelimiterChars(char start_of_packet_char, char end_of_packet_char) { this->start_of_packet_char = start_of_packet_char; this->end_of_packet_char = end_of_packet_char; }
        void setTokenCharSeparators(const char* separators) { strcpy(this->separator_chars, separators); }
        void setMessageHandler(void (*onMessageCallback)(char**, int)) { this->onMessageCallback = onMessageCallback; }
        //
    private:
        SoftwareSerial* serial;
        char            start_of_packet_char;
        char            end_of_packet_char;
        char            separator_chars[8];
        char            rxBuffer[SERIAL_RX_BUFFER_SIZE];
        // char            txBuffer[SERIAL_TX_BUFFER_SIZE];
        uint16_t        rxBufferPtr;
        byte			reading;
        char* bufferTokens[8];
        //String			response;
        //
        void processLine();
        //
        void (*onMessageCallback)(char**, int);
        //
        uint8_t charStringSplit(char* input, char* separators, char** tokens);
    };



#endif
