#ifndef __LOGGING_HPP
#define __LOGGING_HPP

#include <iomanip>
#include <errno.h>

#define _ELPP_THREAD_SAFE
#define _ELPP_DISABLE_ASSERT
#include <easylogging++.hpp>

// Logging helper macros

#define DEFAULT_LOG_ID "default"

#define VBL(l)    CVLOG((l),    DEFAULT_LOG_ID)
#define DBG()     CLOG(DEBUG,   DEFAULT_LOG_ID)
#define INF()     CLOG(INFO,    DEFAULT_LOG_ID)
#define WARNING() CLOG(WARNING, DEFAULT_LOG_ID)
#define ERROR()   CLOG(ERROR,   DEFAULT_LOG_ID)
#define FATAL()   CLOG(FATAL,   DEFAULT_LOG_ID)

#define _VBL(l)    CVLOG(l,      LOG_SUBSYSTEM_ID)
#define _DBG()     CLOG(DEBUG,   LOG_SUBSYSTEM_ID)
#define _INF()     CLOG(INFO,    LOG_SUBSYSTEM_ID)
#define _WARNING() CLOG(WARNING, LOG_SUBSYSTEM_ID)
#define _ERROR()   CLOG(ERROR,   LOG_SUBSYSTEM_ID)
#define _FATAL()   CLOG(FATAL,   LOG_SUBSYSTEM_ID)

#define HEX3(v, w, f)  "0x"<<std::setbase(16)<<std::setw(w)<<std::setfill(f)<<(unsigned)(v)<<std::setbase(10)
#define HEX(v, w)      HEX3(v, w, '0')
#define HEX1(v)        HEX3(v, sizeof(v)*2, '0')

#ifdef ENDL
#undef ENDL
#endif
#define ENDL

#define INITIALIZE_LOGGING(_filename, _logid) \
do { \
	/* Register and get pointer of custom logger */ \
	easyloggingpp::Configurations conf(_filename); \
	m_logger = easyloggingpp::Loggers::getLogger(_logid); \
	m_logger->configure(conf); \
} while(0);

//
#define __PEBUFF_SIZE 80

#define __PERROR(msg) \
do{\
	char _pebuf[__PEBUFF_SIZE];\
	fprintf(stderr, "!!! %s: %s (%d) %s\n",\
		__func__, msg, errno, strerror_r(errno, _pebuf, __PEBUFF_SIZE));\
}while(0);

#define __PERROR_I(msg) \
do{\
	char _pebuf[__PEBUFF_SIZE];\
	fprintf(stderr, "!!! %s(%p): %s (%d) %s\n",\
		__func__, this, msg, errno, strerror_r(errno, _pebuf, __PEBUFF_SIZE));\
}while(0);

#define _LSYSERROR(msg) \
do{\
	char _pebuf[__PEBUFF_SIZE];\
	_ERROR() << msg << " (" << errno << ") " << strerror_r(errno, _pebuf, __PEBUFF_SIZE);\
}while(0);
#define _LSYSERROR_INST(msg) \
do{\
	char _pebuf[__PEBUFF_SIZE];\
	_ERROR() << "@" << this << " " << msg << " (" << errno << ") " << strerror_r(errno, _pebuf, __PEBUFF_SIZE);\
}while(0);

#define _LSYSFATAL(msg) \
do{\
	char _pebuf[__PEBUFF_SIZE];\
	_FATAL() << msg << " (" << errno << ") " << strerror_r(errno, _pebuf, __PEBUFF_SIZE);\
}while(0);
#define _LSYSFATAL_INST(msg) \
do{\
	char _pebuf[__PEBUFF_SIZE];\
	_FATAL() << "@" << this << " " << msg << " (" << errno << ") " << strerror_r(errno, _pebuf, __PEBUFF_SIZE);\
}while(0);

#endif /* __LOGGING_HPP */
