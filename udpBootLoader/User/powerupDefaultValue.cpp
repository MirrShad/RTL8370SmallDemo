#include "powerupOption.h"
#include "stmflash.h"

extern const uint32_t DEFAULT_POWERUP_VAL[pvf::VarNum] = 
{
	BOOT_PARAM_APP, 
	pvf::VarNum,
	0x0003FFFF,
	0x00000000,//ip
	0x00000000,//mask
	0x00000000,//gateway
	0x0803FF00,
	0,
	0x00000000,//ciper 0
	0x00000000,//ciper 1
	0x00000000//ciper 2
	
};

//end of file
