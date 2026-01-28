#include <iostream>

#include "MemoryDriver.h"

class GpioSpiDriver : public ISpiDriver
{
public:

   void chipSelect(bool active) override
   {
      if (active)
      {
         std::cout << "[SPI] CS: ACTIVE" << std::endl;
      }
      else
      {
         std::cout << "[SPI] CS: INACTIVE" << std::endl;
      }
   }

   uint8_t transfer(uint8_t data) override
   {
      std::cout << "[SPI] Transfer: 0x" << std::hex << (int)data
         << " -> 0x" << (int)(~data) << std::dec << std::endl;
      return ~data;
   }

   void delayUs(uint32_t microseconds) override
   {
      std::cout << "[SPI] Delay: " << microseconds << "us" << std::endl;
   }
};

int main() {
   std::cout << "=== Testing SPI Memory Drivers ===\n" << std::endl;

   // Create SPI driver
   GpioSpiDriver spiDriver;

   std::cout << "1. Testing EEPROM 25LC040A: " << std::endl;
   std::cout << "=================================" << std::endl;

   EEPROM_25LC040A eeprom(&spiDriver);

   // Write and read byte
   eeprom.writeByte(0x100, 0xAB);
   uint8_t data = eeprom.readByte(0x100);
   std::cout << "Read back: 0x" << std::hex << (int)data << std::dec << std::endl;

   // Bit operations
   eeprom.writeByte(0x200, 0x00);
   eeprom.setBit(0x200, 3);
   data = eeprom.readByte(0x200);
   std::cout << "After setBit(3): 0x" << std::hex << (int)data << std::dec << std::endl;

   eeprom.clearBit(0x200, 3);
   data = eeprom.readByte(0x200);
   std::cout << "After clearBit(3): 0x" << std::hex << (int)data << std::dec << std::endl;

   std::cout << "\n2. Testing NOR W25Q128:" << std::endl;
   std::cout << "====================================" << std::endl;

   NOR_W25Q128 nor(&spiDriver);

   // For NOR different operations are needed
   nor.eraseSector(0x1000);
   nor.writeByte(0x1000, 0x55);
   data = nor.readByte(0x1000);
   std::cout << "Flash read: 0x" << std::hex << (int)data << std::dec << std::endl;

   std::cout << "\n=== Testing completed ===" << std::endl;

   std::cin.get();

   return 0;
}