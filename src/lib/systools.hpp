/**
******************************************************************************
* @file    systools.hpp
* @brief   A collection of useful operating system related routines
*
* @author  
* @version V1.0.0
* @date    05-Dec-2014
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
#ifndef __SYSTOOLS_HPP_INCLUDED
#define __SYSTOOLS_HPP_INCLUDED

#ifndef __cplusplus
#error systools.hpp is C++ only.
#endif

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <logging.hpp>

#define _PSYSBUF_LEN 1024

class SysTools {
public:
	/*************************************************************************//**
	** Execute a shell command, logging the invocation and the result
	** @param cmd command line to execute
	*/
	static int psystem(const char * const cmd)
	{
		FILE *pipe = popen(cmd, "r");
		_INF() << "system: " << cmd;
		if (pipe == 0) {
			_LSYSERROR(cmd);
			return -1;
		} else {
			char * _outbuf = static_cast<char *>(calloc(_PSYSBUF_LEN, 1));
			if (_outbuf == 0) {
				_ERROR() << "Out of memory";
			} else {
				while (fgets(_outbuf, _PSYSBUF_LEN-1, pipe) != NULL) {
					char *save;
					strtok_r(_outbuf, "\n", &save); // Remove trailing newline
					_INF() << _outbuf;
				}
				free(_outbuf);
			}
			if (pclose(pipe) == -1) {
				_LSYSERROR("pclose error");
				return -1;
			}
		}
		return 0;
	}

	/*************************************************************************//**
	** Format and execute a shell command, logging the invocation and the result
	** @param fmt
	*/
	static int fpsystem(const char * const fmt, ... )
	{
		char * _cmdbuf = static_cast<char *>(calloc(_PSYSBUF_LEN+1, 1));
		if (_cmdbuf == 0) {
			_ERROR() << "Out of memory";
			return -1;
		}
		va_list vl;
		va_start(vl, fmt);
		vsnprintf(_cmdbuf, _PSYSBUF_LEN, fmt, vl);
		va_end(vl);
		strncat(_cmdbuf, " 2>&1", _PSYSBUF_LEN);
		int res = psystem(_cmdbuf);
		free(_cmdbuf);
		return res;
	}
};

/****************************************************************************/

#endif /* __SYSTOOLS_HPP_INCLUDED */
/* EOF */
