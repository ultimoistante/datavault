#ifndef _HOLAPP_UTILS_H
#define _HOLAPP_UTILS_H

#include <Arduino.h>

class Utils
	{
	public:
		static int charStringSplit(char* input, char* separators, char** tokens);
		static int stringSplit(String sInput, char cDelim, String sTokens[], int iMaxTokens);
		static String byte_array_to_hex(byte* buffer, byte bufferSize);

	};


//int StringSplit(String sInput, char cDelim, String sTokens[], int iMaxTokens);
//String byte_array_to_hex(byte* buffer, byte bufferSize);

#endif