/**
******************************************************************************
* @file    typedumpers.cpp
*****************************************************************************/

#include <arpa/inet.h>
#include "asciibin.hpp"
#include "typedumpers.hpp"
#include "logging.hpp"


/*************************************************************************//**
**
*****************************************************************************/

std::ostream& operator<<(std::ostream& w, const sockaddr_in * v)
{
	char _sbuff[INET_ADDRSTRLEN];
    w << "[sockaddr_in " <<
		inet_ntop(AF_INET, &v->sin_addr, _sbuff, INET_ADDRSTRLEN) <<
		":" <<
		ntohs(v->sin_port) <<
	"]";
    return w;
}


/*************************************************************************//**
** Print socket address
*/
std::ostream& operator<<(std::ostream& w, const sockaddr * v)
{
	switch (v->sa_family) {
		case AF_INET:  w << reinterpret_cast<const sockaddr_in *>(v); break;
		case AF_INET6: w << reinterpret_cast<const sockaddr_in6 *>(v); break;
		default: w << "(sockaddr)";
	}
	return w;
}


/*****************************************************************************
**
*****************************************************************************/

std::ostream& operator<<(std::ostream& w, const itimerspec &its)
{
	w << "[itimerspec value:" <<
		its.it_value.tv_sec << "." << its.it_value.tv_nsec <<
		" interval:" <<
		its.it_interval.tv_sec << "." << its.it_interval.tv_nsec << "]";
    return w;
}


/*************************************************************************//**
**
*****************************************************************************/

// std::ostream& operator<<(std::ostream& w, const macadd_t &v)
// {
// 	const size_t _sbufsiz = sizeof(macadd_t) * 3 + 1;
// 	char _sbuff[_sbufsiz];
// 	AsciiBin::binary_to_dotted(_sbuff, v.bytes, _sbufsiz, sizeof(macadd_t), 16, '-');
// 	w << "[macadd_t " << _sbuff << "]";
//     return w;
// }


/*****************************************************************************
**
*****************************************************************************/

// std::ostream& operator<<(std::ostream& w, const device_info_s &v)
// {
// 	w << "[device_info (" << HEX((unsigned)v.device_type, 2) << ") V:" <<
// 			(unsigned)v.major_version << "." <<
// 			(unsigned)v.minor_version << "." <<
// 			(unsigned)v.patch_level << "]";
//     return w;
// }


/*****************************************************************************
**
*****************************************************************************/

// std::ostream& operator<<(std::ostream& w, const firmware_info_ext_s &v)
// {
// 	w << "[fw_info_ext run " << HEX((unsigned)v.running_app_code, 2) <<
// 		 " boot(" << (v.boot_valid ? "ok" : "ko") << "): " << v.boot <<
// 		 " appl(" << (v.appl_valid ? "ok" : "ko") << "): " << v.appl << "]";
// 	return w;
// }


/*****************************************************************************
**
*****************************************************************************/

// std::ostream& operator<<(std::ostream& w, const ext_device_info_s &v)
// {
// 	bool const appl_valid = v.flags & 1;
// 	bool const boot_valid = v.flags & 2;
// 
// 	w << "[ext_dev_info run " << HEX((unsigned)v.running_app_code, 2) <<
// 		 " boot(" << (boot_valid ? "ok" : "ko") << "): " << v.boot <<
// 		 " appl(" << (appl_valid ? "ok" : "ko") << "): " << v.appl << "]";
// 	return w;
// }

