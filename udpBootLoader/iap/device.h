#ifndef DEVICE_H
#define DEVICE_H
#include <stdint.h>
template <typename T>
class CDevice
{
	public:
		virtual int open() = 0;
		virtual int close() = 0;
	
		virtual int write(const T*, uint32_t) = 0;
		virtual int read(T*, uint32_t) = 0;
	
		virtual int data_in_write_buf() = 0;
		virtual int freesize_in_write_buf() = 0;
		virtual int data_in_read_buf() = 0;
		virtual void clear_read_buf(){}
		virtual bool is_data_flow_break(){return false;}
		
	
		virtual void runTransmitter() = 0;
		virtual void runReceiver() = 0;
	
		void run()
		{
			runTransmitter();
			runReceiver();
		}
	
		
	private:
		
};
#endif
//end of file
