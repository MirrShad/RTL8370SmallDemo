#include <stdint.h>
uint32_t byteston(uint8_t* ip)
{
	return *(uint32_t*)ip;
}

uint16_t htons(uint16_t n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

uint16_t ntohs(uint16_t n)
{
	return htons(n);
}
