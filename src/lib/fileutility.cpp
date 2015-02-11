/**
******************************************************************************
* @file    fileutility.cpp
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "fileutility.hpp"
#include "logging.hpp"
#define LOG_SUBSYSTEM_ID "default"


/////////////////////////////////////////////////////////////////////////////
FileUtility::FileUtility()
{
}

/////////////////////////////////////////////////////////////////////////////
FileUtility::~FileUtility()
{
}

/////////////////////////////////////////////////////////////////////////////
long int FileUtility::filesize(const char *filename)
{
    FILE  *fdFile;
    
	if((fdFile = fopen(filename, "r")) == NULL){
		_VBL(1) << "file" << filename << " not exist";
		return 0;
	}
	fseek(fdFile, 0, SEEK_END);
	long int size = ftell(fdFile);
	fclose(fdFile);
		
    _VBL(1) << "file" << filename << " size " << size;
	return size;
}

/////////////////////////////////////////////////////////////////////////////
int FileUtility::domkdir(const char *directory)
{
    typedef struct stat Stat;
    Stat        st;
    int         status = 0;
    int         mode = 0777;

    INF() << __func__ << directory;
    if (stat(directory, &st) != 0)  {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(directory, mode) != 0 && errno != EEXIST)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        status = -1;
    }

    return status;
}
