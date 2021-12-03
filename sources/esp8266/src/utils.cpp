#include "utils.h"


// -----------------------------------------------------------------------------


int Utils::charStringSplit(char* input, char* separators, char** tokens)
    {
	//char str[] ="CMD|TEST|ARGUMENT|1";
	Serial.print("INPUT string: ");
	Serial.println(input);
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


int Utils::stringSplit(String sInput, char cDelim, String sTokens[], int iMaxTokens)
    {
    int iTokenCount = 0;
    int iPosDelim, iPosStart = 0;

    do
	{
        // Searching the delimiter using indexOf()
        iPosDelim = sInput.indexOf(cDelim, iPosStart);
        if (iPosDelim > (iPosStart+1))
	    {
            // Adding a new parameter using substring() 
            //sTokens[iTokenCount] = sInput.substring(iPosStart, iPosDelim-1);
            sTokens[iTokenCount] = sInput.substring(iPosStart, iPosDelim);
            iTokenCount++;
            // Checking the number of parameters
            if (iTokenCount >= iMaxTokens)
                return (iTokenCount);
            iPosStart = iPosDelim + 1;
	    }
	}
    while (iPosDelim >= 0);
	//
    if (iTokenCount < iMaxTokens)
	{
        // Adding the last parameter as the end of the line
        sTokens[iTokenCount] = sInput.substring(iPosStart);
        iTokenCount++;
	}

    return iTokenCount;
    }

// -----------------------------------------------------------------------------


// Helper routine to dump a byte array as hex values to Serial
String Utils::byte_array_to_hex(byte* buffer, byte bufferSize)
	{
	String hexBuffer = "";
	for (byte i = 0; i < bufferSize; i++)
		{
		// high nibble
		char nibbleChar = (buffer[i] >> 4) + 0x30;	// offsets to '0' char (0x30 hex, 48 dec) 
		if (nibbleChar > 0x39) nibbleChar +=7;		// jumps to 'A' character
		hexBuffer += nibbleChar;
		// low nibble
		nibbleChar = (buffer[i] & 0x0F) + 0x30;		// offsets to '0' char (0x30 hex, 48 dec) 
		if (nibbleChar > 0x39) nibbleChar +=7;		// jumps to 'A' character
		hexBuffer += nibbleChar;
		}
	return hexBuffer;
	}


// -----------------------------------------------------------------------------

