#ifndef UDPDEVICE_H
#define UDPDEVICE_H
#include "device.h"
#include "ringque.h"
#include "Timer.h"
#include "socket.h"

class CUdpDevice
	:public CDevice<uint8_t>
{
	public:
		CUdpDevice(uint16_t, uint8_t*, uint16_t);
	
		virtual int open();
		virtual int close();
	
		virtual int write(const uint8_t*, uint32_t);
		virtual int read(uint8_t*, uint32_t);
	
		virtual int data_in_write_buf();
		virtual int freesize_in_write_buf();
		virtual int data_in_read_buf();
		virtual void clear_read_buf();
		virtual bool is_data_flow_break();
	
		virtual void runTransmitter() {}
		virtual void runReceiver();
		void run();
		void set_remote_ip(uint8_t, uint8_t, uint8_t, uint8_t);
		void set_remote_port(uint16_t);
		static uint16_t rxque_push_callback(void*, uint8_t*, uint16_t);

	private:
//		uint16_t remote_port_;
		int socket_n_;
//		uint8_t remote_ip_[4];
		sockaddr_in remote_addr_;
		bool is_first_in_;
		ringque<uint8_t> rx_queue_;
		Timer data_flow_break_timer_;
		uint16_t prev_elems_rx_que_;
};

extern CUdpDevice iapUdpDevice;
#endif
//end of file
