/**
******************************************************************************
* @file    syssettings.cpp
*****************************************************************************/
#include <stdlib.h>
#include <sys/utsname.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <asm/types.h>

#include "asciibin.hpp"
#include "syssettings.hpp"

#include "logging.hpp"
#define LOG_SUBSYSTEM_ID "default"
#include <systools.hpp>

/////////////////////////////////////////////////////////////////////////////
SysSettings::SysSettings()
{
}

/////////////////////////////////////////////////////////////////////////////
SysSettings::~SysSettings()
{
}

/////////////////////////////////////////////////////////////////////////////
void SysSettings::systemDateTime_set(char day, char month, int year, char hour, char minute, char second, bool hwSet)
{
	SysTools::fpsystem("date %02d%02d%02d%02d%04d.%02d", month, day, hour, minute, year, second);
	// aggiorno l'hardware clock
	if (hwSet) {
        if (SysTools::fpsystem("hwclock -w") == 0)
            _VBL(1) << "Successfully updated hardware clock";
	}
}

/////////////////////////////////////////////////////////////////////////////
char *SysSettings::systemUname_request(void)
{
    utsname 	structInfo;
	static char linuxVersion[128];

    // info sistema
    int nRet = uname(&structInfo);
    if( nRet == -1 ) {
		_LSYSERROR("uname failed");
        return 0;
	}

	snprintf(linuxVersion, 128, "%s %s %s %s", structInfo.sysname, structInfo.release, structInfo.version, structInfo.machine);

	_VBL(1) << "Uname:" <<
		" " << structInfo.domainname <<
		" " << structInfo.machine <<
		" " << structInfo.nodename <<
		" " << structInfo.release <<
		" " << structInfo.sysname <<
		" " << structInfo.version;
	
	return linuxVersion;
}

/////////////////////////////////////////////////////////////////////////////
void SysSettings::systemReboot_request()
{
    if (system("reboot") == -1)
		_LSYSERROR("system(\"reboot\") failed");
}
