/* FramI2C.cpp
 *
 * Description:  FramI2C is a driver for I2C nonvolatile Ferroelectric RAM (F-RAM, FRAM, FeRAM) memory.
 *               It supports Cypress and Fujitsu I2C FRAM chips with densities from 4kb to 1Mb (bits)
 * 
 * Author:       Leonel Lopes Parente
 * License:      MIT (see LICENSE file in repository root)
 * 
 */

#include <Wire.h>
#include "FramI2C.h"


// --- Public -----------------------------------------------------------------

FramI2C::FramI2C()
{
    // Empty. All initialization is done in begin().
}


FramI2C::~FramI2C()
{   // Deallocate buffer (if size > 0 and end() was not called).
    if (typebuffer_ != nullptr)
    {
        delete[] typebuffer_;
    }
}


FramI2C::ResultCode FramI2C::begin(const uint16_t densityInKiloBits, const uint8_t i2cAddress, const size_t typebufferSize)
{
    // Initialization method for the FramI2C class instance. This method shall only be called once.
    // The application must first initialize the I2C interface by calling Wire.begin()
    // before FramI2C's begin() method is called.

    if (initialized_)
    {
        // begin() is called again while already initialized.
        if (densityInKiloBits == density_ && i2cAddress == i2cAddress_ && typebufferSize == typebufferSize_)
        {
            // Parameters are identical, FramI2C is already initialized identically.
            // While begin() should only be called once, ignore and return success (don't fail if not neccesary).
            return FramI2C::ResultCode::Success;
        }
        else
        {            
            // Parameters are different from current so return error.
            return FramI2C::ResultCode::AllreadyInitializedError;
        }
    }

    if (!isDensitySupported(densityInKiloBits))
    {
       return FramI2C::ResultCode::UnsupportedDensityError;
    }    

    pageSize_ = densityToPageSize(densityInKiloBits);

    if (typebufferSize > pageSize_)
    {
       return FramI2C::ResultCode::BufferAllocationFailedError;
    }    

    // typebuffer_ = new uint8_t[typebufferSize];
    typebuffer_ = static_cast<uint8_t*>(malloc(typebufferSize));
    if (typebuffer_ == nullptr)
    {
       return FramI2C::ResultCode::BufferAllocationFailedError;
    }    

    typebufferSize_ = typebufferSize;
    i2cAddress_ = i2cAddress;
    density_ = densityInKiloBits;
    memorySize_ = densityToMemorySize(densityInKiloBits);
    addressBytesCount_ = (pageSize_ == 0x100) ? 1 : 2;
    pageCount_ = memorySize_ / pageSize_;
    initialized_ = true;
    deviceIdChecked_ = false;
    deviceIdSupported_ = false;
    manufacturerId_ = 0;
    productId_ = 0;

    return FramI2C::ResultCode::Success;
}


void FramI2C::end(void)
{
    // Counterpart of begin().
    // Not required for most uses. Added for completeness.
    initialized_ = false;  
    if (typebuffer_ != nullptr)
    {
        // delete[] typebuffer_;
        free(typebuffer_);
    }
    typebufferSize_ = 0;
    i2cAddress_ = 0;
    density_ = 0;
    memorySize_ = 0;
    pageSize_ = 0;
    pageCount_ = 0;
    addressBytesCount_ = 0;
    deviceIdChecked_ = false;
    deviceIdSupported_ = false;
    manufacturerId_ = 0;
    productId_ = 0;    
}


uint8_t FramI2C::addressBytesCount(void) const
{
    return addressBytesCount_;
}


uint16_t FramI2C::density(void) const
{
    return density_;
}


uint8_t FramI2C::i2cAddress(void) const
{
    return i2cAddress_;
}


size_t FramI2C::memorySize(void) const
{
    return memorySize_;
}


size_t FramI2C::pageSize(void) const
{
    return pageSize_;
}


uint8_t FramI2C::pageCount(void) const
{
    return pageCount_;
}


size_t FramI2C::typebufferSize(void) const
{
    return typebufferSize_;
}


bool FramI2C::isInitialized(void) const
{
    return initialized_;
}


bool FramI2C::isDeviceIdSupported(void) const
{
    if (!deviceIdChecked_)
    {
        getDeviceId();
    }
    return deviceIdSupported_;
}


