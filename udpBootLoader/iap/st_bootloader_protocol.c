/********************************************************************************
* @file    st_bootloader_protocol.cp
* @author  Chenxx
* @version V1.0
* @date    2016-11-30
* @brief   
********************************************************************************
*All communications from the programming tool (PC) to the device are verified by:
*
*	1. checksum: received blocks of data bytes are XORed. A byte containing the computed
*		XOR of all previous bytes is added to the end of each communication (checksum byte).
*		By XORing all received bytes, data + checksum, the result at the end of the packet
*		must be 0x00
*	2. for each command the host sends a byte and its complement (XOR = 0x00)
********************************************************************************
*	(C) COPYRIGHT 2016 Seer Robotics
********************************************************************************/
#include "st_bootloader_protocol.h"
#include "device.h"
#include "iap.h"
#include "printf.h"
#include "stmflash.h"
#include "Timer.h"

static uint8_t bl_is_read_protected();
static uint8_t xor_check_sum(uint8_t* start_addr, uint16_t size);
uint8_t long_data_buffer[300];
extern uint8_t FIRMWARE_VERSION;
static uint8_t exe_tick = 0;

/**
  * @brief  Gets the version and the allowed commands 
	*					supported by the current version of the bootloader
	* @param  None
  * @retval None
  */
bl_err_t bl_get()
{
	return BL_OK;
}

/**
  * @brief  Gets the bootloader version and the Read
	*					Protectionstatus of the Flash memory
	* @param  None
  * @retval None
  */
bl_err_t bl_get_version_rps()
{
	const uint8_t MSG_SIZE = 5;
	uint8_t msg_buf[MSG_SIZE] = {ACK, FIRMWARE_VERSION, 0x00, 0x00, ACK};

	iap_device.write((uint8_t*)msg_buf, 1);
	iap_device.write((uint8_t*)msg_buf + 1, MSG_SIZE - 1);
	return BL_OK;
}

/**
  * @brief  Gets the chip ID
	* @param  None
  * @retval None
  */
bl_err_t bl_get_id()
{
	const uint16_t PID = 0x413;	//for stm32f4xx -ref. AN2606 Table-21
	const uint8_t MSG_SIZE = 5;
	uint8_t msg_buf[MSG_SIZE] = {ACK, sizeof(PID) -1 , PID >> 8, PID & 0xFF, ACK};

	iap_device.write((uint8_t*)msg_buf, MSG_SIZE);
	return BL_OK;
}
	
/**
  * @brief  Reads up to 256 bytes of memory starting from an
	*					address specified by the application. Ref - AN3155 3.4
	* @param  None
  * @retval None
  */
bl_err_t bl_read_memory()
{
	static uint32_t start_addr = 0;
	static int16_t pack_length = 0;
	switch(exe_tick)
	{
		case 0:
		{
			if(bl_is_read_protected())
			{
				iapdev_write_byte(NACK);
				break;//Error, protected
			}
			iapdev_write_byte(ACK);
			//printf("Get \"Read Memory\" command and wait for 32bits address and 1byte checksum.\r\n");
			exe_tick++;
			return BL_YIELD;
		}
		case 1:
		{
			int i;
			uint8_t sum_check = 0;
			
			if(iap_device.data_in_read_buf() < 5) //4bytes address and 1byte checksum
			{
				if(iap_device.is_data_flow_break())
				{
					printf("timeout and break %s(%d)\r\n",__FUNCTION__, __LINE__);
					iap_device.clear_read_buf();
					exe_tick = 0;
					return BL_ERR;
				}
				return BL_YIELD;
			}
					
			for(i = 0; i < 4; i++)
			{
				/* MSB is transfered fisrt - ref. AN3155 3.6 */
				start_addr <<= 8;
				start_addr += iapdev_read_byte();
			}
			
			sum_check = iapdev_read_byte();			
			
			/* XOR != 0 */
			if(sum_check ^ xor_check_sum((uint8_t*)&start_addr, sizeof(start_addr)))
			{
				printf("start_addr = 0x%X, sum_check = 0x%X\r\n", start_addr, sum_check);
				printf("check sum failed...\r\n");
				break;//sum check error
			}
			iapdev_write_byte(ACK);
			exe_tick++;
			return BL_YIELD;
		}
		case 2:
		{
			if(iap_device.data_in_read_buf() < 2) //4bytes address and 1byte checksum
			{
				if(iap_device.is_data_flow_break())
				{
					printf("timeout and break %s(%d)\r\n",__FUNCTION__, __LINE__);
					iap_device.clear_read_buf();
					exe_tick = 0;
					return BL_ERR;
				}
				return BL_YIELD;
			}
			//get pack length
			pack_length = iapdev_read_byte() + 1;
			//get pack length xor check
			if(iapdev_read_byte() != pack_length -1)
			{
				printf("pack length check sum failed...\r\n");
				exe_tick = 0;
				return BL_ERR;
			}
			iapdev_write_byte(ACK);
			iap_device.write((const uint8_t*)start_addr, pack_length);
			//send ACK. This is not mentioned in AN3155 3.4
			iapdev_write_byte(xor_check_sum((uint8_t*)start_addr, pack_length));
			printf("send %d bytes at address 0x%X\r\n", pack_length, start_addr);
			exe_tick = 0;
			return BL_OK;
		}
		default:
			break;
	}
	exe_tick = 0;
	return BL_ERR;
}

