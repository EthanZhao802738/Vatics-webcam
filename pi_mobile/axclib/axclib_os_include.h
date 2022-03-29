#ifndef _C_AXC_OS_INCLUDE_H_
#define _C_AXC_OS_INCLUDE_H_

#include "axclib_os_check.h"

#if defined(AXC_OS_WINDOWS)
	#include <winsock2.h>
	#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <errno.h>

#if defined(AXC_OS_WINDOWS)
	#include <time.h>
	#include <io.h>
	#include <direct.h>
#elif defined(AXC_OS_LINUX)
	#include <stdint.h>
	#include <pthread.h>
	#include <errno.h>
	#include <signal.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <termios.h>
	#include <netdb.h>
	#include <dirent.h>
	#include <fnmatch.h>
	#include <stdarg.h>
	#include <sys/time.h>
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <net/if.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#endif // _C_AXC_OS_INCLUDE_H_
