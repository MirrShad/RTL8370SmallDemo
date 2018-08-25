#include "Console.h"
#include "udpDevice.h"
#include <string.h>
#include "NetworkAdapter.h"
#include "iapportconfig.h"

#define ENABLE_RX_QUEUE 1
/**
  * @brief  Constructor
	* @param  None
  * @retval None
  */
CUdpDevice::CUdpDevice(uint16_t remote_port,
											uint8_t* rx_buffer,
											uint16_t rx_buffer_size)
	:socket_n_(-1),
	is_first_in_(true),
	rx_queue_(rx_buffer, rx_buffer_size),
	data_flow_break_timer_(200, 200),
	prev_elems_rx_que_(0)
{
	remote_addr_.sin_family = AF_INET;
}
	
/**
* @brief  open
* @param  None
* @retval state
*/
int CUdpDevice::open()
{
	if(!NetworkAdapter::Instance()->isOpen())
		return false;
	
	sockaddr_in localaddr;

	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(IAP_PORT);
	localaddr.sin_addr.s_addr = INADDR_ANY;

	net::close(socket_n_);	
	socket_n_ = net::socket(AF_INET, SOCK_DGRAM, 0);
	net::bind(socket_n_, (sockaddr *)&localaddr,  sizeof(localaddr));

	Console::Instance()->printf("Init socket[%d] for udpDevice\r\n", socket_n_);
	return true;
}

/**
* @brief  close
* @param  None
* @retval 
*/
int CUdpDevice::close()
{
	return net::close(socket_n_);
}

/**
  * @brief  write
	* @param  databuf: the data pointer wants to send
	* @param  len: data length
  * @retval actual send length
  */
int CUdpDevice::write(const uint8_t* databuf, uint32_t len)
{
	return net::sendto(socket_n_, databuf, len, MSG_DONTWAIT, (sockaddr *)&remote_addr_, sizeof(remote_addr_));
}

/**
  * @brief  read
	* @param  databuf: the data pointer wants to read
	* @param  len: data length
  * @retval actual read length
  */
int CUdpDevice::read(uint8_t* databuf, uint32_t len)
{
	int32_t ret;
	
	ret = rx_queue_.pop_array(databuf, len);

	return ret;
}

/**
  * @brief  data in read buffer
	* @param  None
  * @retval number of bytes
  */
#if ENABLE_RX_QUEUE
int CUdpDevice::data_in_read_buf()
{
	return rx_queue_.elemsInQue();
}
#else 
#endif

/**
  * @brief  data in write buffer
	* @param  None
  * @retval number of bytes
  */
int CUdpDevice::data_in_write_buf()
{
	return -1;
}

/**
  * @brief  data in write buffer
	* @param  None
  * @retval number of bytes
  */
int CUdpDevice::freesize_in_write_buf()
{
	return 300;
}

/**
  * @brief  run
	* @param  None
  * @retval None
  */
void CUdpDevice::run()
{
	runReceiver();
//		switch(getSn_SR(socket_n_))
//	{
//																											// -------------------------------
//		case SOCK_UDP:                                    //
//		{
//			runReceiver();
//			break;
//		}
//																											// -----------------
//		case SOCK_CLOSED:                                 // CLOSED
//			::close(socket_n_);                             // close the SOCKET
//			 socket(socket_n_, Sn_MR_UDP, native_port_, 0); // open the SOCKET with UDP mode
//			 break;
//		default:
//			 break;
//	}
}

///**
//  * @brief  set remote ip address
//	* @param  192 168 1 4
//  * @retval None
//  */
//void CUdpDevice::set_remote_ip(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3)
//{
//	remote_ip_[0] = byte0;
//	remote_ip_[1] = byte1;
//	remote_ip_[2] = byte2;
//	remote_ip_[3] = byte3;
//}

///**
//  * @brief  set remote port
//	* @param  port num
//  * @retval None
//  */
//void CUdpDevice::set_remote_port(uint16_t port)
//{
//	remote_port_ = port;
//}

/**
  * @brief  run transmitter
	* @param  
  * @retval 
  */
void CUdpDevice::runReceiver()
{
	const uint16_t BUFFLEN = 300;
	uint8_t buff[BUFFLEN];
	unsigned int remoteaddrlen;
	
	int len = 0;
	len = net::recvfrom(socket_n_, buff, BUFFLEN, MSG_DONTWAIT, (sockaddr *)&remote_addr_, &remoteaddrlen);
	if( len > 0) // check the size of received data
	{
		rx_queue_.push_array(buff, len);
		data_flow_break_timer_.reset();
//		Console::Instance()->printf("src addr: %X:%d\r\n", remote_addr_.sin_addr.s_addr, ntohs(remote_addr_.sin_port));
	}
}

/**
  * @brief  clear read buffer
	* @param  None
  * @retval None
  */
void CUdpDevice::clear_read_buf()
{
#if ENABLE_RX_QUEUE
	rx_queue_.clear();
#else
#endif
}

///**
//  * @brief  rxque_push_callback  
//	* @param  None
//  * @retval None
//  */
//uint16_t CUdpDevice::rxque_push_callback(void* obj, uint8_t* ptr, uint16_t len)
//{
//	CUdpDevice* p_obj = (CUdpDevice*)obj;
//	return recvfrom(p_obj->socket_n_, ptr, len, p_obj->remote_ip_, &(p_obj->remote_port_));
//}

/**
  * @brief  is data flow break
	* @param  None
  * @retval None
  */
bool CUdpDevice::is_data_flow_break()
{
	if(rx_queue_.empty())
		return false;
	else
		return data_flow_break_timer_.isAbsoluteTimeUp();
}
const uint16_t IAP_BUFF_SIZE = 300;
uint8_t iap_udp_buffer[IAP_BUFF_SIZE];
CUdpDevice iapUdpDevice(0, iap_udp_buffer, IAP_BUFF_SIZE);

