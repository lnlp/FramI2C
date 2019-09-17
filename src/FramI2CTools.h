/* FramI2CTools.cpp
 *
 * Description:  Utilities / helper functions for FramI2C.
 * 
 * Author:       Leonel Lopes Parente
 * License:      MIT (see LICENSE file in repository root)
 * 
 */

#ifndef FRAMI2CTOOLS_H_
#define FRAMI2CTOOLS_H_

#include <Arduino.h>
#include "FramI2C.h"


void printChars(Stream& stream, char ch, uint8_t count, bool linefeed = false)
{
    for (uint8_t i = 0; i < count; ++i)
    {
        stream.print(ch);
    }
    if (linefeed)
    {
        stream.println();
    }
}


void printSpaces(Stream& stream, uint8_t count, bool linefeed = false)
{
    printChars(stream, ' ', count, linefeed);
}


void printHex(Stream& stream, uint32_t value, bool prefix = true, uint8_t width = 0, bool uppercase = true)
{
    // Prints hex value to stream.
    // Value is prefixed with "0x" unless prefix is false.
    // If string length is uneven then a leading zero is added (0xF => "0x0F").
    // If width > value string length then multiple leading zeros will be added.
    // Note that width does not include the "0x" prefix.

    uint8_t zeros;
    String s(value, HEX);
    uint8_t len = s.length();

    if (uppercase)
    {
        s.toUpperCase();
    }

    if (width == 0)
    {
        zeros = len % 2;
    }
    else
    {
        zeros = width <= len ? 0 : width - len;
    }

    if (prefix)
    {
        stream.print("0x");
    }

    if (zeros > 0)
    {
        printChars(stream, '0', zeros);
    }
    stream.print(s);
}


void printFramInfo(Stream& stream, FramI2C& fram, const char* const instanceName = "FramI2C")
{
    if (!fram.isInitialized())
    {
        stream.print(instanceName);
        stream.println(F(" is not initialized.\n"));
    }
    else
    {
        // stream.printf("%s properties:\n\n", instanceName);        
        stream.print(instanceName);
        stream.println(F(" properties:"));
        printChars(stream, '-', strlen(instanceName) + 12);
        stream.println();

        // stream.printf("Density:          %u kb\n", fram.density()); 
        stream.print(F("Density:"));  
        printSpaces(stream, 10);  
        stream.print(fram.density(), DEC);    
        stream.println(F(" kb"));

        // stream.printf("I2C address:      0x%X\n", fram.i2cAddress());
        stream.print(F("I2C address:"));
        printSpaces(stream, 6);
        printHex(stream, fram.i2cAddress());
        stream.println();

        // stream.printf("Memory size:      0x%X B\n", fram.memorySize());
        stream.print(F("Memory size:"));
        printSpaces(stream, 6);
        if (fram.memorySize() < 1024)
        {
            stream.print(fram.memorySize());
            stream.println(F(" B"));
        }
        else
        {
            stream.print(fram.memorySize() / 1024);
            stream.println(F(" kB"));
        } 

        // stream.printf("Page size:        0x%X B\n", fram.pageSize());
        stream.print(F("Page size:"));
        printSpaces(stream, 8);        
        if (fram.pageSize() < 1024)
        {
            stream.print(fram.pageSize());
            stream.println(F(" B"));
        }
        else
        {
            stream.print(fram.pageSize() / 1024);
            stream.println(F(" kB"));
        }    

        // stream.printf("Page count:       %u\n", fram.pageCount());
        stream.print(F("Page count:"));
        printSpaces(stream, 7);        
        stream.println(fram.pageCount(), DEC);

        // stream.printf("Type buffer size: %u B\n\n", fram.typebufferSize());
        stream.print(F("Type buffer size: "));
        stream.print(fram.typebufferSize());
        stream.println(F(" B"));           
    }

	if (fram.isDeviceIdSupported())	
	{
        stream.print(F("Device ID:"));
        printSpaces(stream, 8);        		
        stream.println(F("supported"));		

        // stream.printf("Manufacturer ID:  %u\n", fram.manufacturerId());		
        stream.print(F("Manufacturer ID:  "));
        stream.println(fram.manufacturerId(), DEC);

        // stream.printf("Product ID:       0x%04X\n\n", fram.productId());	
        stream.print(F("Product ID:"));
        printSpaces(stream, 7);
        printHex(stream, fram.productId(), true, 3);
        stream.println();
	} 
    stream.println();
}


