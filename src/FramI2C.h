/* FramI2C.h
 *
 * Description:  FramI2C is a driver for I2C nonvolatile Ferroelectric RAM (F-RAM, FRAM, FeRAM) memory.
 *               It supports Cypress and Fujitsu I2C FRAM chips with densities from 4kb to 1Mb (bits)
 * 
 * Author:       Leonel Lopes Parente
 * 
 * License:      MIT (see LICENSE file in repository root)
 * 
 */


#ifndef FRAMI2C_H_
#define FRAMI2C_H_

#include <Arduino.h>


class FramI2C 
{

private:
    // Values returned by Wire library
    enum TwiResultCode
    {
        TwiSuccess = 0,
        TwiBufferOverflow = 1,
        TwiAddressNack = 2,
        TwiDataNack = 3,
        TwiLineBusy = 4        
    };


public:

    enum class ResultCode : uint8_t 
    {
        Success = 0, 
        I2CBufferOverflowError = TwiBufferOverflow, 
        I2CAddressNackError = TwiAddressNack,
        I2CDataNackError = TwiDataNack, 
        I2CLineBusyError = TwiLineBusy,
        I2CReadError = 0xC0,
        I2CWriteError = 0xC1,
        I2CUnknownTwiResultCode = 0xC2,
        NullPtrError = 0xE0, 
        NotInitializedError = 0xE1,
        AllreadyInitializedError = 0xE2,
        UnsupportedDensityError = 0xE3,
        InvalidPageError = 0xE4,
        PageSizeRangeError = 0xE5,
        BufferAllocationFailedError = 0xE6, 
        BufferOverflowError = 0xE7,
        Uninitialized = 0xFF
    };

    FramI2C();
    ~FramI2C();

	ResultCode begin(const uint16_t densityInKiloBits, const uint8_t i2cAddress = DefaultI2CAddress, const size_t typebufferSize = DefaultTypeBufferSize);
	void end(void);

    uint16_t density(void) const;
    uint8_t i2cAddress(void) const;
    size_t memorySize(void) const;
    uint8_t addressBytesCount(void) const;
    size_t pageSize(void) const;
    uint8_t pageCount(void) const;
    size_t typebufferSize(void) const; 
    bool isInitialized(void) const;
    bool isDeviceIdSupported(void) const;
    uint16_t manufacturerId(void) const;
    uint16_t productId(void) const;

    ResultCode readBytes(const uint16_t address, const size_t byteCount, uint8_t* const data) const;
    ResultCode readBytes(const uint8_t page, const uint16_t address, const size_t byteCount, uint8_t* const data) const;       

    ResultCode writeBytes(const uint16_t address, const size_t byteCount, const uint8_t* const data) const;
    ResultCode writeBytes(const uint8_t page, const uint16_t address, const size_t byteCount, const uint8_t* const data) const;

    ResultCode fill(const uint16_t address, const size_t byteCount, const uint8_t value) const;
    ResultCode fill(const uint8_t page, const uint16_t address, const size_t byteCount, const uint8_t value) const;

    // ResultCode sleep(void) const;


    template<typename T> ResultCode read(const uint8_t page, const uint16_t address, T& t) 
    {
        // Generic read method.
        // Sets value of output parameter t to value read from FRAM(page, address).
        // If the read fails t is unchanged.
        // With the default type buffer size (10) read() can be used for integral and floating point types.
        // For use with other, larger types the type buffer size must be increased to at least sizeof(t). 
        // Example usage:
        //   double storedTemperature = 0.0;
        //   read(0, 0, storedTemperature);      

        if (sizeof(T) > typebufferSize_)
        {
            return FramI2C::ResultCode::BufferOverflowError;
        }  

        ResultCode resultcode = readBytes(page, address, sizeof(T), typebuffer_);
        if (resultcode == FramI2C::ResultCode::Success)
        {
            memcpy((uint8_t*) &t, typebuffer_, sizeof(T));
        }
        return resultcode;
    }    


    template<typename T> ResultCode read(const uint16_t address, const T& t) 
    {
        // Overload without page parameter, uses page 0.
        return read(0, address, t);
    }        


    template<typename T> ResultCode write(const uint8_t page, const uint16_t address, const T& t) 
    {
        // Generic write method.
        // Writes input parameter t's value to FRAM(page, address).
        // With the default type buffer size (10) write() can be used for integral and floating point types.
        // For use with other, larger types the type buffer size must be increased to at least sizeof(t). 
        // Example usage:
        //   double temperature = 21.3;
        //   write(0, 0, temperature);        

        if (sizeof(T) > typebufferSize_)
        {
            return FramI2C::ResultCode::BufferOverflowError;
        }

        memcpy(typebuffer_, (const uint8_t*)&t, sizeof(T));
        return writeBytes(page, address, sizeof(T), typebuffer_);
    }    


    template<typename T> ResultCode write(const uint16_t address, const T& t) 
    {
        // Overload without page parameter, uses page 0.
        return write(0, address, t);
    }    


private:

    static const uint8_t DefaultI2CAddress = 0x50;
    // A 10 byte type buffer is sufficient for all integral and floating point types (max 8 bytes).
    static const size_t DefaultTypeBufferSize = 10;
    static const size_t I2CBufferLength = 32;   // Hardcoded in Arduino Wire library.
    static const uint16_t SupportedDensitiesInKiloBits[];

    uint16_t density_ = 0;
    uint8_t i2cAddress_ = 0;
    size_t memorySize_ = 0;
    size_t pageSize_ = 0;
    uint8_t pageCount_ = 0;
    uint8_t addressBytesCount_ = 0;
    size_t typebufferSize_ = 0;
    uint8_t* typebuffer_ = nullptr;
    
    bool initialized_ = false;
    mutable bool deviceIdChecked_ = false;
    mutable bool deviceIdSupported_ = false;
    mutable uint16_t manufacturerId_ = 0;
    mutable uint16_t productId_ = 0;

    size_t densityToMemorySize(const uint16_t densityInKiloBits) const;
    size_t densityToPageSize(const uint16_t density) const;
    bool getDeviceId(void) const;    
    bool isDensitySupported(const uint16_t densityInKiloBits) const;
    ResultCode twiCodeToResultCode(const uint8_t twiCode) const;
};

#endif  //FRAMI2C_H_