uint16_t FramI2C::manufacturerId(void) const
{
    if (!deviceIdChecked_)
    {
        getDeviceId();
    }
    return manufacturerId_;
}


uint16_t FramI2C::productId(void) const
{
    if (!deviceIdChecked_)
    {
        getDeviceId();
    }
    return productId_;
}


FramI2C::ResultCode FramI2C::readBytes(const uint16_t address, const size_t byteCount, uint8_t* const data) const
{
	// Overload without page parameter.
    return FramI2C::readBytes(0, address, byteCount, data);
}


FramI2C::ResultCode FramI2C::readBytes(const uint8_t page, const uint16_t address, const size_t byteCount, uint8_t* const data) const
{
    // Reads byteCount bytes from the specified FRAM page starting at memory address into data.
    // The Wire library's I2C buffer has a limited size of 32 bytes. When byteCount is larger
    // than the I2C buffer, data will automatically be transmitted in multiple smaller chunks.    
    // Data is read in chunks that are (max) 32 bytes in size.    
    //
    // The read operation performed is described in datasheets as 'selective address read' (because address is specified).

    if (!initialized_)
    {
        return FramI2C::ResultCode::NotInitializedError;
    }
    if (data == nullptr)
    {
        return FramI2C::ResultCode::NullPtrError;
    }
    if (page >= pageCount_)
    {
        return FramI2C::ResultCode::InvalidPageError;
    }
    if ((address >= pageSize_) || (address + byteCount) > pageSize_)
    {
        return FramI2C::ResultCode::PageSizeRangeError;
    }

    uint8_t pageI2cAddress = i2cAddress_ + page;
    uint8_t* dataChunk = data;
    uint16_t framChunkAddress = address;    
    size_t totalBytesRemaining = byteCount;

    while (totalBytesRemaining > 0)
    {
        size_t chunkSize = (totalBytesRemaining > I2CBufferLength) ? I2CBufferLength : totalBytesRemaining;

        size_t bytesQueued = 0;
        Wire.beginTransmission(pageI2cAddress);
        if (addressBytesCount_ > 1)
        {
            bytesQueued += Wire.write(framChunkAddress >> 8);
        }
        bytesQueued += Wire.write(framChunkAddress & 0xFF);
        if (bytesQueued != addressBytesCount_)
        {
            return FramI2C::ResultCode::I2CWriteError;
        }  
        uint8_t twiresult = Wire.endTransmission();
        if (twiresult != TwiSuccess)
        {
            return twiCodeToResultCode(twiresult);
        }

        // Read chunk from FRAM into I2C buffer.
        size_t bytesRead = Wire.requestFrom(pageI2cAddress, chunkSize, true);       
        if (bytesRead != chunkSize)
        {
            return FramI2C::ResultCode::I2CReadError;
        }

        // Copy chunk from I2C buffer to data.
        if (chunkSize == 1)  
        {
            dataChunk[0] = Wire.read();  //Is a tiny bit faster for single byte.
        }
        else
        {
            Wire.readBytes(dataChunk, chunkSize);
        }
        
        totalBytesRemaining -= chunkSize;
        framChunkAddress += chunkSize;
        dataChunk += chunkSize;
#if defined(ESP8266)        
        // If ESP8266 MCU yield() after reading every chunk to prevent WDT reset.
        yield();
#endif        
    }
    return FramI2C::ResultCode::Success;
}


FramI2C::ResultCode FramI2C::writeBytes(const uint16_t address, const size_t byteCount, const uint8_t* const data) const
{
	// Overload without page parameter.
    return writeBytes(0, address, byteCount, data);
}


