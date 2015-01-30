/*
 * version.c
 *
 */

#include "version.h"

#define VERSION_STRING_PREFIX "INFOVER:"
#define VERSION_STRING_PREFIX_SIZE 8

#define BUILD_DATE_STRING_PREFIX "BUILDAT:"
#define BUILD_DATE_STRING_PREFIX_SIZE 8

#define STR(s) _STR(s)
#define _STR(s) #s
#define VERSION_STRING(p, a, b, c, d) p "" STR(a) "." STR(b) "/" STR(c) "." STR(d) ""
#define LAST_BUILT() BUILD_DATE_STRING_PREFIX __DATE__ " " __TIME__

static const char version_string[] = VERSION_STRING(VERSION_STRING_PREFIX,
		VERSION_MAJOR_APPLICATIVE, VERSION_MINOR_APPLICATIVE,
		VERSION_MAJOR_PROTOCOL, VERSION_MINOR_PROTOCOL);

static const char last_built[] = LAST_BUILT();

const char * get_version_string(void)
{
	return version_string + VERSION_STRING_PREFIX_SIZE;
}

const char * get_build_date(void)
{
	return last_built + BUILD_DATE_STRING_PREFIX_SIZE;
}

const char * get_description_string(void) {
	return "test application";
}
