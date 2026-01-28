#include <cstring>

#include "MemoryDriver.h"

// ============================================================================
// EEPROM_25LC040A Implementation
// ============================================================================

EEPROM_25LC040A::EEPROM_25LC040A(ISpiDriver *spi)
	:	spi(spi)
{
}

uint8_t EEPROM_25LC040A::readByte(uint16_t address)
{
	if (address >= getSize())
	{
		return 0xFF;
	}

	spi->chipSelect(true);

	// Send READ command with address
	spi->transfer(CMD_READ);
	spi->transfer(static_cast<uint8_t>(address >> 8));
	spi->transfer(static_cast<uint8_t>(address & 0xFF));

	// Read data
	uint8_t data = spi->transfer(0x00);

	spi->chipSelect(false);

	return data;
}

void EEPROM_25LC040A::writeByte(uint16_t address, uint8_t data)
{
	if (address >= getSize()) return;

	// Enable write
	writeEnable();

	spi->chipSelect(true);

	// Send WRITE command with address and data
	spi->transfer(CMD_WRITE);
	spi->transfer(static_cast<uint8_t>(address >> 8));
	spi->transfer(static_cast<uint8_t>(address & 0xFF));
	spi->transfer(data);

	spi->chipSelect(false);

	// Wait for write completion
	waitForWriteComlete();
}

void EEPROM_25LC040A::readArray(uint16_t address, uint8_t *buffer, size_t length)
{
	if (buffer == nullptr || address + length > getSize()) return;

	spi->chipSelect(true);

	// READ command with address
	spi->transfer(CMD_READ);
	spi->transfer(static_cast<uint8_t>(address >> 8));
	spi->transfer(static_cast<uint8_t>(address & 0xFF));

	// Sequential read
	for (size_t i = 0; i < length; i++)
	{
		buffer[i] = spi->transfer(0x00);
	}

	spi->chipSelect(false);
}

void EEPROM_25LC040A::writeArray(uint16_t address, const uint8_t *data, size_t length)
{
	if (data == nullptr || address + length > getSize()) return;

	// EEPROM has page write limitation of 16 bytes
	const uint16_t PAGE_SIZE = 16;

	size_t written = 0;
	while (written < length)
	{
		// Write up to 16 bytes at a time
		size_t chunk;
		if (length - written > PAGE_SIZE)
		{
			chunk = PAGE_SIZE;
		}
		else
		{
			chunk = length - written;
		}

		// Enable write for each chunk
		writeEnable();

		spi->chipSelect(true);

		uint16_t currentAddr = address + written;
		spi->transfer(CMD_WRITE);
		spi->transfer(static_cast<uint8_t>(currentAddr >> 8));
		spi->transfer(static_cast<uint8_t>(currentAddr & 0xFF));

		// Transfer chunk data
		for (size_t i = 0; i < chunk; i++)
		{
			spi->transfer(data[written + i]);
		}

		spi->chipSelect(false);

		// Wait for chunk write completion
		waitForWriteComlete();

		written += chunk;
	}
}

void EEPROM_25LC040A::setBit(uint16_t address, uint8_t bit)
{
	if (bit > 7 || address >= getSize()) return;

	// Set bit
	uint8_t value = readByte(address);
	value |= (1 << bit);
	writeByte(address, value);
}

void EEPROM_25LC040A::clearBit(uint16_t address, uint8_t bit)
{
	if (bit > 7 || address >= getSize()) return;

	// Clear bit
	uint8_t value = readByte(address);
	value &= ~(1 << bit);
	writeByte(address, value);
}

void EEPROM_25LC040A::waitForWriteComlete()
{
	// Poll status until write completes
	do
	{
		spi->chipSelect(true);
		spi->transfer(CMD_RDSR);
		uint8_t status = spi->transfer(0x00);
		spi->chipSelect(false);

		if ((status & 0x01) == 0)
		{
			break;
		}

		// Short delay between polls
		spi->delayUs(100);
	} while (true);
}

void EEPROM_25LC040A::writeEnable()
{
	spi->chipSelect(true);
	spi->transfer(CMD_WREN);
	spi->chipSelect(false);
	spi->delayUs(1);
}

// ============================================================================
// NOR_W25Q128 Implementation
// ============================================================================

NOR_W25Q128::NOR_W25Q128(ISpiDriver *spi)
	:	spi(spi)
{
}

