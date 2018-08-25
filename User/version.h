#ifndef __VERSION_H__
#define __VERSION_H__

#include "Singleton.h"
#include <stdio.h>
#include "CommandDispatchTask.h"


///////////////////////////////////////////////////////////////
/************************Version Define************************/
#define SDB_VERSION_MAJOR           1
#define SDB_VERSION_MINOR           7
#define SDB_VERSION_REVISION        909
/************************Version Define************************/
////////////////////////////////////////////////////////////////

#define SDB_BUILD_DATE      __DATE__
#define SDB_BUILD_TIME      __TIME__
#define SDB_STR_2(x)        #x
#define SDB_STR(x)          SDB_STR_2(x)
#define SDB_MAIN_VERSION    SDB_STR(SDB_VERSION_MAJOR)##"."##SDB_STR(SDB_VERSION_MINOR)##"."##SDB_STR(SDB_VERSION_REVISION)

#define SDB_VERSION_LENGTH  100

class CVersion
{
public:
    CVersion(){resetStr();}

    int getMajor(){return SDB_VERSION_MAJOR;}
    int getMinor(){return SDB_VERSION_MINOR;}
    int getRevision(){return SDB_VERSION_REVISION;}

    const char* getMainVersion()
    {
        resetStr();
        sprintf(verStr,"%s",SDB_MAIN_VERSION);
        return verStr;
    }

    const char* getBuildTime()
    {
        resetStr();
        sprintf(verStr,"%s,%s",SDB_BUILD_DATE,SDB_BUILD_TIME);
        return verStr;
    }

    const char* getFullVersion()
    {
        resetStr();
        sprintf(verStr,"%s, built on %s-%s",SDB_MAIN_VERSION,SDB_BUILD_DATE,SDB_BUILD_TIME);
        return verStr;
    }
		
	int queryVersionHandler(uint8_t*, uint16_t)
	{
		Console::Instance()->printf("SeerDIOBoard - %s\r\n", getFullVersion());
		
		CmdSocket::Instance()->transParam(0) = CmdSocket::Instance()->lastCmdTyp();
		CmdSocket::Instance()->transParam(1) = getMajor();
		CmdSocket::Instance()->transParam(2) = getMinor();
		CmdSocket::Instance()->transParam(3) = getRevision();
		CmdSocket::Instance()->sendto(4 * sizeof(uint32_t), CmdSocket::Instance()->cmderIP(), CmdSocket::Instance()->cmderPort());
		
		return 0;
	}

private:
    void resetStr(){memset(verStr,0,SDB_VERSION_LENGTH);}

private:
    char verStr[SDB_VERSION_LENGTH];
};

typedef NormalSingleton<CVersion> Version;


#endif  //~__VERSION_H__
