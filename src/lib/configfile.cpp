/**
******************************************************************************
* @file    configfile.hpp
*****************************************************************************/

#define LOG_SUBSYSTEM_ID "conf"
#include <logging.hpp>

//////////////////////////////////////////////////////////////////////////////
//                         I N C L U D E S                                  //
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include "configfile.hpp"

#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

using namespace std;

#if defined(_WIN32) || defined(_WIN64)
#define strtoll  _strtoi64
#define strtoull _strtoui64
#define snprintf _snprintf
#define inet_pton InetPton
#endif

#define SIG32_MAX  2147483647
#define SIG32_MIN -2147483648
#define UNS32_MAX  4294967295


/* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
   the file. See http://code.google.com/p/inih/issues/detail?id=21 */
#define INI_ALLOW_BOM           0

/* Nonzero to allow multi-line value parsing, in the style of Python's
   ConfigParser. If allowed, ini_parse() will call the handler with the same
   name for each subsequent line parsed. */
#define INI_ALLOW_MULTILINE     0


/* Stop parsing on first error (default is to keep parsing). */
#define INI_STOP_ON_FIRST_ERROR 0

/* Don't care about session error */
#define INI_IGNORE_ERRORS       0

/* Directives on how to handle logging */
#define DUMP_KEYS_ON_LOAD       0
#define DUMP_KEYS_ON_SAVE       1
#define DUMP_KEYS_ON_SET        1

#ifndef _VBL
#define _VBL(l) std::cerr << "[filecfg " << l << "] "
#endif
#ifndef ENDL
#define ENDL <<std::endl
#endif


//////////////////////////////////////////////////////////////////////////////
//                     C L A S S    M E T H O D S                           //
//////////////////////////////////////////////////////////////////////////////


/*************************************************************************//**
**
** Constructors / Destructor
**
*****************************************************************************/

/*************************************************************************//**
**
*/
ConfigFile::ConfigFile():
	active_section(0),
	modified(false),
	update_on_destruction(false)
{
}


/*************************************************************************//**
**
*/
ConfigFile::ConfigFile(const char * filepath):
	active_section(0),
	modified(false),
	update_on_destruction(true)
{
	open(filepath);
}


/*************************************************************************//**
**
*/
ConfigFile::ConfigFile(const char * filepath, const char * section):
	active_section(0),
	modified(false),
	update_on_destruction(true)
{
	open(filepath, section);
}


/*************************************************************************//**
**
*/
ConfigFile::~ConfigFile()
{
	if (update_on_destruction) {
		if (modified)
			write();
	}
}


/*************************************************************************//**
**
** helper static functions
**
*****************************************************************************/

#define MAX_SECTION 50
#define MAX_NAME 50

/* Strip whitespace chars off end of given string, in place. Return s. */
static char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char* lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

/* Return pointer to first char c or ';' comment in given string, or pointer to
   null at end of string if neither found. ';' must be prefixed by a whitespace
   character to register as a comment. */
static char* find_char_or_comment(const char* s, char c)
{
    int was_whitespace = 0;
    while (*s && *s != c && !(was_whitespace && *s == ';')) {
        was_whitespace = isspace((unsigned char)(*s));
        s++;
    }
    return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char* strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
}


/*************************************************************************//**
**
** Parser function
**
*****************************************************************************/

