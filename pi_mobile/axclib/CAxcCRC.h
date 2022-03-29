#ifndef _C_AXC_CRC_H_
#define _C_AXC_CRC_H_

#include "axclib_data_type.h"

class AXC_API CAxcCRC
{
public:
	CAxcCRC() {}
	virtual ~CAxcCRC() {}

	static axc_word Make(const void* pData, axc_i32 iLength, axc_word wOrgCrc = 0);

	static axc_word Make_ITU(const void* pData, axc_i32 iLength, axc_word wOrgCrc = 0);

	static axc_word Make_CCITT(const void* pData, axc_i32 iLength, axc_word wOrgCrc = 0);
};

#endif // _C_AXC_CRC_H_