FramI2C::ResultCode FramI2C::writeBytes(const uint8_t page, const uint16_t address, const size_t byteCount, const uint8_t* const data) const
{
    // Writes byteCount bytes from data to the specified FRAM page starting at memory address.
    // The Wire library's I2C buffer has a limited size of 32 bytes. When byteCount is larger
    // than the I2C buffer, data will automatically be transmitted in multiple smaller chunks.    
    // Data is written in chunks that are (max) 30 or 31 bytes in size.
    // 
    // When writing to FRAM the memory address needs to be included in the I2C buffer, therefore the
    // size of data that can be written per chunk is smaller than the size of the I2C buffer.
    // FRAM with 16-bit addressing uses 2 address bytes, FRAM with 8-bit addressing uses 1 addres byte.

    if(!initialized_)
    {
       return FramI2C::ResultCode::NotInitializedError;
    }  
    if(data == nullptr)
    {
        return FramI2C::ResultCode::NullPtrError;
    }
    if (page >= pageCount_)
    {
        return FramI2C::ResultCode::InvalidPageError;
    }
    if ((address >= pageSize_) || (address + byteCount) > pageSize_)
    {
        return FramI2C::ResultCode::PageSizeRangeError;
    }    

    uint8_t pageI2cAddress = i2cAddress_ + page;
    const uint8_t* dataChunk = data;
    uint16_t framChunkAddress = address;
    size_t totalBytesRemaining = byteCount;
    size_t i2cBufferUsableLength = I2CBufferLength - addressBytesCount_;
    ResultCode resultcode = ResultCode::Success;

    while(totalBytesRemaining > 0)
    {
        size_t chunkSize = (totalBytesRemaining > i2cBufferUsableLength) ? i2cBufferUsableLength : totalBytesRemaining;
        
        size_t bytesQueued = 0;
        Wire.beginTransmission(pageI2cAddress);
        if (addressBytesCount_ > 1)
        {
            bytesQueued += Wire.write(framChunkAddress >> 8);   //MSB
        }
        bytesQueued += Wire.write(framChunkAddress & 0xFF);     //LSB
        
        // Copy chunk from data to I2C buffer.      
        if (chunkSize == 1)
        {
            bytesQueued += Wire.write(dataChunk[0]);    //Is a tiny bit faster for single byte.
        }
        else
        {
            bytesQueued += Wire.write(dataChunk, chunkSize);
        }
        
        if (bytesQueued != addressBytesCount_ + chunkSize)
        {     
            return FramI2C::ResultCode::I2CWriteError;
        }  

        // Transmit I2C buffer to FRAM.
        uint8_t twiresult = Wire.endTransmission();
        if (twiresult != TwiSuccess)
        {
            return twiCodeToResultCode(twiresult);
        }
        
        totalBytesRemaining -= chunkSize;
        framChunkAddress += chunkSize;
        dataChunk += chunkSize;
#if defined(ESP8266)        
        // If ESP8266 MCU yield() after writing every chunk to prevent WDT reset.
        yield();
#endif           
    }
    return resultcode;
}


FramI2C::ResultCode FramI2C::fill(const uint16_t address, const size_t byteCount, const uint8_t value) const
{
	// Overload without page parameter.
    return fill(0, address, byteCount, value);
}


FramI2C::ResultCode FramI2C::fill(const uint8_t page, const uint16_t address, const size_t byteCount, const uint8_t value) const
{
    // Fills byteCount bytes of FRAM page starting at memory address, with value.
    // Uses a mechanism similar to writeBytes by writing data in chunks (which is faster).

    if(!initialized_)
    {
       return FramI2C::ResultCode::NotInitializedError;
    }  
    if (page >= pageCount_)
    {
        return FramI2C::ResultCode::InvalidPageError;
    }
    if ((address >= pageSize_) || (address + byteCount) > pageSize_)
    {
        return FramI2C::ResultCode::PageSizeRangeError;
    }    

    uint8_t pageI2cAddress = i2cAddress_ + page;
    uint16_t framChunkAddress = address;
    size_t totalBytesRemaining = byteCount;
    size_t i2cBufferUsableLength = I2CBufferLength - addressBytesCount_;
    size_t bytesQueued = 0;
    ResultCode resultcode = ResultCode::Success;

    while(totalBytesRemaining > 0)
    {
        size_t chunkSize = (totalBytesRemaining > i2cBufferUsableLength) ? i2cBufferUsableLength : totalBytesRemaining;
        
        bytesQueued = 0;
        Wire.beginTransmission(pageI2cAddress);
        if (addressBytesCount_ > 1)
        {
            bytesQueued += Wire.write(framChunkAddress >> 8);   //MSB
        }
        bytesQueued += Wire.write(framChunkAddress & 0xFF);     //LSB
        
        // Write chunkSize values to I2C buffer.      
        for (size_t i = 0; i < chunkSize; ++i)
        {
            bytesQueued += Wire.write(value);
        }
        if (bytesQueued != addressBytesCount_ + chunkSize)
        {     
            return FramI2C::ResultCode::I2CWriteError;
        }  

        // Transmit I2C buffer to FRAM.
        uint8_t twiresult = Wire.endTransmission();
        if (twiresult != TwiSuccess)
        {
            return twiCodeToResultCode(twiresult);
        }
        
        totalBytesRemaining -= chunkSize;
        framChunkAddress += chunkSize;
#if defined(ESP8266)        
        // If ESP8266 MCU yield() after writing every chunk to prevent WDT reset.
        yield();
#endif           
    }
    return resultcode;
}


