#ifndef __PRINTF_H__
#define __PRINTF_H__
#include "Console.h"

#define printf(fmt, args...) Console::Instance()->printf(fmt, ##args)
#define is_printf_idel()	Console::Instance()->isIdle()

#endif
//end of file
