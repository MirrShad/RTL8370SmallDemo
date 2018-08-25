#include "socket.h"
#include "W5500Device.h"
#include <string.h>
#include "w5500socket/socket.h"

namespace{
	const uint8_t MAX_SOCK_NUM = 8;
	bool socket_array[MAX_SOCK_NUM] = {true};
}

int net::socket(int domain, int type, int protocol)
{
	return W5500Device::Instance()->allocSocket();
}

int net::sendto(int s, const void *dataptr, size_t size, int flags,
    const struct sockaddr *to, socklen_t tolen)
{
	int ret = 0;
	
	if((ret = W5500Device::Instance()->validSocket(s)) < 0)
		return ret;
	
	uint8_t ip[4];
	uint16_t port;
	memcpy(ip, &(((sockaddr_in*)to)->sin_addr.s_addr), 4);
	port = ntohs(((sockaddr_in*)to)->sin_port);
	ret = ::sendto(s, (uint8_t*)dataptr, size, ip, port);
	return ret;
}

int net::recvfrom(int s, void *mem, size_t bufflen, int flags, struct sockaddr *from, socklen_t *fromlen)
{
	int datalen;
	uint8_t ip[4];
	uint16_t port;
	
	int ret = 0;
	
	if((ret = W5500Device::Instance()->validSocket(s)) < 0)
		return ret;
	
	uint8_t so = getSn_SR(s);
	switch(so)                // check SOCKET status
	{                                   // ------------
		case SOCK_UDP: 
			if(getSn_IR(s) & Sn_IR_CON)   // check Sn_IR_CON bit
			{
				setSn_IR(s,Sn_IR_CON);     // clear Sn_IR_CON
			}
			if((datalen = getSn_RX_RSR(s)) > 0) // check the size of received data
			{
				datalen = datalen > bufflen ? bufflen : datalen;
				datalen = ::recvfrom(s, (uint8_t*)mem, bufflen, ip, &port);
				if(datalen > 0)
				{
					((sockaddr_in*)from)->sin_addr.s_addr = byteston(ip);
					((sockaddr_in*)from)->sin_port = htons(port);
					ret = datalen;
				}
			}
			
			break;
			
		default:
			ret = 0;
			break;
	}
	
	return ret;
}

int net::bind(int s, const struct sockaddr *name, socklen_t namelen)
{
	int ret = 0;
	
	uint16_t port = ntohs(((sockaddr_in*)name)->sin_port);
	
	if((ret = W5500Device::Instance()->bindSocket(s, Sn_MR_UDP, port,0)) < 0)
		return ret;
	
	return ::socket(s, Sn_MR_UDP, port,0);
}

int net::close(int s)
{
	W5500Device::Instance()->freeSocket(s);
	
	return ::close(s);
	
}

//end of file
