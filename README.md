# FramI2C

FramI2C is an Arduino library for FRAM (F-FRAM, Ferroelectric RAM) non-volatile memory chips with I2C interface.
- Supports most common Cypress and Fujitsu I2C FRAM chips with densities of 4, 16, 64, 128, 256, 512, and 1024 kilobits (kb).
- Provides simple, easy to use `read()` and `write()` methods for reading/writing integral and floating point types (uses automatic type inference and byte conversion), `readBytes()` and `writeBytes()` for reading/writing larger amounts of data as byte array, and `fill()` to fill or clear a range of FRAM memory.
- If an FRAM chip contains multiple memory pages, memory access is handled per page. The user only needs to specify a specific page number. The underlying complexity of how how this needs to be translated to different I2C addresses is hidden from the user.
<br>

## Introduction to FRAM
FRAM, F-RAM or FeRAM (Ferroelectric Random Access Memory) is nonvolatile RAM memory that combines the advantages of both RAM and nonvolatile memory like Flash and EEPROM. It is very suitable for storing data and settings in low-power applications. FRAM is available with different interfaces (I2C, SPI and parallel). This library is for FRAM with I2C interface. <br>
FRAM has the following benefits when compared to Flash and EEPROM:
* Fast symmetrical read and write times (access time < 100 ns).  
(With I2C FRAM the speed is limited/determined by the speed of the I2C bus.)
* Random access at byte level for both reads and writes.
* No erase required before writes.
* Low power consumption (and energy required for write is same as for read).
* Very High Write Endurance (10<sup>14</sup> write cycles).  
Which is almost indefinite when compared to Flash (10<sup>5</sup> for NOR Flash) and EEPROM.

I2C FRAM chips (8-pin) are currently available in sizes from 4 kb (0.5 kB) to 1 Mb (128 kB). The size is called density and is expressed in kilobits. The default I2C address of FRAM chips is 0x50. A maximum of 3 I2C address selection pins is available per chip which allows to set the I2C address in the range from 0x50 to 0x57. A maximum of 8 FRAM chips can be used on the same physical I2C bus. Note that Serial Flash chips use the same I2C address range.<br>
For some FRAM chips the adressable memory is split up in multiple pages. Whether a FRAM chip has multiple pages or not is determined by its density (memory size) and if it uses 8-bit or 16-bit memory addressing. The chips with smallest densities use multiple 256 byte pages (8-bit addressing). The larger ones use 16-bit addressing and have a single page with size equal to memory size (density) up to a maximum of 64 kB, except for chips with 1 Mb density which have 2 pages of 64 kB.
Selection of a page is done via one or more bits of the I2C address. This means that each page is available on a different I2C address. FRAM chips with multiple pages therefore will have fewer I2C address selection pins. When using FRAM's with multiple pages the maximum number of FRAM chips that can be connected to the same I2C bus will be reduced. *(FRAM chips with 16 kb density are the worst in this aspect because they use 8 pages of 256 bytes in size and uses all 8 available I2C adresses, which means that only one FRAM chip and no serial Flash can be connected to the same I2C bus.)*<br>
<br>
Example:<br>
A Cypress FM24V10 1 Mb (128 kB) FRAM chip has 2 memory pages of 64 kB each. Each page is available via a different I2C address. The default base I2C address is 0x50. The first page is available on I2C address 0x50 and the second page is available on I2C address 0x51.<br>

The library hides the complexity of I2C page addressing from the user and takes care of this automatically. The user only has to specify a page number if a chip has multiple pages and does not need to specify a page number when there is only a single page.<br>
<br>

## The FramI2C library

To use the library the following include statement is needed:

```cpp
#include "FramI2C.h"
```

In addition an instance of the FramI2C class must be created (without any parameters).<br>
The FramI2C instance is configured by specifying parameters in a call to the `FramI2C.begin()` method.<br>
*When using multiple FRAM chips, a separate instance must be created for each chip.* 

```cpp
FramI2C fram;
```

The `fram` instance needs to be initialized before it can be used. This is done by calling `fram.begin()`. FramI2C needs to access the I2C bus and uses the Wire library for this. The I2C bus is a shared resource that should therefore be initialized by the application (and not by libraries). The application must therefore #include Wire.h and call `Wire.begin()`. `Wire.begin()` must be called before `FramI2C.begin()` is called.<br>
`FramI2C.begin()` supports multiple parameters. At least one parameter has to be specified: the density of the FRAM chip in kilobits. The other parameters are optional and have default values. Specifying the correct density is essential for proper operation. While begin() will check if the density is supported, it cannot check if the specified density matches the actual density of the FRAM chip.<br>
For a chip with density 64 kb (with default I2C address 0x50) the FramI2C instance can be initialized in setup() as follows:

```cpp
void setup()
{
    Wire.begin();
    fram.begin(64);
}
```

Below is a complete basic example of how FramI2C can be used:

```cpp
#include <Arduino.h>
#include <Wire.h>
#include "FramI2C.h"

FramI2C fram;

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    fram.begin(64); // Specifying the correct FRAM density is essential for proper operation
    
    // To prevent unnecessary FRAM writes the example runs only once in setup()
    // instead of running endlessly in loop().
    
    uint32_t stored = 0xBABE;
    uint32_t restored;
    
    fram.write(0, stored);  // Write value of stored to FRAM memory address 0
    fram.read(0, restored); // Read value from FRAM memory address 0 into restored
    Serial.print("Value of restored is: 0x");
    Serial.print(restored, HEX);    
}

void loop()
{
    // Empty
}
```
<br>

*Under construction. More documentation will be added.*
