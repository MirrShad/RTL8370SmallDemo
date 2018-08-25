#ifndef __STMFLASH_H__
#define __STMFLASH_H__ 

#include <stdint.h>

//FLASH起始地址
#define STM32_FLASH_BASE 	0x08000000 	//STM32 FLASH的起始地址
#define FLASH_APP1_ADDR		0x08020000

#define BOOT_PARAM_APP	 	0x5555AAAA
#define BOOT_PARAM_BL			0x2b2b6666
#define BOOT_PARAM_LOADING 0x66668888 

//FLASH 扇区的起始地址
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) 	//扇区0起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) 	//扇区1起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) 	//扇区2起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) 	//扇区3起始地址, 16 Kbytes  
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) 	//扇区4起始地址, 64 Kbytes  
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) 	//扇区5起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) 	//扇区6起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) 	//扇区7起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) 	//扇区8起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) 	//扇区9起始地址, 128 Kbytes  
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) 	//扇区10起始地址,128 Kbytes  
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) 	//扇区11起始地址,128 Kbytes  

int STMFLASH_write_bytes(uint32_t appxaddr,uint8_t *buf,uint16_t len);
int STMFLASH_write_bytes_noerase(uint32_t appxaddr,uint8_t *buf,uint16_t len);
void rst_addr_erase_mark(uint32_t addr);
						   
#endif

















