#include "powerupOption.h"
#include "stmflash.h"
#include "Console.h"
#include "CommonConfig.h"
#include "network_conf.h"

#define POWERUP_VAR_ADDR 0x080FC000
#define MAX_VAR_NUM_FUTURE 256
static uint32_t* powerupVar = (uint32_t*)POWERUP_VAR_ADDR;
static uint32_t powerupVarShadow[pvf::VarNum + 1]; //store XOR check at the last DWORD of the array
extern const uint32_t DEFAULT_POWERUP_VAL[pvf::VarNum];

static uint32_t check_func(uint32_t* startAddr, uint16_t len)
{
	uint32_t res = 0;
	for(int i = 0; i < len; i++)
	{
		res ^= startAddr[i];
	}
	return res;
}

bool pvf::verifyPowerupField()
{
	uint32_t actualVarNum = read(VAR_NUM_OF_VAR);
	if(actualVarNum > MAX_VAR_NUM_FUTURE || actualVarNum < 1)//may be 0xFFFFFFFF
	{
		Console::Instance()->printf("Invalid actual VarNum: 0x%08X\r\n", actualVarNum);
		return false;
	}

	return (check_func(powerupVar, actualVarNum) == powerupVar[actualVarNum]);
}

uint32_t pvf::read(powerupVarEnum idx)
{
	return powerupVar[idx];
}

void pvf::write(powerupVarEnum idx, uint32_t val)
{
	uint32_t suitVarNum = getSuitVarNum();
	if(idx >= suitVarNum)
	{
		Console::Instance()->printf("powerupVar index exceed limited...\r\n");
		return;
	}
	
	//give up memory writing when nothing changed
	if(pvf::read(idx) == val && true == verifyPowerupField())
		return ;
	
	memcpy(powerupVarShadow, powerupVar, suitVarNum * 4);
	powerupVarShadow[idx] = val;
	uint32_t check_res = check_func(powerupVarShadow, suitVarNum);
	powerupVarShadow[suitVarNum] = check_res;
	
	STMFLASH_write_bytes((uint32_t)powerupVar, (uint8_t*)powerupVarShadow, (suitVarNum + 1) * 4);
	rst_addr_erase_mark((uint32_t)powerupVar);
	ServiceDog();
}

void pvf::initInApp()
{
	//No parameter initialize excute only when 
	//powerup field verify failed.
	if(true == verifyPowerupField() && pvf::VarNum == getSuitVarNum())
	{
		Console::Instance()->printf("PVF verify passed\r\n");
		return;
	}
	
	Console::Instance()->printf("Init powerup var field...\r\n");
	uint16_t actualVarNum = getSuitVarNum();
	for(int i = 0; i < actualVarNum; i++)
	{
		powerupVarShadow[i] = pvf::read((pvf::powerupVarEnum)i);
	}
	
	for(int i = getSuitVarNum(); i < pvf::VarNum; i++)
	{
		powerupVarShadow[i] = DEFAULT_POWERUP_VAL[i];
	}
	
	powerupVarShadow[pvf::VAR_NUM_OF_VAR] = pvf::VarNum; 
	
	
//	powerupVarShadow[pvf::VAR_BOOT_OPTI] = BOOT_PARAM_APP;
//	powerupVarShadow[pvf::VAR_NUM_OF_VAR] = pvf::VarNum;
//	powerupVarShadow[pvf::VAR_DEFA_DO] = 0x0003FFFF;
//	
////	uint8_t tempIP[4] = IPV4_ADDR;
////	powerupVarShadow[pvf::VAR_DEFA_IP] = *((uint32_t*)tempIP);
////	
////	uint8_t tempMask[4] = IPV4_SUBNETMASK;
////	powerupVarShadow[pvf::VAR_DEFA_NET_MASK] = *((uint32_t*)tempMask);
////	
////	uint8_t tempGateWay[4] = IPV4_GATEWAY;
////	powerupVarShadow[pvf::VAR_DEFA_GATE_WAY] = *((uint32_t*)tempGateWay);
//	
//	powerupVarShadow[pvf::VAR_BOOT_OPTI_ADDR] = 0x0803FF00;
//	powerupVarShadow[pvf::VAR_MOTOR_ID_INV] = 0;

	uint32_t check_res = check_func(powerupVarShadow, pvf::VarNum);
	STMFLASH_write_bytes((uint32_t)powerupVar, (uint8_t*)powerupVarShadow, pvf::VarNum * 4);
	STMFLASH_write_bytes((uint32_t)powerupVar + pvf::VarNum * 4, (uint8_t*)&check_res, 4);
	rst_addr_erase_mark((uint32_t)powerupVar);
}
	
void pvf::printField()
{
	uint32_t suitVarNum = getSuitVarNum();
	Console::Instance()->printf("========== Powerup Field ==========\r\n");
	for(int i = 0; i <= suitVarNum; i++)
	{
		Console::Instance()->printf("Var[%d] = 0x%08X\r\n", i, read((powerupVarEnum)i));
	}

	if(true == verifyPowerupField())
	{
		Console::Instance()->printf("Verify passed...\r\n");
	}
	else
	{
		Console::Instance()->printf("Verify failed...\r\n");
	}
}

uint32_t pvf::getSuitVarNum()
{
	uint32_t actualVarNum = read(VAR_NUM_OF_VAR);
	
	if(actualVarNum > MAX_VAR_NUM_FUTURE)//may be 0xFFFFFFFF
	{
		Console::Instance()->printf("Invalid actual VarNum: %d. Use Enum VarNum\r\n", actualVarNum);
			return VarNum;
	}else if(actualVarNum != VarNum)
	{
		Console::Instance()->printf("actual VarNum = %d, Enum VarNum = %d, mismatch. Use actual.\r\n", actualVarNum, VarNum);
	}
	return actualVarNum;
}

//end of file
