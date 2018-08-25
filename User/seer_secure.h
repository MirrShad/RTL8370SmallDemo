#ifndef SEER_SECURE_H
#define SEER_SECURE_H
#include <stdint.h>
#include "Timer.h"

int seer_uid_verify(uint8_t* uid_ciphertext);
int seer_uid_encrypt();
int seer_uid_pc_decrypt();
int seer_uid_pc_encrypt();

int GetUidHandlerFunc(uint8_t* msg, uint16_t len);
int UidUnlockHandlerFunc(uint8_t* msg, uint16_t len);
void secureCheck();
//extern CAsyncTimer<void> SecureCheckTimer;

#endif
//end of file
