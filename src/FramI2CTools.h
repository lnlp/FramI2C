/* FramI2CTools.cpp
 *
 * Description:  Utilities / helper functions for FramI2C.
 * 
 * Note:         Initially designed for ESP8266 therefore uses printf.
 *               To be replaced with standard Arduino print and println in a future version.
 * 
 * Author:       Leonel Lopes Parente
 * License:      MIT (see LICENSE file in repository root)
 * 
 */

#ifndef FRAMI2CTOOLS_H_
#define FRAMI2CTOOLS_H_

#include <Arduino.h>
#include "FramI2C.h"


void printFramInfo(Stream& stream, FramI2C& fram, const char* const instanceName = "FramI2C")
{
    if (!fram.isInitialized())
    {
        stream.printf("FramI2C is not initialized.\n\n");
    }
    else
    {
        stream.printf("%s properties:\n\n", instanceName);
        stream.printf("  Density:           %u kilobits\n", fram.density());    
        stream.printf("  I2C address:       0x%X\n", fram.i2cAddress());
        stream.printf("  Memory size:       0x%X bytes\n", fram.memorySize());
        stream.printf("  Page size:         0x%X bytes\n", fram.pageSize());
        stream.printf("  Number of pages:   %u\n", fram.pageCount());
        stream.printf("  Type buffer size:  %u bytes\n\n", fram.typebufferSize());
    }

	if (fram.isDeviceIdSupported())	
	{
    	stream.println("  Device ID is supported");		
    	stream.printf("  Manufacturer ID:   %u\n", fram.manufacturerId());		
    	stream.printf("  Product ID:        0x%04X\n\n", fram.productId());		
	} 
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
        stream.print("Error: ");
    }

    switch (resultcode)
    {
        case FramI2C::ResultCode::Success:		
            stream.print("Success.");
            break;
        case FramI2C::ResultCode::I2CBufferOverflowError:		
            stream.print("I2C buffer overflow.");
            break;
        case FramI2C::ResultCode::I2CAddressNackError:		
            stream.print("I2C address not acknowleged (nack).");
            break;
        case FramI2C::ResultCode::I2CDataNackError:		
            stream.print("I2C data not acknowleged (nack).");
            break;
        case FramI2C::ResultCode::I2CLineBusyError:		
            stream.print("I2C line is busy.");
            break;
        case FramI2C::ResultCode::I2CReadError:
            stream.print("I2C Read Error.");
            break;
        case FramI2C::ResultCode::I2CWriteError:
            stream.print("I2C Write Error.");
            break;
        case FramI2C::ResultCode::I2CUnknownTwiResultCode:
            stream.print("I2C Unknown TwiResultCode.");
            break;
        case FramI2C::ResultCode::NullPtrError:		
            stream.print("Null pointer.");
            break;
        case FramI2C::ResultCode::NotInitializedError:		
            stream.print("FramI2C not initialized.");
            break;
        case FramI2C::ResultCode::AllreadyInitializedError:		
            stream.print("FramI2C already initialized (differently).");
            break;
        case FramI2C::ResultCode::UnsupportedDensityError:		
            stream.print("Unsupported density.");
            break;
        case FramI2C::ResultCode::InvalidPageError:		
            stream.print("Invalid page.");
            break;			
        case FramI2C::ResultCode::PageSizeRangeError:		
            stream.print("Out of page size range.");
            break;
        case FramI2C::ResultCode::BufferAllocationFailedError:		
            stream.print("Type buffer allocation failed.");
            break;
        case FramI2C::ResultCode::BufferOverflowError:		
            stream.print("Type too large for buffer.");
            break;
        case FramI2C::ResultCode::Uninitialized:		
            stream.print("Uninitialized.");
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
    const char* const message = nullptr, 
    const uint8_t linefeeds = 1)
{
    char pageString[9] = "";

    if (header)
    {
        if (fram.pageCount() > 1)
        {
            // Show page in header only if FRAM has multiple pages.
            sprintf(pageString, "page %u, ", page);
        }
        if (message == nullptr || strlen(message) == 0)
        {
            stream.printf("FRAM hexdump - %saddress 0x%X, 0x%X (%u) bytes\n", pageString, address, byteCount, byteCount);
        }
        else
        {
            stream.printf("FRAM hexdump %s - %saddress 0x%X, 0x%X (%u) bytes\n", message, pageString, address, byteCount, byteCount);
        }
    }

    if (byteCount == 0)
    {
        stream.println("Byte count is 0 (framHexdump).");
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
        stream.println();

        // Output header
        stream.print("     ");
        for (uint8_t i = 0; i < 16; ++i)
        {
            if (i == 8)
            {
                stream.print("  ");
            }
            stream.printf(" %02X", i);
        }
        stream.print("\n     ");
        for (uint8_t i = 0; i < 16; ++i)
        {
            if (i == 8)
            {
                stream.print("  ");
            }
            stream.printf(" --");
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
        stream.printf("%04X:", address & 0xFFF0);
        for (uint8_t i = 0; i < firstByteColumnOffset; ++i)		
        {
            if (i == 8 && (firstByteColumnOffset) >= 8)
            {
                stream.print("  ");
            } 
            stream.print("   ");
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
            stream.printf("%04X:", addr & 0xFFF0);
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
        stream.printf(" %02X", value);
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