/**
  * @brief  Jumps to user application code located in the internal
	*					Flash memory or in SRAM
	* @param  None
  * @retval None
  */
bl_err_t bl_go()
{
	//static uint8_t exe_tick = 0;
	uint32_t start_addr = 0;
	
	switch(exe_tick)
	{
		case 0:
		{
			if(bl_is_read_protected())
			{
				iapdev_write_byte(NACK);
				break;//Error, protected
			}
			iapdev_write_byte(ACK);
			printf("Get \"Go\" command and wait for 32bits address and 1byte checksum.\r\n");
			exe_tick++;
			return BL_YIELD;
		}
		case 1:
		{
			int i;
			uint8_t sum_check = 0;
			
			if(iap_device.data_in_read_buf() < 5) //4bytes address and 1byte checksum
			{
				if(iap_device.is_data_flow_break())
				{
					printf("timeout and break %s(%d)\r\n",__FUNCTION__, __LINE__);
					iap_device.clear_read_buf();
					exe_tick = 0;
					return BL_ERR;
				}
				return BL_YIELD;
			}
			
			
			for(i = 0; i < 4; i++)
			{
				/* MSB is transfered fisrt - ref. AN3155 3.6 */
				start_addr <<= 8;
				start_addr += iapdev_read_byte();
			}
			
			sum_check = iapdev_read_byte();
			printf("start_addr = 0x%X, sum_check = 0x%X\r\n", start_addr, sum_check);
			
			/* XOR != 0 */
			if(sum_check ^ xor_check_sum((uint8_t*)&start_addr, sizeof(start_addr)))
			{
				printf("check sum failed...\r\n");
				break;//sum check error
			}
			iapdev_write_byte(ACK);
			printf("check sum ok...\r\n");	
			printf("Go to address: 0x%X\r\n", start_addr);	
				
			while(!is_printf_idel());
			iap_load_app(start_addr);
			
			break;//if program goes here, means load app failed.
		}
		default:
			break;
	}

	exe_tick = 0;
	return BL_ERR;
}

/**
  * @brief  Writes up to 256 bytes to the RAM or Flash memory
	*					starting from an address specified by the application
	* @param  None
  * @retval None
	* @Note 	1. XOR check include dataLength and data.
	*					2. dataLength does not include the sumcheck byte.
  */
