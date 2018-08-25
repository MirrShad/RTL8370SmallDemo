#ifndef __NET_SOCKET_H__
#define __NET_SOCKET_H__

#include <stddef.h>
#include <stdint.h>	
	
struct in_addr {
  uint32_t s_addr;
};

	/* members are in network byte order */
struct sockaddr_in {
  uint8_t sin_len;
  uint8_t sin_family;
  uint16_t sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

struct sockaddr {
  uint8_t sa_len;
  uint8_t sa_family;
  char sa_data[14];
};

/** 255.255.255.255 */
#define INADDR_NONE         ((uint32_t)0xffffffffUL)
/** 127.0.0.1 */
#define INADDR_LOOPBACK     ((uint32_t)0x7f000001UL)
/** 0.0.0.0 */
#define INADDR_ANY          ((uint32_t)0x00000000UL)
/** 255.255.255.255 */
#define INADDR_BROADCAST    ((uint32_t)0xffffffffUL)

/* Flags we can use with send and recv. */
#define MSG_PEEK       0x01    /* Peeks at an incoming message */
#define MSG_WAITALL    0x02    /* Unimplemented: Requests that the function block until the full amount of data requested can be returned */
#define MSG_OOB        0x04    /* Unimplemented: Requests out-of-band data. The significance and semantics of out-of-band data are protocol-specific */
#define MSG_DONTWAIT   0x08    /* Nonblocking i/o for this operation only */
#define MSG_MORE       0x10    /* Sender will send more */

#define AF_INET         2

/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

uint32_t byteston(uint8_t* ip);
uint16_t htons(uint16_t n);
uint16_t ntohs(uint16_t n);

typedef uint32_t socklen_t;

namespace net{
	
	int socket(int domain, int type, int protocol);
	int sendto(int s, const void *dataptr, size_t size, int flags,
			const struct sockaddr *to, socklen_t tolen);
	int recvfrom(int s, void *mem, size_t len, int flags,
				struct sockaddr *from, socklen_t *fromlen);
	int bind(int s, const struct sockaddr *name, socklen_t namelen);
	int close(int s);
	
}

#endif
//end of file