int ConfigFile::ini_parse_file(FILE* file)
{
    char* line;
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;

    line = (char*)malloc(INI_MAX_LINE);
    if (!line) {
        return -1;
    }

    /* Scan through file line by line */
    while (fgets(line, INI_MAX_LINE, file) != NULL) {
        lineno++;

        start = line;
#if INI_ALLOW_BOM
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = lskip(rstrip(start));

        if (*start == ';' || *start == '#') {
            /* Per Python ConfigParser, allow '#' comments at start of line */
        }
#if INI_ALLOW_MULTILINE
        else if (*prev_name && *start && start > line) {
            /* Non-black line with leading whitespace, treat as continuation
               of previous name's value (as per Python ConfigParser). */
            if (!handler(user, section, prev_name, start) && !error)
                error = lineno;
        }
#endif
        else if (*start == '[') {
            /* A "[section]" line */
            end = find_char_or_comment(start + 1, ']');
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start && *start != ';') {
            /* Not a comment, must be a name[=:]value pair */
            end = find_char_or_comment(start, '=');
            if (*end != '=') {
                end = find_char_or_comment(start, ':');
            }
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
                end = find_char_or_comment(value, '\0');
                if (*end == ';')
                    *end = '\0';
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
#if INI_IGNORE_ERRORS
                handle_key(section, name, value);
#else
                if (!handle_key(section, name, value) && !error)
                    error = lineno;
#endif
            }
            else if (!error) {
                /* No '=' or ':' found on name[=:]value line */
                error = lineno;
            }
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

    free(line);

    return error;
}


int ConfigFile::open(const char * filepath, const char * section)
{
    FILE* file;
    int error;

	_VBL(1) << "Opening " << std::setw(24) << std::setfill(' ') << filepath ENDL;

	file_path = filepath;
	active_section = (section == 0) ? "": section;

	file = fopen(filepath, "r");
    if (!file)
        return -1;
    
	error = ini_parse_file(file);
	modified = false;
    
	fclose(file);

#if DUMP_KEYS_ON_LOAD
	// Dump (key,value) pairs to console (or log)
 	keyval_dict_t::const_iterator ii;
 	for (ii = dict.begin(); ii != dict.end(); ++ii)
		_VBL(2) << "\t" << ii->first << " = \"" << ii->second << "\"" ENDL;
#endif
	
	return error;
}


bool ConfigFile::handle_key(const char* section, const char* key, const char* value)
{
	if (active_section && strcmp(section, active_section))
		return true; // Ignored section

	dict.insert(std::pair<string,string>(key, value));
	return false;
}


/*************************************************************************//**
**
** File writing methods
**
*****************************************************************************/

int ConfigFile::write()
{
	return rewrite(file_path.c_str(), active_section);
}

int ConfigFile::write(const char * filepath)
{
	return rewrite(filepath, active_section);
}

int ConfigFile::rewrite(const char * filepath, const char * section)
{
    FILE* file;
    int error = 0;

	_VBL(1) << "Rewriting file \"" << filepath << "\" section [" << section << "]" ENDL;

	// Try to open the file for update
	file = fopen(filepath, "w+");
	if (!file)
		return -1;

	// Write active section name, if available
	if ((section != NULL) && (*section != 0))
		fprintf(file, "[%s]\n", section);

	// Write (key,value) pairs
	keyval_dict_t::const_iterator ii;
	for (ii = dict.begin(); ii != dict.end(); ++ii) {
		fprintf(file, "%s = %s\n", ii->first.c_str(), ii->second.c_str());
#if DUMP_KEYS_ON_SAVE
		_VBL(2) << "    " << ii->first << " = \"" << ii->second << "\"" ENDL;
#endif
	}
	
	fclose(file);
	
	if (error == 0)
		modified = false;
	
	return error;
}


/*************************************************************************//**
**
** Load methods
**
*****************************************************************************/

/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, char * dest, const char * default_value, size_t max_size)
{
	keyval_dict_t::iterator elem = dict.find(key);
	
	if (elem == dict.end()) {
		if (default_value)
			strncpy0(dest, default_value, max_size);
		return -1;
	}
	
	strncpy0(dest, elem->second.c_str(), max_size);
	return 0;
}

/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, int64_t &dest, int64_t default_value)
{
	keyval_dict_t::iterator elem = dict.find(key);
	char * eptr;
	
	if (elem == dict.end()) {
		dest = default_value;
		return -1;
	}

	const char * const s = elem->second.c_str();
	int64_t tval = strtoll(s, &eptr, 10);
	if (*eptr != 0) {
		dest = default_value;
		return -1;
	}
	
	dest = tval;
	return 0;
}


/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, int32_t &dest, int32_t default_value)
{
	int64_t t;
	int err = get(key, t, 0);
	if (err) {
		dest = default_value;
		return err;
	}
	if (t > SIG32_MAX)
		dest = (int32_t)SIG32_MAX;
	else
	if (t < SIG32_MIN)
		dest = (int32_t)SIG32_MIN;
	else
		dest = (int32_t)t;
	return 0;
}