void printResultCodeDescription(
    Stream& stream, 
    const FramI2C::ResultCode resultcode, 
    uint8_t linefeeds = 1, 
    bool prefixSpace = false,
    bool postfixSpace = false)
{
    if (prefixSpace)
    {
        stream.print(" ");
    }
    
    if (!(resultcode == FramI2C::ResultCode::Success || resultcode == FramI2C::ResultCode::Uninitialized))
    {
        stream.print(F("Error: "));
    }

    switch (resultcode)
    {
        case FramI2C::ResultCode::Success:		
            stream.print(F("Success."));
            break;
        case FramI2C::ResultCode::I2CBufferOverflowError:		
            stream.print(F("I2C buffer overflow."));
            break;
        case FramI2C::ResultCode::I2CAddressNackError:		
            stream.print(F("I2C address not acknowleged (nack)."));
            break;
        case FramI2C::ResultCode::I2CDataNackError:		
            stream.print(F("I2C data not acknowleged (nack)."));
            break;
        case FramI2C::ResultCode::I2CLineBusyError:		
            stream.print(F("I2C line is busy."));
            break;
        case FramI2C::ResultCode::I2CReadError:
            stream.print(F("I2C Read Error."));
            break;
        case FramI2C::ResultCode::I2CWriteError:
            stream.print(F("I2C Write Error."));
            break;
        case FramI2C::ResultCode::I2CUnknownTwiResultCode:
            stream.print(F("I2C Unknown TwiResultCode."));
            break;
        case FramI2C::ResultCode::NullPtrError:		
            stream.print(F("Null pointer."));
            break;
        case FramI2C::ResultCode::NotInitializedError:		
            stream.print(F("FramI2C not initialized."));
            break;
        case FramI2C::ResultCode::AllreadyInitializedError:		
            stream.print(F("FramI2C already initialized (differently)."));
            break;
        case FramI2C::ResultCode::UnsupportedDensityError:		
            stream.print(F("Unsupported density."));
            break;
        case FramI2C::ResultCode::InvalidPageError:		
            stream.print(F("Invalid page."));
            break;			
        case FramI2C::ResultCode::PageSizeRangeError:		
            stream.print(F("Out of page size range."));
            break;
        case FramI2C::ResultCode::BufferAllocationFailedError:		
            stream.print(F("Type buffer allocation failed."));
            break;
        case FramI2C::ResultCode::BufferOverflowError:		
            stream.print(F("Type too large for buffer."));
            break;
        case FramI2C::ResultCode::Uninitialized:		
            stream.print(F("Uninitialized."));
            break;
    }
    if (postfixSpace)
    {
        stream.print(" ");
    }
    for (uint8_t i = 0; i < linefeeds; ++i)
    {
        stream.println();
    }
}


