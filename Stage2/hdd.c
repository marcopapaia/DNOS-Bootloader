/*
* License: Public Domain
*/


#include "hdd.h"
#include <stdint.h>
#include <stdbool.h>
#include "io.h"

#include "display.h"

#define HDD_REG_Data 0
#define HDD_REG_FeaturesInfo 1
#define HDD_REG_SectorCount 2
#define HDD_REG_LBAlo 3
#define HDD_REG_LBAmid	4
#define HDD_REG_LBAhi	5
#define HDD_REG_DriveSelect	6
#define HDD_REG_commandStatus 7

#define HDD_REG_PRIMARY_BUS_CONTROL		0x3F6
#define HDD_REG_PRIMARY_BUS_BASE		0x1F0

#define HDD_REG_SECONDARY_BUS_CONTROL	0x376
#define HDD_REG_SECONDARY_BUS_BASE		0x170

bool hdd_check_device(unsigned drive);

extern volatile uint32_t timerTicks;

struct HDD_INFO hdd_devices[HDD_MAX_DEVICES];

void hdd_detect_devices()
{
	for(unsigned i = 0; i < sizeof(hdd_devices); i++){
		((char *)&hdd_devices)[i] = 0;
	}
	
	for(int i = 0; i < HDD_MAX_DEVICES; i++){
		hdd_check_device(i);
	}
}

void hdd_soft_reset()
{
	outb(HDD_REG_PRIMARY_BUS_CONTROL, inb(HDD_REG_PRIMARY_BUS_CONTROL) | 0x2);
	outb(HDD_REG_PRIMARY_BUS_CONTROL, inb(HDD_REG_PRIMARY_BUS_CONTROL) & ~(0x2));

	outb(HDD_REG_SECONDARY_BUS_CONTROL, inb(HDD_REG_SECONDARY_BUS_CONTROL) | 0x2);
	outb(HDD_REG_SECONDARY_BUS_CONTROL, inb(HDD_REG_SECONDARY_BUS_CONTROL) & ~(0x2));
}

bool hdd_check_device(unsigned drive)
{
	hdd_devices[drive].exists = false;
	hdd_soft_reset();

	uint16_t regBase = (drive > 1) ? HDD_REG_SECONDARY_BUS_BASE : HDD_REG_PRIMARY_BUS_BASE;

	// Send Disk Select Command
	outb(regBase + HDD_REG_DriveSelect, (drive % 2 == 1 ? 0xB0 : 0xA0));

	outb(regBase + HDD_REG_SectorCount, 0);
	outb(regBase + HDD_REG_LBAlo, 0);
	outb(regBase + HDD_REG_LBAmid, 0);
	outb(regBase + HDD_REG_LBAhi, 0);

	outb(regBase + HDD_REG_commandStatus, 0xEC);// IDENTIFY command

	uint8_t stat = inb(regBase + HDD_REG_commandStatus);// Read status port

	if (stat == 0) return false;

	uint32_t endTime = timerTicks + 2000;
	while (timerTicks < endTime){
		uint8_t b = inb(regBase + HDD_REG_commandStatus);
		if ((b & 0x1) != 0){// ERR
			return false;
		}
		if ((b & 0x80) == 0){
			break;
		}
	}

	if (inb(regBase + HDD_REG_LBAmid) != 0) return false;
	if (inb(regBase + HDD_REG_LBAhi) != 0) return false;

	endTime = timerTicks + 2000;
	while (timerTicks < endTime){
		uint8_t b = inb(regBase + HDD_REG_commandStatus);
		if ((b & 0x1) != 0){// ERROR
			return false;
		}
		if ((b & 0x08) != 0){// DRQ
			break;
		}
	}

	// We are here so there has been no error

	uint16_t data[256];
	for (int i = 0; i < 256; i++){
		data[i] = inw(regBase);
	}

	if ((data[83] & 0x400) != 0){
		hdd_devices[drive].lba48 = true;
		hdd_devices[drive].sectorCount = *((uint32_t *)&data[100]);
	}
	else {
		hdd_devices[drive].lba48 = false;
		hdd_devices[drive].sectorCount = *(&data[60]);
	}

	hdd_devices[drive].exists = true;
	return true;
}

// Limited to 255 sectors max.
int hdd_read_sectors(uint16_t * readTo, uint16_t sectors, unsigned drive, uint32_t startSector){
	if (sectors > 255 || sectors == 0){
		return 1;
	}
	
	// Going over the hard drives capacity? Nope.
	if ((startSector + sectors) > hdd_devices[drive].sectorCount) return 1;
	
	uint16_t reg_base = (drive > 1) ? HDD_REG_SECONDARY_BUS_BASE : HDD_REG_PRIMARY_BUS_BASE;
	
	if (!hdd_devices[drive].lba48 || (startSector + sectors) < 268435455){
		
		if (drive % 2 != 0)
			outb(reg_base + HDD_REG_DriveSelect, 0xE0 | (1 << 4) | ((startSector >> 24) & 0x0F));
		else
			outb(reg_base + HDD_REG_DriveSelect, 0xE0 | ((startSector >> 24) & 0x0F));
		//outb(HDD_REG_PRIMARY_BUS_BASE + HDD_REG_FeaturesInfo, 0x00);// Optional


		//if (sectors == 256) sectors = 0;
		outb(reg_base + HDD_REG_SectorCount, (unsigned char)sectors);

		outb(reg_base + HDD_REG_LBAlo, (unsigned char)startSector);
		outb(reg_base + HDD_REG_LBAmid, (unsigned char)(startSector >> 8));
		outb(reg_base + HDD_REG_LBAhi, (unsigned char)(startSector >> 16));

		outb(reg_base + HDD_REG_commandStatus, 0x20);// READ

		// Initial wait
		uint32_t endTime = timerTicks + 30000;// 30 secs for drive spin up
		for (;;){
			if (timerTicks >= endTime){
				return 1;// Can we correct this read failure?
			}
			uint8_t b = inb(reg_base + HDD_REG_commandStatus);
			if ((b & 0x80) != 0){//  BUSY set
				continue;
			}

			// not busy

			if ((b & 0x8) != 0){// DRQ set
				break;
			}
		}

		for (int s = 0; s < sectors; s++){
			for (int i = 0; i < 256; i++){
				readTo[(s * 256) + i] = inw(reg_base);
			}

			if (s >= (sectors - 1)) break;

			// 400ns delay
			inb(reg_base);
			inb(reg_base);
			inb(reg_base);
			inb(reg_base);

			endTime = timerTicks + 2000;
			for (;;){
				if (timerTicks >= endTime){
					return 1;
				}
				uint8_t b = inb(reg_base + HDD_REG_commandStatus);
				if ((b & 0x80) != 0){//  BUSY
					continue;
				}

				// not busy

				if ((b & 0x21) != 0){// ERR or DF set
					return 1;
				}

				// not busy and error not set and DF not set

				break;
			}

		}

		return 0;
	}
	else {// lba48
		// TODO
		return 1;
		// http://wiki.osdev.org/ATA_PIO_Mode#48_bit_PIO

	}

	return 1;
}