bl_err_t bl_write_memory()
{
	//static uint8_t exe_tick = 0;
	static uint32_t start_addr = 0;
	static int16_t pack_length = 0;
	
	switch (exe_tick)
	{
		case 0:
		{
			if(bl_is_read_protected())
			{
				iapdev_write_byte(NACK);
				printf("Get \"Write Memory\" command but flash protected\r\n");
				exe_tick = 0;
				return BL_ERR;
			}
			iapdev_write_byte(ACK);
			exe_tick++;
			return BL_YIELD;
		}
		case 1:
		{
			int i;
			uint8_t sum_check = 0;
			if(iap_device.data_in_read_buf() < 5) //4bytes address and 1byte checksum
			{
				if(iap_device.is_data_flow_break())
				{
					printf("timeout and break %s(%d)\r\n",__FUNCTION__, __LINE__);
					printf("give up %d byte, the first byte is 0x%X\r\n", iap_device.data_in_read_buf(), iapdev_read_byte());
					iap_device.clear_read_buf();
					exe_tick = 0;
					return BL_ERR;
				}
				return BL_YIELD;
			}
			
			
			start_addr = 0;
			for(i = 0; i < 4; i++)
			{
				/* MSB is transfered fisrt - ref. AN3155 3.6 */
				start_addr <<= 8;
				start_addr += iapdev_read_byte();
			}
			
			sum_check = iapdev_read_byte();
			//printf("start_addr = 0x%X, sum_check = 0x%X\r\n", start_addr, sum_check);
			/* XOR != 0 */
			if(sum_check ^ xor_check_sum((uint8_t*)&start_addr, sizeof(start_addr))) 
			{
				printf("Error: start_addr = 0x%X, sum_check = 0x%X, result = 0x%X, failed...\r\n",
					start_addr, sum_check, xor_check_sum((uint8_t*)&start_addr, sizeof(start_addr)));
				iapdev_write_byte(NACK);
				exe_tick = 0;
				return BL_ERR;
			}
			//printf("wait for pack length\r\n");
			iapdev_write_byte(ACK);
			exe_tick++;
			return BL_YIELD;
		}
		
		case 2:
		{
			if(iap_device.data_in_read_buf() == 0) //wait for pack length
				return BL_YIELD;			
			pack_length = iapdev_read_byte() + 1;
			
			// ref. AN3155 3.6 page20
			// pack_length must be a multiple of 4
			if(pack_length <= 0 || pack_length > 256 || (pack_length & 0x3) != 0)
			{
				printf("pack length = %d, check failed (0 < pl < 256, multiple of 4)\r\n", pack_length);
				exe_tick = 0;
				return BL_ERR;
			}
			//printf("wait for long data and sumcheck(packlen XOR longdata)\r\n");
			exe_tick++;
			return BL_YIELD;
		}
		
		case 3:
		{
			uint8_t sum_check = 0;

			//pack_length does not include sum check byte
			//ref. AN3155 Rev6 3.6 Figure 12.
			if(iap_device.data_in_read_buf() < (pack_length + 1))
			{
				if(iap_device.is_data_flow_break())
				{
					printf("timeout and break %s(%d)\r\n",__FUNCTION__, __LINE__);
					iap_device.clear_read_buf();
					exe_tick = 0;
					return BL_ERR;
				}
				return BL_YIELD;
			}
			
			
			//printf("get long data\r\n");
			iap_device.read(long_data_buffer, pack_length);
			sum_check = iapdev_read_byte();
			
			//sum check for [data] and [pack_length]
			//XOR != 0
			//ref. AN3155 Rev6 3.6 page18
			if(sum_check ^ (xor_check_sum(long_data_buffer, pack_length) ^ (pack_length - 1)))
			{
				iapdev_write_byte(NACK);
				printf("long data sum check failed...\r\n");
				exe_tick = 0;
				return BL_ERR;
			}
			
			exe_tick = 0;
			
			if(0 != STMFLASH_write_bytes(start_addr, long_data_buffer, pack_length))
			{
				printf("write to address = 0x%X failed\r\n", start_addr);
				iapdev_write_byte(NACK);
				exe_tick = 0;
				return BL_ERR;
			}

			printf("write %d bytes at 0x%X ok.\r\n", pack_length, start_addr);
			iapdev_write_byte(ACK);
			return BL_OK;
		}
		default:
			break;
	};
	
	exe_tick = 0;
	return BL_ERR;
}

