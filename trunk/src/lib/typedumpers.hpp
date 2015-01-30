#ifndef __TYPEDUMPERS_HPP
#define __TYPEDUMPERS_HPP

#include <iostream>
#include <netinet/in.h>
#include <time.h>

extern std::ostream& operator<<(std::ostream& w, const sockaddr*            );
extern std::ostream& operator<<(std::ostream& w, const sockaddr_in*         );
extern std::ostream& operator<<(std::ostream& w, const itimerspec&          );
// extern std::ostream& operator<<(std::ostream& w, const macadd_t&            );
// extern std::ostream& operator<<(std::ostream& w, const device_info_s&       );
// extern std::ostream& operator<<(std::ostream& w, const firmware_info_ext_s& );
// extern std::ostream& operator<<(std::ostream& w, const ext_device_info_s&   );

#endif /* __TYPEDUMPERS_HPP */
