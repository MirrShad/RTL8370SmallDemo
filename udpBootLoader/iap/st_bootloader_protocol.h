/********************************************************************************
* @file    st_bootloader_protocol.cp
* @author  Chenxx
* @version V1.0
* @date    2016-11-30
* @brief   
*	(C) COPYRIGHT 2016 Seer Robotics
********************************************************************************/
#ifndef ST_BOOTLOADER_PROTOCOL_H
#define ST_BOOTLOADER_PROTOCOL_H
#include <stdint.h>
#include "device.h"

enum bl_err_t
{
	BL_OK = 0,
	BL_YIELD,
	BL_ERR
};

#define ACK 0x79
#define NACK 0x1F

bl_err_t bl_excute_cmd(uint8_t cmd);
bl_err_t iapdev_write_byte(uint8_t value);
int iapdev_read_byte();
extern CDevice<uint8_t>& iap_device;

#endif
//end of file
