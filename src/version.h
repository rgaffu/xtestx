#ifndef _VERSION_H_
#define _VERSION_H_

#define APPLICATION_MAGIC_NUMBER  0xB0FAB0FA

#define VERSION_MAJOR_APPLICATIVE		0
#define VERSION_MINOR_APPLICATIVE		1

#define VERSION_MAJOR_PROTOCOL			0
#define VERSION_MINOR_PROTOCOL			1

#ifdef __cplusplus
extern "C" {
#endif

const char * get_version_string(void);
const char * get_build_date(void);
const char * get_description_string(void);

#ifdef __cplusplus
}
#endif
#endif /* _VERSION_H_ */

