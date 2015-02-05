/**
******************************************************************************
* @file    configfile.hpp
* @brief   Class for managing a simple INI-style configuration file
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
#ifndef __CONFIGFILE_HPP_INCLUDED
#define __CONFIGFILE_HPP_INCLUDED

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

class ConfigFile
{
//  TYPES  ///////////////////////////////////////////////////////////////////
protected:
	typedef map<string, string> keyval_dict_t;

//  METHODS  /////////////////////////////////////////////////////////////////
public:
	ConfigFile();
	ConfigFile(const char * filepath);
	ConfigFile(const char * filepath, const char * section);
	virtual ~ConfigFile();
	
	int open(const char * filepath, const char * section = 0);
	
	int write();
	int write(const char * filepath);
	
	int rewrite(const char * filepath, const char * section);
	
	int close();
	
	bool is_changed() const {
		return modified;
	}

// Load functions
	int get(const char * key, char * dest, const char * default_value, size_t max_size);

	int get(const char * key, int64_t &dest, int64_t default_value);
	int get(const char * key, int32_t &dest, int32_t default_value);
	int get(const char * key, int16_t &dest, int16_t default_value);
	int get(const char * key, int8_t  &dest, int8_t  default_value);

	int get(const char * key, uint64_t &dest, uint64_t default_value);
	int get(const char * key, uint32_t &dest, uint32_t default_value);
	int get(const char * key, uint16_t &dest, uint16_t default_value);
	int get(const char * key, uint8_t  &dest, uint8_t  default_value);
	int get(const char * key, bool     &dest, bool     default_value);

	int get(const char * key, double &dest, double default_value);
	int get(const char * key, float  &dest, float  default_value);
	
	int get(const char * key, struct in_addr *dest, const struct in_addr *default_value);
	int get(const char * key, struct in_addr *dest, const char *default_value);

	int get_raw(const char * key, void * dest, size_t expected_size);

// Compact load functions
	int64_t  get(const char * key, int64_t  default_value = 0) { int64_t  tmp; get(key, tmp, default_value); return tmp; }
	int32_t  get(const char * key, int32_t  default_value = 0) { int32_t  tmp; get(key, tmp, default_value); return tmp; }
	int16_t  get(const char * key, int16_t  default_value = 0) { int16_t  tmp; get(key, tmp, default_value); return tmp; }
	int8_t   get(const char * key, int8_t   default_value = 0) { int8_t   tmp; get(key, tmp, default_value); return tmp; }
	uint64_t get(const char * key, uint64_t default_value = 0) { uint64_t tmp; get(key, tmp, default_value); return tmp; }
	uint32_t get(const char * key, uint32_t default_value = 0) { uint32_t tmp; get(key, tmp, default_value); return tmp; }
	uint16_t get(const char * key, uint16_t default_value = 0) { uint16_t tmp; get(key, tmp, default_value); return tmp; }
	uint8_t  get(const char * key, uint8_t  default_value = 0) { uint8_t  tmp; get(key, tmp, default_value); return tmp; }
	double   get(const char * key, double   default_value = 0) { double   tmp; get(key, tmp, default_value); return tmp; }
	float    get(const char * key, float    default_value = 0) { float    tmp; get(key, tmp, default_value); return tmp; }
	
// Save functions
	int put(const char * key, const char * dest);

	int put(const char * key, int64_t const value);
	int put(const char * key, int32_t const value);
	int put(const char * key, int16_t const value);
	int put(const char * key, int8_t const value);

	int put(const char * key, uint64_t const value);
	int put(const char * key, uint32_t const value);
	int put(const char * key, uint16_t const value);
	int put(const char * key, uint8_t const value);
	int put(const char * key, bool const value);

	int put(const char * key, double const value);
	int put(const char * key, float const value);

	int put(const char * key, const struct in_addr *src);
	int put(const char * key, const struct in6_addr *src);

	int put_raw(const char * key, const void * dest, size_t data_size);

protected:
	virtual int bin_decode(uint8_t *dst, const char *src, size_t dst_size) {
		return AsciiBin::hex_to_binary(dst, src, dst_size);
	}
	
	virtual int bin_encode(char *dst, const uint8_t *src, size_t dst_size, size_t src_size) {
		return AsciiBin::binary_to_hex(dst, src, dst_size, src_size);
	}


private:
	int ini_parse_file(FILE*);
	bool handle_key(const char*, const char*, const char*);
	
//  MEMBER VARIABLES  ////////////////////////////////////////////////////////
public:

protected:
	keyval_dict_t dict;

private:
	/* Maximum line length for any line in INI file. */
	string file_path;
	static const size_t INI_MAX_LINE = 256;
	static const size_t STR_BUFF_LEN = 32;
	const char * active_section;
	bool modified;
	bool update_on_destruction;
};


/****************************************************************************/

#endif /* __CONFIGFILE_HPP_INCLUDED */
/* EOF */
