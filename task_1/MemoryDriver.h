#pragma once

#include <cstdint>
#include <cstddef>

/**
* @class ISpiDriver
* @brief Abstract interface for low-level SPI driver
*/
class ISpiDriver 
{
public:
	virtual ~ISpiDriver() = default;

	/**
	* @brief Activate Chip Select line
	*/
	virtual void chipSelect(bool active) = 0;

	/**
	* @brief Transfer byte over SPI
	* @param data Byte to transfer
	* @return Received byte
	*/
	virtual uint8_t transfer(uint8_t data) = 0;

	/**
	* @brief Delay in microseconds
	*/
	virtual void delayUs(uint32_t microseconds) = 0;
};

/**
* @class EEPROM_25LC040A
* @brief Driver for 25LC040A EEPROM memory
*/
class EEPROM_25LC040A
{
public:

	/**
	* @brief Constructor
	* @param spi Pointer to SPI driver
	*/
	explicit EEPROM_25LC040A(ISpiDriver* spi);

	/**
	* @brief Read byte
	* @param address Address (0-511)
	* @return Read byte
	*/
	uint8_t readByte(uint16_t address);

	/**
	* @brief Write byte
	* @param address Address (0-511)
	* @param data Byte to write
	*/
	void writeByte(uint16_t address, uint8_t data);

	/**
	* @brief Read byte array
	* @param address Start address
	* @param buffer Data buffer
	* @param length Number of bytes
	*/
	void readArray(uint16_t address, uint8_t *buffer, size_t length);

	/**
	* @brief Write byte array
	* @param address Start address
	* @param data Data to write
	* @param length Number of bytes
	*/
	void writeArray(uint16_t address, const uint8_t *data, size_t length);

	/**
	* @brief Set bit
	* @param address Byte address
	* @param bit Bit number (0-7)
	*/
	void setBit(uint16_t address, uint8_t bit);

	/**
	* @brief Clear bit in byte
	* @param address Byte address
	* @param bit Bit number (0-7)
	*/
	void clearBit(uint16_t address, uint8_t bit);

	/**
	* @brief Get memory size
	* @return Size in bytes
	*/
	static constexpr size_t getSize() { return 512; }

private:

	enum Command : uint8_t
	{
		CMD_READ  = 0x03,
		CMD_WRITE = 0x02,
		CMD_WREN  = 0x06,
		CMD_RDSR  = 0x05,
	};

	ISpiDriver *spi;

	/**
	* @brief Wait for write completion
	*/
	void waitForWriteComlete();

	/**
	* @brief Enable write mode
	*/
	void writeEnable();
};

/**
* @class NOR_W25Q128
* @brief Driver for NOR memory W25Q128
*/
class	NOR_W25Q128
{
public:

	/**
	* @brief Constructor
	* @param spi Pointer to SPI driver
	*/
	explicit NOR_W25Q128(ISpiDriver *spi);

	/**
	* @brief Read byte
	* @param address Address (24-bit)
	* @return Read byte
	*/
	uint8_t readByte(uint32_t address);

	/**
	* @brief Write byte
	* @param address Address
	* @param data Byte to write
	* @note Requires prior sector erase
	*/
	void writeByte(uint32_t address, uint8_t data);

	/**
	* @brief Read byte array
	* @param address Start address
	* @param buffer Data buffer
	* @param length Number of bytes
	*/
	void readArray(uint32_t address, uint8_t *buffer, size_t length);

	/**
	* @brief Write byte array
	* @param address Start address
	* @param data Data to write
	* @param length Number of bytes
	*/
	void writeArray(uint32_t address, const uint8_t *data, size_t length);

	/**
	* @brief Set bit
	* @param address Byte address
	* @param bit Bit number (0-7)
	*/
	void setBit(uint32_t address, uint8_t bit);

	/**
	* @brief Clear bit
	* @param address Byte address
	* @param bit Bit number (0-7)
	*/
	void clearBit(uint32_t address, uint8_t bit);

	/**
	* @brief Erase sector (4KB)
	* @param address Address within sector
	*/
	void eraseSector(uint32_t address);

	/**
	* @brief Get memory size
	* @return Size in bytes
	*/
	static constexpr size_t getSize() { return 16 * 1024 * 1024; }

private:

	enum Command : uint8_t
	{
		CMD_READ  = 0x03,
		CMD_WRITE = 0x02,
		CMD_WREN  = 0x06,
		CMD_RDSR1 = 0x05,
		CMD_SECTOR_ERASE = 0x20,
	};

	static constexpr uint32_t SECTOR_SIZE = 4096;
	static constexpr uint32_t PAGE_SIZE = 256;

	ISpiDriver *spi;

	/**
	* @brief Wait for operation completion
	*/
	void waitForReady();

	/**
	* @brief Enable write mode
	*/
	void writeEnable();

	/**
	* @brief Get sector start by address
	*/
	uint32_t getSectorStart(uint32_t address) const;
};