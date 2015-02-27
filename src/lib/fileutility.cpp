/**
******************************************************************************
* @file    fileutility.cpp
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

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
bool FileUtility::fileexist(const char *filename)
{
    FILE  *fdFile;
    
    if ((fdFile = fopen(filename, "r")) == NULL){
        _VBL(1) << "file" << filename << " not exist";
        return 0;
    }
    fclose(fdFile);
    return true;
}

/////////////////////////////////////////////////////////////////////////////
long int FileUtility::filesize(const char *filename)
{
    FILE  *fdFile;
    
	if ((fdFile = fopen(filename, "r")) == NULL){
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
bool FileUtility::findstring(const char *filename, const char *str)
{
    bool ret = false;
    
    ifstream infile;
    string line;
    infile.open(filename);
    
    while (ret == false) {
        getline(infile, line); 
        if (infile.eof())
            break;
        
        size_t found = line.find(str);
        if (found != string::npos) {
            _VBL(1) << "findstring: " <<  str << " in file " << filename;
            ret = true;
        }        
    }
    infile.close();
    return ret;
}

/////////////////////////////////////////////////////////////////////////////
string FileUtility::line(const char *filename, const int numline)
{
    int num;
    string line;
    bool ret = false;
    
    ifstream infile;
    infile.open(filename);
    
    num = 0;
    line.clear();
    while (ret == false) {
        getline(infile, line); 
        if (infile.eof())
            break;
        
        if (num++ == numline) 
            ret = true;
    }
    infile.close();
    return line;
}

/////////////////////////////////////////////////////////////////////////////
int FileUtility::domkdir(const char *directory)
{
    typedef struct stat Stat;
    Stat        st;
    int         status = 0;
    int         mode = 0777;

    INF() << __func__ << directory;
    if (stat(directory, &st) != 0) {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(directory, mode) != 0 && errno != EEXIST)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode)) {
        errno = ENOTDIR;
        status = -1;
    }

    return status;
}

/////////////////////////////////////////////////////////////////////////////
int FileUtility::check_free_space(const char *media)
{
    struct statvfs buf;

   if (!statvfs(media, &buf)) {
      unsigned long blksize, blocks, freeblks, disk_size, used, free;
      blksize = buf.f_bsize;
      blocks = buf.f_blocks;
      freeblks = buf.f_bfree;

      disk_size = blocks*blksize;
      free = freeblks*blksize;
      used = disk_size - free;
      
      INF() << "media:     " << media;
      INF() << "disk size: " << disk_size;
      INF() << "used:      " << used;
      INF() << "free:      " << free;

      return free;
    }
    else {
        return -1;
    }    
}