/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, int16_t &dest, int16_t default_value)
{
	int64_t t;
	int err = get(key, t, 0);
	if (err) {
		dest = default_value;
		return err;
	}
	if (t > SHRT_MAX)
		dest = SHRT_MAX;
	else
	if (t < SHRT_MIN)
		dest = SHRT_MIN;
	else
		dest = (int16_t)t;
	return 0;
}


/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, int8_t  &dest, int8_t  default_value)
{
	int64_t t;
	int err = get(key, t, 0);
	if (err) {
		dest = default_value;
		return err;
	}
	if (t > SCHAR_MAX)
		dest = SCHAR_MAX;
	else
	if (t < SCHAR_MIN)
		dest = SCHAR_MIN;
	else
		dest = (int8_t)t;
	return 0;
}



/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, uint64_t &dest, uint64_t default_value)
{
	keyval_dict_t::iterator elem = dict.find(key);
	char * eptr;
	int base = 10;
	
	if (elem == dict.end()) {
		dest = default_value;
		return -1;
	}

	const char * s = elem->second.c_str();
	// Skip leading spaces
	while (*s == ' ') s++;
	 // Check for hexadecimal value
	if ((s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X')))
		base = 16;
	// Convert value
	dest = strtoull(s, &eptr, base);
	// If string contains invalid characters, set default value
	// and return with an error code
	if ((*eptr != 0) && (*eptr != ' ')) {
		dest = default_value;
		return -1;
	}
	return 0;
}


/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, uint32_t &dest, uint32_t default_value)
{
	uint64_t t;
	int err = get(key, t, 0);
	if (err) {
		dest = default_value;
		return err;
	}
	if (t > UNS32_MAX)
		dest = (uint32_t)UNS32_MAX;
	else
		dest = (uint32_t)t;
	return 0;
}


/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, uint16_t &dest, uint16_t default_value)
{
	uint64_t t;
	int err = get(key, t, 0);
	if (err) {
		dest = default_value;
		return err;
	}
	if (t > USHRT_MAX)
		dest = USHRT_MAX;
	else
		dest = (uint16_t)t;
	return 0;
}


/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, uint8_t  &dest, uint8_t  default_value)
{
	uint64_t t;
	int err = get(key, t, 0);
	if (err) {
		dest = default_value;
		return err;
	}
	if (t > UCHAR_MAX)
		dest = UCHAR_MAX;
	else
		dest = (uint8_t)t;
	return 0;
}


/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, bool &dest, bool default_value)
{
	uint64_t t;
	int err = get(key, t, 0);
	if (err) {
		dest = default_value;
		return err;
	}
	dest = (t != 0) ? true : false;
	return 0;
}


/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, double &dest, double default_value)
{
	keyval_dict_t::iterator elem = dict.find(key);
	char * eptr;
	
	if (elem == dict.end()) {
		dest = default_value;
		return -1;
	}

	const char * const s = elem->second.c_str();
	double tval = strtod(s, &eptr);
	if (*eptr != 0) {
		dest = default_value;
		return -1;
	}
	
	dest = tval;
	return 0;
}


/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, float  &dest, float  default_value)
{
	double t;
	int err = get(key, t, 0);
	if (err) {
		dest = default_value;
		return err;
	}
	if (t > FLT_MAX)
		dest = FLT_MAX;
	else
	if (t < FLT_MIN)
		dest = FLT_MIN;
	else
		dest = (float)t;
	return 0;
}


/*************************************************************************//**
**
*/
int ConfigFile::get(const char * key, struct in_addr *dest, const struct in_addr *default_value)
{
	keyval_dict_t::iterator elem = dict.find(key);
	
	if (elem == dict.end()) {
		*dest = *default_value;
		return -1;
	}

	const char * const s = elem->second.c_str();
	struct in_addr tmp;

	if (!inet_pton(AF_INET, s, &tmp)) {
		*dest = *default_value;
		return -1;
	}
	*dest = tmp;
	return 0;
}

