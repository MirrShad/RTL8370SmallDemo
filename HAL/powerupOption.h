#ifndef POWERUP_OPTION_H
#define POWERUP_OPTION_H

#include <string.h>
#include <stdint.h>
namespace pvf //powerup variables field
{
//	enum powerupVarEnum
//	{
//		VAR_BOOT_OPTI = 0,
//		VAR_NUM_OF_VAR,
//		VAR_DEFA_DO,
//		VAR_DEFA_IP,
//		VAR_DEFA_NET_MASK,
//		VAR_DEFA_GATE_WAY,
//		VAR_BOOT_OPTI_ADDR,
//		VAR_MOTOR_ID_INV,
//		VAR_CIPHER_TXT_0,
//		VAR_CIPHER_TXT_1,
//		VAR_CIPHER_TXT_2,
//		VarNum
//	};
	#include "powerupDefaultValue.h"

	bool verifyPowerupField();

	uint32_t read(powerupVarEnum idx);

	void write(powerupVarEnum idx, uint32_t val);

	void initInApp();

	void printField();

	uint32_t getSuitVarNum();
}



#endif
//end of file
