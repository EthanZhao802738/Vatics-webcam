#ifndef _C_AXC_MD5_H_
#define _C_AXC_MD5_H_

#include "axclib_data_type.h"

class AXC_API CAxcMD5
{
public:
	CAxcMD5() {}
	virtual ~CAxcMD5() {}

	static void Make(const void* pData, axc_dword dwDataBytes, axc_byte abyOutBuffer[16]);

	static axc_bool MD5ToString(const axc_byte abyMD5[16], char* szString, const axc_dword dwStringBufferSize);
	static axc_bool StringToMD5(const char* szString, axc_byte abyMD5[16]);
};

#endif // _C_AXC_MD5_H_
