#ifndef _C_AXC_TIME_H_
#define _C_AXC_TIME_H_

#include "axclib_data_type.h"

class AXC_API CAxcTime
{
public:
	CAxcTime();
	virtual ~CAxcTime();

	// 毫秒级延时
	static void SleepMilliSeconds(axc_dword dwMilliSeconds);
	// 秒级延时
	static void SleepSeconds(axc_dword dwSeconds);

	// 获取当前系统时间，以秒为单位的，从1970-1-1开始的UTC时间
	static axc_dword GetCurrentUTCTime32(); // 32bit 系统UTC时间
	static axc_ddword GetCurrentUTCTime64(); // 64bit 系统UTC时间
	static double GetCurrentUTCTimeMs(); // 带毫秒数的系统UTC时间，整数位为秒数，小数位为毫秒数

	// 获取本机系统时间（叠加时区后的时间）
	static void GetCurrentLocalTime(AXC_T_SYSTEM_TIME* pSysTime);

	// 本机系统时间（AXC_SYSTEM_TIME） 和 UTC时间（time_t） 之间的转换
	static axc_ddword LocalTimeToUTC(const AXC_T_SYSTEM_TIME* pSysTime);
	static AXC_T_SYSTEM_TIME UTCToLocalTime(const axc_ddword ddwUTCTime64);

	// 高精度CPU时间（微秒级，精确到小数点后6位，整数位为秒数）
	static double GetCurrentCPUTime();
};


#endif // _C_AXC_TIME_H_
