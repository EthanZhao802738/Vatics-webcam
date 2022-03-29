#ifndef _C_AXC_BASE64_H_
#define _C_AXC_BASE64_H_

#include "axclib_data_type.h"

class AXC_API CAxcBase64
{
public:
	CAxcBase64() {}
	virtual ~CAxcBase64() {}

	static axc_i32 Encode(
		const axc_byte *pInputData,
		axc_i32 iInputLength,
		char* szEncodeData,
		axc_i32 iEncodeBufferLength);

	static axc_i32 Decode(
		const char *pInputData,
		axc_i32 iInputLength,
		axc_byte *pDecodeData,
		axc_i32 iDecodeBufferLength);

	static AXC_INLINE axc_dword GetEncodeOutputLength(axc_dword dwInputLength) { return (4 * ((dwInputLength + 2) / 3)); }
	static AXC_INLINE axc_dword GetDecodeOutputLength(axc_dword dwInputLength) { return (dwInputLength / 4 * 3); }
};

#endif // _C_AXC_BASE64_H_
