#include "seer_secure.h"
#include "polarssl/config.h"
#include "polarssl/rsa.h"
#include <stdlib.h>
#include "printf.h"

/*
 * RSA-512 keypair, for test purposes
 */
#define KEY_LEN 128

#define RSA_E   "10001"

#define RSA_N   "87344E1AFDF784F50A2719E840F56121B2335A2EB0A06565840495D34F20E22909C5EF022C15D635B2796548328FBC3D61B5C2841EB2792D5AD2DB8B96001536"\
								"1816F079D9064D38D7EFC074330D0B18ABE6608B6DEBC5C4FC436B59B6FEE32D6173E8E14A3FCA5627437632232EAC7F7371EC5497F5D5375653A46A51AF9411"

#define RSA_D   "15A202D542C975D697374667E8ECDA0BC02D8342713458C4F368A0685D02CFECED0964829E7727FD137F64FB364BD53CA503A609E9B778B80F0E29DE9DB389B1"\
								"101754A612E4EDE635DB3FA575263320909460AC02B1A3065DDACF7ADD2552884ED5880656863B19AD55630D5EBB087E044FA8B199D1271C24380B028D36C401"

#define RSA_P   "D1B25317B6AAA949D9DBB634A6327CBD4BAEF3CAF5506DD14D4629A333553774BB878E2E68156BF5A795EBFC8D65EA89D2591C460FA36CC936019364512CFB71"
#define RSA_Q   "A50F19C2A0A62189A232F5ACC1BDE34C19A372CC0280BD5D915F12CC531FCA8A472769FC1C4DF903AB02A2CCB5C18CDBE0B9F754EDB79A8B3E583CCE876F92A1"
#define RSA_DP  "551FBAB369A7A5C920F31BE80B7F7B3BACB97CA5EB27972CDBDD9B9170892B55EBBA7F713F3752F809EBC63C6379F7D7B2D811AE2F32FC3A41FE29C31360DB41"
#define RSA_DQ  "744DFEA5D0931F409F04EF6D8A9AE5A582CF541D8289FA650B0105FAE97B9638F748437AFCEAC8531855E42700CF6014B2F5F6DC6620A08A0B251BBB9F5D33E1"
#define RSA_QP  "3241B111F514B2D1E0F7C84FA1E84A6AE3D2446AC0BF7088F73F1CE108C3A6441A0A9A64BEC5BEF0471C67F6881EFFC77B8F6DED22336CBF72559CD0671E4444"

#define TEMP_CIPEREXT "547C7FE09A537F4E61C9C62D0613E81BA5A68C296B98FF42876E928B0DCD2D4C9F36C5C01E1469FD813B794609EFF391A5413765450AD0AB51D388B24C4E497A5E2D20DD3B9E19D490CC2CE643CBCAEC00E19DFF6A0314742EAC238246175E0EE2282B18E82DF46C585105C600C67D566C7421A845C090028C81685542198A7A"

//96bits UID in BYTE
#define PT_LEN  (96/8)
#define RSA_PT  "112233445566778899AABB"
uint8_t rsa_pt[PT_LEN] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
unsigned char *rsa_plaintext = rsa_pt;
unsigned char rsa_decrypted[PT_LEN];
unsigned char rsa_ciphertext[KEY_LEN];
static rsa_context rsa;

int seer_uid_verify(uint8_t* uid_ciphertext)
{
	size_t len;
	int ret = 0;
	rsa_init( &rsa, RSA_PKCS_V15, 0 );

	rsa.len = KEY_LEN;
	mpi_read_string( &rsa.N , 16, RSA_N  );
	mpi_read_string( &rsa.E , 16, RSA_E  );
	mpi_read_string( &rsa.D , 16, RSA_D  );
	mpi_read_string( &rsa.P , 16, RSA_P  );
	mpi_read_string( &rsa.Q , 16, RSA_Q  );
	mpi_read_string( &rsa.DP, 16, RSA_DP );
	mpi_read_string( &rsa.DQ, 16, RSA_DQ );
	mpi_read_string( &rsa.QP, 16, RSA_QP );

	if( rsa_check_pubkey(  &rsa ) != 0 || rsa_check_privkey( &rsa ) != 0 )
	{
			printf( "Key validation: failed\n" );
			return( 1 );
	}

	printf( "String decode: " );
	if( (ret = rsa_pkcs1_decrypt( &rsa, RSA_PUBLIC, &len,
                           uid_ciphertext, rsa_decrypted,
                           sizeof(rsa_decrypted) )) != 0 )
	{
			printf( "failed, error code :-%X\n" , -ret);
			return( 1 );
	}

	if( memcmp( rsa_decrypted, (uint8_t*)0x1FFF7A10, len ) != 0 )
	{
			printf( "check failed\r\n" );
			printf("decryptedtext = ");
			for(int i = 0; i < len; i++)
			{
				printf("\\x%X", rsa_decrypted[i]);
			}
			printf("\r\n");
			return( 1 );
	}
	printf( "passed\r\n" );
	rsa_free( &rsa );
	return 0;
}

#include "powerupOption.h"
#ifdef printf
#undef printf
#endif
#include "CommandDispatchTask.h"
#include "MessageTask.h"

const uint32_t* uid_addr = (uint32_t*)0x1FFF7A10;

int GetUidHandlerFunc(uint8_t* msg, uint16_t len)
{
	if(sizeof(uint32_t) != len)
	 return -1;

	CmdSocket::Instance()->transParam(0) = CmdSocket::Instance()->lastCmdTyp();

	CmdSocket::Instance()->transParam(1) = uid_addr[0];
	CmdSocket::Instance()->transParam(2) = uid_addr[1];
	CmdSocket::Instance()->transParam(3) = uid_addr[2];

	len = 4 * sizeof(uint32_t);
	CmdSocket::Instance()->sendto(len, CmdSocket::Instance()->cmderIP(), CmdSocket::Instance()->cmderPort());

	return 0;
}

int UidUnlockHandlerFunc(uint8_t* msg, uint16_t len)
{
	int ret = 0;
	do
	{
		if(4 * sizeof(uint32_t) != len)
		{
			ret = -1;
			break;
		}
		
		uint32_t check = 0x0;	
		uint32_t* key = (uint32_t*)msg;
		
		for(int i = 0; i < 3; i++)
		{
			check ^= msg[i];
		}
		
		if(msg[3] != check)
		{
			Console::Instance()->printf("Xor checkout faild\r\n");
			ret = -2;
			break;
		}
		
		pvf::write(pvf::VAR_CIPHER_TXT_0, key[0]);
		pvf::write(pvf::VAR_CIPHER_TXT_1, key[1]);
		pvf::write(pvf::VAR_CIPHER_TXT_2, key[2]);
		
	}while(0);

	CmdSocket::Instance()->transParam(0) = CmdSocket::Instance()->lastCmdTyp();
	CmdSocket::Instance()->transParam(1) = ret;

	len = 2 * sizeof(uint32_t);
	CmdSocket::Instance()->sendto(len, CmdSocket::Instance()->cmderIP(), CmdSocket::Instance()->cmderPort());
	return ret;
}

void secureCheck()
{
	return;
	
	for(int i = 0; i < 3; i++)
	{
		uint32_t result = pvf::read((pvf::powerupVarEnum)((int)pvf::VAR_CIPHER_TXT_0 + i)) ^ uid_addr[i]; 
		if(0 != ~result)
		{
			Message::Instance()->postErrMsg(CODE_HARD_CONN_ERRO, "Error code: %08X", result);
		}
	}
}

//end of file
