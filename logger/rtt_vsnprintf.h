#ifndef RTT_VSPRINTF_H
#define RTT_VSPRINTF_H
#include <stdarg.h>
#include <stdlib.h>
int SEGGER_RTT_vsnprintf(char* destBuff, size_t size, const char * sFormat, va_list * pParamList);

#endif
//end of file

