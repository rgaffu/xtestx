/**
******************************************************************************
* @file    APPLConfigFile.hpp
* @brief   INI-style configuration for APPL
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
#ifndef __APPLCFGFILE_HPP_INCLUDED
#define __APPLCFGFILE_HPP_INCLUDED

#ifndef __cplusplus
#error applConfigFile.hpp is C++ only.
#endif

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////

#include <configfile.hpp>


//////////////////////////////////////////////////////////////////////////////
//                   T Y P E   D E F I N I T I O N S                        //
//////////////////////////////////////////////////////////////////////////////

using namespace std;


class ApplConfigFile : public ConfigFile
{
//  METHODS  /////////////////////////////////////////////////////////////////
public:
	ApplConfigFile():
		ConfigFile()
	{}
	ApplConfigFile(const char * filepath):
		ConfigFile(filepath)
	{}
	ApplConfigFile(const char * filepath, const char * section):
		ConfigFile(filepath, section)
	{}
/*
	using ConfigFile::get;
	int get(const char * key, macadd_t *dest, const macadd_t *default_value) {
		keyval_dict_t::iterator elem = dict.find(key);
		if (elem == dict.end()) {
			*dest = *default_value;
			return -1;
		}
		int dec_len = bin_decode(dest->bytes, elem->second.c_str(), MAC_ADDR_LEN);
		if (dec_len != MAC_ADDR_LEN) {
			*dest = *default_value;
			return -1;
		}
		return 0;
	}
	int get(const char * key, macadd_t *dest, const char *default_value) {
		macadd_t tmp;
		int dec_len = bin_decode(tmp.bytes, default_value, MAC_ADDR_LEN);
		if (dec_len != MAC_ADDR_LEN)
			tmp.val = 0;
		return get(key, dest, &tmp);
	}
	
	using ConfigFile::put;
	int put(const char * key, const macadd_t *src) {
		const size_t hbuf_size = MAC_ADDR_LEN * 3;
		char hbuf[hbuf_size];
		AsciiBin::binary_to_dotted(hbuf, src->bytes, hbuf_size, MAC_ADDR_LEN, 16, ':');
		return ConfigFile::put(key, hbuf);
	}
	
	int get(const char * key, meas_unit_t &dest, meas_unit_t const default_value) {
		char tmp[32];
		if (get(key, tmp, 0, 32)) {
			dest = default_value;
			return -1;
		}
		switch(atoi(tmp)) {
			case 0: dest = UNM_LITER;  break;
			case 1: dest = UNM_GALLON; break;
			case 2: dest = UNM_PINT;   break;
			case 3: dest = UNM_QUART;  break;
			default:
				dest = default_value;
				return -1;
		}
		return 0;
	}
	int put(const char * key, meas_unit_t const value) {
		int t = 0;
		switch(value) {
			case UNM_LITER:  t = 0; break;
			case UNM_GALLON: t = 1; break;
			case UNM_PINT:   t = 2; break;
			case UNM_QUART:  t = 3; break;
		}
		return put(key, t);
	}
*/
};


/****************************************************************************/

#endif /* __GPVCFGFILE_HPP_INCLUDED */
/* EOF */
