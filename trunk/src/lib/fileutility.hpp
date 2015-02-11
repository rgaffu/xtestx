/**
******************************************************************************
* @file    fileutility.hpp
* @brief   Class for utility file system
*
* @author  
* @version V1.0.0
* @date    01-Jan-2015
*          
* @verbatim
* @endverbatim
*
******************************************************************************
* @attention
*
******************************************************************************
* @note
*
*****************************************************************************/

/*Include only once */
#ifndef __FILEUTILITY_HPP_INCLUDED
#define __FILEUTILITY_HPP_INCLUDED

#ifndef __cplusplus
#error configfile.hpp is C++ only.
#endif

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>
#include <stdint.h>
#include <stdio.h> 
#include <string>
#include <map>

#include <asciibin.hpp>

#if defined(_WIN32) || defined(_WIN64)
#include <Ws2tcpip.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


//////////////////////////////////////////////////////////////////////////////
//                   T Y P E   D E F I N I T I O N S                        //
//////////////////////////////////////////////////////////////////////////////

using namespace std;

class FileUtility
{
//  METHODS  /////////////////////////////////////////////////////////////////
public:
	FileUtility();
	virtual ~FileUtility();

    long int filesize(const char *filename);
    int domkdir(const char *directory);
    
protected:


private:
	
//  MEMBER VARIABLES  ////////////////////////////////////////////////////////
public:

protected:

private:
};


/****************************************************************************/

#endif /* __FILEUTILITY_HPP_INCLUDED */
/* EOF */