uint8_t NOR_W25Q128::readByte(uint32_t address)
{
	if (address >= getSize()) return 0xFF;

	spi->chipSelect(true);

	// 24-bit address for NOR
	spi->transfer(CMD_READ);
	spi->transfer(static_cast<uint8_t>(address >> 16));
	spi->transfer(static_cast<uint8_t>(address >> 8));
	spi->transfer(static_cast<uint8_t>(address & 0xFF));

	uint8_t data = spi->transfer(0x00);

	spi->chipSelect(false);

	return data;
}

void NOR_W25Q128::writeByte(uint32_t address, uint8_t data)
{
	if (address >= getSize()) return;

	// For NOR prior sector erase is required
	// Need to read entire sector, modify byte and rewrite

	// Simplified version - write only if clearing bits
	uint8_t current = readByte(address);
	// Can write without erase
	if ((current & data) == data)
	{
		writeEnable();

		spi->chipSelect(true);
		spi->transfer(CMD_WRITE);
		spi->transfer(static_cast<uint8_t>(address >> 16));
		spi->transfer(static_cast<uint8_t>(address >> 8));
		spi->transfer(static_cast<uint8_t>(address & 0xFF));
		spi->transfer(data);
		spi->chipSelect(false);

		waitForReady();
	}
	else
	{
		// Sector erase required
	}
}

void NOR_W25Q128::readArray(uint32_t address, uint8_t *buffer, size_t length)
{
	if (buffer == nullptr || address + length > getSize()) return;

	spi->chipSelect(true);
	spi->transfer(CMD_READ);
	spi->transfer(static_cast<uint8_t>(address >> 16));
	spi->transfer(static_cast<uint8_t>(address >> 8));
	spi->transfer(static_cast<uint8_t>(address & 0xFF));

	for (size_t i = 0; i < length; i++)
	{
		buffer[i] = spi->transfer(0x00);
	}

	spi->chipSelect(false);
}

void NOR_W25Q128::writeArray(uint32_t address, const uint8_t *data, size_t length)
{
	if (data == nullptr || address + length > getSize()) return;

	// NOR requires page write of 256 bytes
	size_t written = 0;
	while (written < length)
	{
		// Don't cross page boundary
		uint32_t pageStart = address + written;
		uint32_t pageOffset = pageStart % PAGE_SIZE;
		size_t chunk =	PAGE_SIZE - pageOffset;
		if (chunk > length - written)
		{
			chunk = length - written;
		}

		writeEnable();

		spi->chipSelect(true);
		spi->transfer(CMD_WRITE);
		spi->transfer(static_cast<uint8_t>(pageStart >> 16));
		spi->transfer(static_cast<uint8_t>(pageStart >> 8));
		spi->transfer(static_cast<uint8_t>(pageStart & 0xFF));

		for (size_t i = 0; i < chunk; i++)
		{
			spi->transfer(data[written + i]);
		}

		spi->chipSelect(false);
		waitForReady();

		written += chunk;
	}
}

void NOR_W25Q128::setBit(uint32_t address, uint8_t bit)
{
	if (bit > 7 || address >= getSize()) return;

	// In NOR cannot set bit (0->1), can only clear (1->0)
	// To set bit need to erase sector and rewrite
}

void NOR_W25Q128::clearBit(uint32_t address, uint8_t bit)
{
	if (bit > 7 || address >= getSize()) return;

	uint8_t value = readByte(address);
	// Clear bit
	value &= ~(1 << bit);
	// Write (if possible)
	writeByte(address, value);
}

void NOR_W25Q128::eraseSector(uint32_t address)
{
	if (address >= getSize()) return;

	uint32_t sectorStart = getSectorStart(address);

	writeEnable();

	spi->chipSelect(true);
	spi->transfer(CMD_SECTOR_ERASE);
	spi->transfer(static_cast<uint8_t>(sectorStart >> 16));
	spi->transfer(static_cast<uint8_t>(sectorStart >> 8));
	spi->transfer(static_cast<uint8_t>(sectorStart & 0xFF));
	spi->chipSelect(false);

	waitForReady();
}

void NOR_W25Q128::waitForReady()
{
	do
	{
		spi->chipSelect(true);
		spi->transfer(CMD_RDSR1);
		uint8_t status = spi->transfer(0x00);
		spi->chipSelect(false);

		// BUSY bit
		if ((status & 0x01) == 0)
		{
			break;
		}

		spi->delayUs(100);
	} while (true);
}

void NOR_W25Q128::writeEnable()
{
	spi->chipSelect(true);
	spi->transfer(CMD_WREN);
	spi->chipSelect(false);
	spi->delayUs(1);
}

uint32_t NOR_W25Q128::getSectorStart(uint32_t address) const
{
	return (address / SECTOR_SIZE) * SECTOR_SIZE;
}