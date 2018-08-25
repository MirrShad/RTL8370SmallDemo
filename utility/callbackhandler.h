#ifndef CALLBACK_HANDLER_H
#define CALLBACK_HANDLER_H

#include <stdint.h>
#include <string.h>

class CCallbackHandler_base
{
	public:
		virtual int handle(uint8_t* pbCmd = NULL, uint16_t len = 0) = 0;
};

template <class Type>
class CCallbackHandler: public CCallbackHandler_base
{
	public:
		typedef int (Type::*HandlerFunc)(uint8_t* pbCmd, uint16_t len);
	
	public:
		CCallbackHandler(Type *ptr, HandlerFunc func):
		pointer_(ptr),
		func_(func)
		{}
		virtual int handle(uint8_t* pbCmd = NULL, uint16_t len = 0)
		{
			if (pointer_ && func_)
				return (pointer_->*func_)(pbCmd, len);
			return 0;
		}
		
	private:
			Type *pointer_;
			HandlerFunc func_;
};

template <>
class CCallbackHandler<void>: public CCallbackHandler_base
{
	public:
		typedef int (*HandlerFunc)(uint8_t* pbCmd, uint16_t len);
		CCallbackHandler(HandlerFunc func):func_(func)
		{}
		virtual int handle(uint8_t* pbCmd = NULL, uint16_t len = 0)
		{
			if (func_)
				return (*func_)(pbCmd, len);
			return 0;
		}
	private:
		HandlerFunc func_;	
};

#endif
//end of file
