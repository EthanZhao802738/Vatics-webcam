#ifndef _C_AXC_UTILITY_H_
#define _C_AXC_UTILITY_H_

#include "axclib_data_type.h"

class AXC_API CAxcUtility
{
public:
	CAxcUtility(); 
	virtual ~CAxcUtility();

	// 字节顺序转换（大头、小头顺序）
	static axc_word   SwapUint16(axc_word w);
	static axc_dword  SwapUint32(axc_dword dw);
	static axc_ddword SwapUint64(axc_ddword ddw);
};


#endif // _C_AXC_UTILITY_H_