/**
  * @brief  Erases from one to all the Flash memory pages
	* @param  None
  * @retval None
  */
bl_err_t bl_erase()
{
	return BL_OK;
}

/**
  * @brief  Erases from one to all the Flash memory pages using
	*					two byte addressing mode (available only for v3.0 usart
	*					bootloader versions and above).
	* @param  None
  * @retval None
  */
bl_err_t bl_extended_erase()
{
	return BL_OK;
}

/**
  * @brief  Enables the write protection for some sectors
	* @param  None
  * @retval None
  */
bl_err_t bl_write_protect()
{
	return BL_OK;
}

/**
  * @brief  Disables the write protection for all Flash memory sectors
	* @param  None
  * @retval None
  */
bl_err_t bl_write_unprotect()
{
	return BL_OK;
}

/**
  * @brief  Enables the read protection
	* @param  None
  * @retval None
  */
bl_err_t bl_readout_protect()
{
	return BL_OK;
}

/**
  * @brief  Disables the read protection
	* @param  None
  * @retval None
  */
bl_err_t bl_readout_unprotect()
{
	return BL_OK;
}

/**
  * @brief  check memory read protect
	* @param  None
	* @retval 1: is protected
	* @retval 0: not protected
  */
static uint8_t bl_is_read_protected()
{
 return 0;
}

/**
  * @brief  XOR check sum
	* @param  start_addr: data start address for checking
	* @param  size: data number
	* @retval check sum result
  */
static uint8_t xor_check_sum(uint8_t* data_addr, uint16_t size)
{
	uint8_t result = 0x00;
	int i;
	for(i = 0; i < size; i++)
	{
		result ^= *(data_addr + i);
	}
	//printf("sum check result = 0x%X\r\n", result);
	return result;
}

/**
  * @brief  iap device write byte
	* @param  value: data number
	* @retval bl_err_t
  */
bl_err_t iapdev_write_byte(uint8_t value)
{
	if (1 != iap_device.write(&value, 1))
	{
		/* tx buffer overflow */
		return BL_ERR;
	}
	
	return BL_OK;
}

/**
  * @brief  iap device read byte
	* @param  None
	* @retval value: 0-255
	* @Note 	If return value = -1, means rx buffer empty
  */
int iapdev_read_byte()
{
	uint8_t temp_char = 0;
	
	if(iap_device.data_in_read_buf() == 0)
		return -1;
	
	iap_device.read(&temp_char, 1);
	return temp_char;
}

const uint8_t BL_CMD_CODE_NUM = 12;

typedef struct
{
	uint8_t bl_cmd_code;
	bl_err_t (*bl_cmd_entry)(void);
}bl_cmd_map_typ;

bl_cmd_map_typ bl_cmd_map[BL_CMD_CODE_NUM] = 
{
	{0x00, bl_get},				
	{0x01, bl_get_version_rps},
	{0x02, bl_get_id}, 				
	{0x11, bl_read_memory}, 			
	{0x21, bl_go}, 						
	{0x31, bl_write_memory}, 		
	{0x43, bl_erase}, 				
	{0x44, bl_extended_erase}, 
	{0x63, bl_write_protect}, 
	{0x73, bl_write_unprotect},
	{0x82, bl_readout_protect}, 	
	{0x92, bl_readout_unprotect}
};

/**
  * @brief  excute correspond function according to CMD
	* @param  uint8_t cmd
	* @retval bl_err_t
  */
bl_err_t bl_excute_cmd(uint8_t cmd)
{

	int i = 0;
	for(i = 0; i < BL_CMD_CODE_NUM; i++)
	{
		if(bl_cmd_map[i].bl_cmd_code == cmd)
		{
			return bl_cmd_map[i].bl_cmd_entry();
		}
		
	}
	printf("Cannot find command 0x%X\r\n", cmd);
	return BL_ERR;
	
}

#include "udpDevice.h"
CDevice<uint8_t>& iap_device(iapUdpDevice);
//end of file
