#ifndef _XSTREAM_PARSE_H_20110323_
#define _XSTREAM_PARSE_H_20110323_

#include "axclib.h"

// 从H264序列参数表中获取图像的宽和高
BOOL H264SPS_GetImageSize(const BYTE* pNALHeader, const int iHeadSize, int* pWidth, int* pHeight, int* pFmoFlag = NULL, int* pPocType = NULL, int* pNumRefFrames = NULL);

// 获取图像编码条带(Slice)的起始宏块编号
BOOL H264Slice_GetFirstMbIndex(const BYTE* pSliceHeader, const int iSliceHeaderLen, DWORD* pFirstMbIndex);

// 寻找H264的开始码
int H264_FindStartCode(const BYTE* pH264Data, const int iDataSize);

// 从MPEG4数据中获取帧类型
BOOL MPEG4_GetFrameType(const BYTE* pData, const int iDataSize, int* pFrameType);

// 从MPEG4数据中获取图像的宽和高
BOOL MPEG4_GetImageSize(const BYTE* pData, const int iDataSize, int* pWidth, int* pHeight);

// 寻找MPEG4的开始码
int MPEG4_FindStartCode(const BYTE* pH264Data, const int iDataSize, BYTE cCode);
int MPEG4_FindStartCode2(const BYTE* pH264Data, const int iDataSize, BYTE cCodeMin, BYTE cCodeMax);


#endif // #ifndef _XSTREAM_PARSE_H_20110323_