FramI2C::ResultCode hexdumpFram(
    Stream& stream, 
    FramI2C& fram, 
    const uint8_t page, 
    const uint16_t address, 
    const uint32_t byteCount, 
    const bool header = true, 
    const char* const message = "", 
    const uint8_t linefeeds = 1)
{
    // char pageString[9] = "";
    String pageStr;

    if (header)
    {
        // Print header
        if (fram.pageCount() > 1)
        {
            // Show page in header only if FRAM has multiple pages.
            pageStr = "page " + String(page) + ", ";
        }
        stream.print(F("FRAM hexdump - "));
        stream.print(pageStr);
        stream.print(F("address "));
        printHex(stream, address);
        stream.print(F(", "));
        printHex(stream, byteCount);
        stream.print(F(" ("));
        stream.print(byteCount, DEC);
        stream.println(F(") bytes"));
    }

    if (byteCount == 0)
    {
        stream.println(F("Byte count is 0 (framHexdump)."));
        for (uint8_t i = 0; i < linefeeds; ++i)
        {
            stream.println();
        }
        return FramI2C::ResultCode::Success;
    }

    if (page >= fram.pageCount())
    {
        printResultCodeDescription(stream, FramI2C::ResultCode::InvalidPageError, linefeeds);
        return FramI2C::ResultCode::InvalidPageError;
    }    	

    if ((address >= fram.pageSize()) || (byteCount + address) > fram.pageSize())
    {
        printResultCodeDescription(stream, FramI2C::ResultCode::PageSizeRangeError, linefeeds);
        return FramI2C::ResultCode::PageSizeRangeError;
    }    	

    if (header)
    {
        // Print label row
        stream.println();
        printSpaces(stream, 5);
        for (uint8_t i = 0; i < 16; ++i)
        {
            if (i == 8)
            {
                printSpaces(stream, 2);
            }
            stream.print(' ');
            printHex(stream, i, false);
        }
        stream.println();
        printSpaces(stream, 5);
        for (uint8_t i = 0; i < 16; ++i)
        {
            if (i == 8)
            {
                printSpaces(stream, 2);
            }
            stream.print(" --");
        }
        stream.println();
    }

    // Output data

    uint16_t lineNr = 0;
    uint16_t firstByteColumnOffset = address & 0xF;
    if (firstByteColumnOffset > 0)
    {
        ++lineNr;
        // Output first adreslabel and indent
        printHex(stream, address & 0xFFF0, false, 4); 
        stream.print(":");
        for (uint8_t i = 0; i < firstByteColumnOffset; ++i)		
        {
            if (i == 8 && (firstByteColumnOffset) >= 8)
            {
                printSpaces(stream, 2);
            } 
            printSpaces(stream, 3);
        }
    }

    // Output byte values
    FramI2C::ResultCode resultcode = FramI2C::ResultCode::Uninitialized;
    uint16_t addr = address;	
    for (uint32_t i = 0; i < byteCount; ++i)
    {
        // stream.printf("[index=%u, addr=0x%04X, lineNr=%u]", index, addr, lineNr); //debug
        if ((addr & 0xF) == 0)
        {
#if defined(ESP8266)
            // If ESP8266 MCU yield() once every 16 bytes to prevent WDT resets.
            yield();
#endif						
            ++lineNr;
            if (lineNr > 1)
            {
                stream.println();
            }
            // Write address label
            printHex(stream, addr & 0xFFF0, false, 4);             
            stream.print(":");
        }
        if ((addr & 0xF) ==  8)
        {
            stream.print(" -");
        }		
        uint8_t value = 0;
        resultcode = fram.readBytes(page, addr, 1, &value);
        if (resultcode != FramI2C::ResultCode::Success)
        {
            stream.println();
            printResultCodeDescription(stream, resultcode, 0);
            break;
        }
        stream.print(" ");        
        printHex(stream, value, false, 2);             
        ++addr;
    }
    if (resultcode == FramI2C::ResultCode::Success)
    {
        stream.println();
    }
    for (uint8_t i = 0; i < linefeeds; ++i)
    {
        stream.println();
    }

    return resultcode;
}


FramI2C::ResultCode hexdumpFram(
    Stream& stream, 
    FramI2C& fram, 
    const uint16_t address, 
    const uint32_t byteCount, 
    const bool header = true, 
    const char* const message = nullptr, 
    const uint8_t linefeeds = 1)
                                        
{
    // Overload without page parameter.
    return hexdumpFram(stream, fram, 0, address, byteCount, header, message, linefeeds);
}

#endif  //FRAMI2CTOOLS_H_