int ConfigFile::get(const char * key, struct in_addr *dest, const char * const default_value)
{
	struct in_addr tmp;

	if (!inet_pton(AF_INET, default_value, &tmp))
		tmp.s_addr = 0;
	
	return get(key, dest, &tmp);
}


/*************************************************************************//**
**
*/
int ConfigFile::get_raw(const char * key, void * dest, size_t expected_size)
{
	keyval_dict_t::iterator elem = dict.find(key);
	
	if (elem == dict.end())
		return -1;
	
	unsigned dec_len = bin_decode(static_cast<uint8_t *>(dest),
									elem->second.c_str(), expected_size);
	if (dec_len != expected_size)
		return -1;
	else
		return 0;
}



/*************************************************************************//**
**
** Save methods
**
*****************************************************************************/

/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, const char * value)
{
#if DUMP_KEYS_ON_SET
#define _DUMP_KEYVAL(k, v, m) \
	_VBL(2) << name << ":" << k << " = \"" << v << "\" (" << m << ")" ENDL
	const size_t sep = file_path.rfind("/");
	const string name = (sep==string::npos) ? file_path : file_path.substr(sep + 1);
#else
#define _DUMP_KEYVAL(k, v, m)
#endif
	keyval_dict_t::iterator elem = dict.find(key);
	if (elem == dict.end()) {
		modified = true;
		dict.insert(std::pair<string,string>(key, value));
		_DUMP_KEYVAL(key, value, "created");
		return 0;
	}

	const char * orig = elem->second.c_str();
	if (strcmp(orig, value) != 0) {
		modified = true;
		elem->second = value;
		_DUMP_KEYVAL(elem->first, elem->second, "modified");
		return 0;
	} else {
		_DUMP_KEYVAL(elem->first, elem->second, "unchanged");
	}

	return 1;
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, int64_t const value)
{
	char sbuff[STR_BUFF_LEN];
	snprintf(sbuff, STR_BUFF_LEN, "%" PRId64 "", value);
	return put(key, sbuff);
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, int32_t const value)
{
	char sbuff[STR_BUFF_LEN];
	snprintf(sbuff, STR_BUFF_LEN, "%d", value);
	return put(key, sbuff);
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, int16_t const value)
{
	return put(key, (int32_t)value);
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, int8_t  const value)
{
	return put(key, (int32_t)value);
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, uint64_t const value)
{
	char sbuff[STR_BUFF_LEN];
	snprintf(sbuff, STR_BUFF_LEN, "%" PRIu64 "", value);
	return put(key, sbuff);
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, uint32_t const value)
{
	char sbuff[STR_BUFF_LEN];
	snprintf(sbuff, STR_BUFF_LEN, "%u", value);
	return put(key, sbuff);
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, uint16_t const value)
{
	return put(key, (uint32_t)value);
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, uint8_t const value)
{
	return put(key, (uint32_t)value);
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, bool const value)
{
	return put(key, value ? "1" : "0");
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, double const value)
{
	char sbuff[STR_BUFF_LEN];
	snprintf(sbuff, STR_BUFF_LEN, "%g", value);
	return put(key, sbuff);
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, float const value)
{
	return put(key, (double)value);
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, const struct in_addr *src)
{
	char sbuff[INET_ADDRSTRLEN];
	return put(key, inet_ntop(AF_INET, (void *)src, sbuff, INET_ADDRSTRLEN));
}


/*************************************************************************//**
**
*/
int ConfigFile::put(const char * key, const struct in6_addr *src)
{
	char sbuff[INET_ADDRSTRLEN];
	return put(key, inet_ntop(AF_INET6, (void *)src, sbuff, INET_ADDRSTRLEN));
}


/*************************************************************************//**
**
*/
int ConfigFile::put_raw(const char * key, const void * const src, size_t const data_size)
{
	size_t const hbuf_size = data_size * 2 + 1;
	char *hbuf = static_cast<char *>(calloc(hbuf_size, 1));
	if (hbuf == 0)
		return -1;
	bin_encode(hbuf, static_cast<const uint8_t*>(src), hbuf_size, data_size);
	return put(key, hbuf);
}