bool FramI2C::getDeviceId(void) const
{
    // Reads the device id (if the FRAM that is used supports it).

    const uint8_t reservedSlaveAddress = 0x7C;  // See datasheets for information.
    const size_t deviceIdSize = 3;
    uint8_t deviceId[3];

    size_t bytesQueued = 0;
    Wire.beginTransmission(reservedSlaveAddress);
    bytesQueued += Wire.write(i2cAddress_ << 1);
    if (bytesQueued != 1)
    {
        return false;
    }  
    uint8_t twiresult = Wire.endTransmission(false);
    if (twiresult != TwiSuccess)
    {
        return false;
    }

    size_t bytesRead = Wire.requestFrom(reservedSlaveAddress, deviceIdSize, true);       
    if (bytesRead != deviceIdSize)
    {
        return false;
    }

    if (Wire.readBytes(deviceId, deviceIdSize) != deviceIdSize)
    {
        return false;
    }

    deviceIdSupported_ = true;
    
    // DEBUG: The following is temporary debug code for testing:
    // Serial.printf("Device ID = 0x%02X 0x%02X 0x%02X\n\n", deviceId[0], deviceId[1], deviceId[2]);

    // Manufacturer ID = Device ID bits 23-12,
    manufacturerId_ = (static_cast<uint16_t>(deviceId[0]) << 4) | (deviceId[1] >> 4);

    // Product ID = Device ID bits 11-0.
    productId_ = (static_cast<uint16_t>(deviceId[1] & 0x0F) << 8) | deviceId[2];

    // uint8_t densityCode = deviceId[1] & 0x0F;
    
    return true;
}

// --- Private ----------------------------------------------------------------

// The last element 0 denotes the end of the array, 0 is not a supported density.
const uint16_t FramI2C::SupportedDensitiesInKiloBits[] = {4, 16, 64, 128, 256, 512, 1024, 0}; 


size_t FramI2C::densityToMemorySize(const uint16_t density) const
{
    if (!isDensitySupported(density))
    {
        return 0;
    } 

    size_t memorySize = density * 1024 / 8;
    return memorySize;
}


size_t FramI2C::densityToPageSize(const uint16_t density) const
{
    if (!isDensitySupported(density))
    {
        return 0;
    }

    size_t pageSize = 0;  
    if (density <= 16)
    {
        pageSize = 0x100;   // Densities 4 and 16.
    }
    else if (density <= 256)
    {
        pageSize = densityToMemorySize(density);  // Densities 64, 128 and 256.      
    }
    else
    {  
        pageSize = 0x10000;  // Densities 512 and 1024.         
    }
    return pageSize;
}


bool FramI2C::isDensitySupported(const uint16_t densityInKiloBits) const
{
    bool isSupported = false;
    uint8_t i = 0;
    while (FramI2C::SupportedDensitiesInKiloBits[i] != 0)
    {
        if (densityInKiloBits == SupportedDensitiesInKiloBits[i])
        {
            isSupported = true;
            break;
        }
        ++i;
    }
    return isSupported;
}


FramI2C::ResultCode FramI2C::twiCodeToResultCode(const uint8_t twiCode) const
{
    ResultCode retval = FramI2C::ResultCode::I2CUnknownTwiResultCode;
    switch (twiCode)
    {
        case FramI2C::TwiSuccess:
            retval = FramI2C::ResultCode::Success;
            break;
        case FramI2C::TwiAddressNack:
            retval = FramI2C::ResultCode::I2CAddressNackError;
            break;
        case FramI2C::TwiDataNack:
            retval = FramI2C::ResultCode::I2CDataNackError;
            break;
        case FramI2C::TwiLineBusy:
            retval = FramI2C::ResultCode::I2CLineBusyError;
            break;
    }
    return retval;
}


/* eof */
