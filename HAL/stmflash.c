#include "stmflash.h"
#include "stm32f4xx.h"
#include <string.h>
#include "printf.h"
 



uint16_t STMFLASH_GetFlashSector(u32 addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_10; 
	return FLASH_Sector_11;	
}

uint8_t FLASH_Sector_to_index(uint16_t FLASH_Sector)
{
	return (FLASH_Sector>>3);
}

const uint8_t MAX_SECTOR_IDX = 12;
uint8_t sector_erase_state_tab[MAX_SECTOR_IDX] = 
{
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00
};

#define RESET_SECTOR_ERASE_MARK() \
	memset(sector_erase_state_tab, 0x00, MAX_SECTOR_IDX)

uint8_t is_sector_erased(uint8_t sector_idx)
{
	if(sector_idx < MAX_SECTOR_IDX)
	{
		return (sector_erase_state_tab[sector_idx] == 0xFF);
	}
	return 0;
}

void mark_sector_erased(uint8_t sector_idx)
{
	if(sector_idx < MAX_SECTOR_IDX)
		sector_erase_state_tab[sector_idx] = 0xFF;
}

void rst_addr_erase_mark(uint32_t addr)
{
	uint8_t sector_idx = FLASH_Sector_to_index(STMFLASH_GetFlashSector(addr));
	sector_erase_state_tab[sector_idx] = 0x00;
}

/**
  * @brief  Check before erasing. If erased, do nothing
	* @param  FLASH_Sector
	* @retval None
  */
FLASH_Status erase_sector_once(uint32_t FLASH_Sector)
{
	FLASH_Status ret = FLASH_COMPLETE;
	uint8_t idx = FLASH_Sector_to_index(FLASH_Sector);
	if(!is_sector_erased(idx))
	{
		printf("Erasing sector %d\r\n", FLASH_Sector_to_index(FLASH_Sector));
		ret = FLASH_EraseSector(FLASH_Sector, VoltageRange_3);
		mark_sector_erased(idx);
	}
	return ret;
}

int STMFLASH_write_bytes_noerase(uint32_t appxaddr,uint8_t *buf,uint16_t len)
{
	uint8_t idx = FLASH_Sector_to_index(STMFLASH_GetFlashSector(appxaddr));
	mark_sector_erased(idx);
	return STMFLASH_write_bytes(appxaddr, buf, len);
}

/**
  * @brief  write byte array to flash
	* @param  FLASH_Sector
	* @retval -1 means error
	* @Note   Also active for OTP. OTP address: 0X1FFF7800~0X1FFF7A0F
  */
int STMFLASH_write_bytes(uint32_t appxaddr,uint8_t *buf,uint16_t len)
{
	int i;
	int ret = 0;
	FLASH_Status flash_ret;
	// pack_length must be a multiple of 4
	// pack_length is less than 256
	// that means it will never write cross sector
	if((len & 0x3) != 0 || len > 256 || appxaddr<STM32_FLASH_BASE)
	{
		printf("buffer length check failed\r\n");
		return -1;
	}	
	
	FLASH_Unlock();							
  FLASH_DataCacheCmd(DISABLE);
	
	if(appxaddr < 0X1FFF0000)			//do not erase OTP sector
	{
		//printf("Erasing sector %d\r\n", FLASH_Sector_to_index(STMFLASH_GetFlashSector(appxaddr)));
		FLASH_ClearFlag( FLASH_FLAG_EOP |  FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
		if((flash_ret = erase_sector_once(STMFLASH_GetFlashSector(appxaddr))) != FLASH_COMPLETE)
		{
			printf("erase sector failed, error code: %d\r\n", flash_ret);
			RESET_SECTOR_ERASE_MARK();
			ret = -1;
		}
	}
//	printf("Erase finished...\r\nStart to write\r\n");
	if(ret == 0) //did not get any error
	{
		for(i = 0; i < len; i+= 4)
		{
			if((flash_ret = FLASH_ProgramWord(appxaddr + i, *(uint32_t*)(buf + i))) != FLASH_COMPLETE)
			{ 
				printf("flash program failed, error code: %d\r\n", flash_ret);
				RESET_SECTOR_ERASE_MARK();
				ret = -1;
				break;
			}
		}
	}
//	printf("Flash programming finished...\r\n");
	FLASH_DataCacheCmd(ENABLE);
	FLASH_Lock();
		
	return ret;
}
