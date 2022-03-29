#include "XStreamParse.h"

#pragma GCC diagnostic ignored "-Wunused-variable"

#define MAX_HEADER_OFFSET (1024)

//
// Bit Stream
//

typedef struct
{
	BYTE* pBuffer;
	int iTotalBits;
	int iReadBits;
	int iWriteBits;
} T_BITSTREAM;

void Bitstream_Init(T_BITSTREAM* bs, const void* pBuffer, const int iBufferSize)
{
	memset(bs, 0, sizeof(T_BITSTREAM));
	bs->pBuffer = (BYTE*) pBuffer;
	bs->iTotalBits = iBufferSize * 8;
	bs->iReadBits = 0;
}

DWORD Bitstream_ReadBits(T_BITSTREAM* bs, const int iBits)
{
	if(iBits <= 0 || iBits > 32 || (bs->iReadBits + iBits) > bs->iTotalBits)
	{
		return 0;
	}

	DWORD dwValue = 0;
	int iByteOffset = 0;
	int iBitOffset = 0;
	for(int i=0; i<iBits; i++)
	{
		iByteOffset = bs->iReadBits / 8;
		iBitOffset = bs->iReadBits % 8;

		dwValue <<= 1;
		dwValue |= (bs->pBuffer[iByteOffset] >> (7-iBitOffset)) & 1;

		bs->iReadBits ++;
	}

	return dwValue;
}

void Bitstream_WriteBits(T_BITSTREAM* bs, const DWORD dwValue, const int iBits)
{
	int iByteOffset = 0;
	int iBitOffset = 0;
	int i=0;

	if(iBits <= 0 || iBits > 32 || (bs->iWriteBits + iBits) > bs->iTotalBits)
	{
		return;
	}

	for(i=0; i<iBits; i++)
	{
		iByteOffset = bs->iWriteBits / 8;
		iBitOffset = bs->iWriteBits % 8;

		bs->pBuffer[iByteOffset] |= ((dwValue >> (iBits-i-1)) & 1) << (7-iBitOffset);

		bs->iWriteBits ++;
	}
}


//
// H264
//

DWORD H264_UE(T_BITSTREAM* bs)
{
	int iZeroBits = 0;
	for(;;)
	{
		if(bs->iReadBits >= bs->iTotalBits)
		{
			return 0;
		}
		if( Bitstream_ReadBits(bs, 1) )
		{
			break;
		}
		else
		{
			iZeroBits ++;
		}
	}
	if(0 == iZeroBits)
	{
		return 0;
	}

	DWORD dw = 1;
	for(int i=0; i<iZeroBits; i++)
	{
		dw *= 2;
	}
	return (dw + Bitstream_ReadBits(bs, iZeroBits) - 1);
}
int H264_SE(T_BITSTREAM* bs)
{
	DWORD n = H264_UE(bs);
	int value = (n+1)/2;
	if((n & 0x01)==0) // lsb is signed bit
	{
		value = -value;
	}
	return value;
}

void H264_ScalingList(int *scalingList, int sizeOfScalingList, BOOL* UseDefaultScalingMatrix, T_BITSTREAM* bs)
{
	int j, scanj;
	int delta_scale, lastScale, nextScale;

	static const unsigned char ZZ_SCAN[16]  =
	{  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
	};

	static const unsigned char ZZ_SCAN8[64] =
	{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
	   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
	   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
	   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
	};

	lastScale      = 8;
	nextScale      = 8;

	for(j=0; j<sizeOfScalingList; j++)
	{
		scanj = (sizeOfScalingList==16) ? ZZ_SCAN[j]:ZZ_SCAN8[j];

		if(nextScale!=0)
		{
			delta_scale = (int) H264_SE(bs);
			nextScale = (lastScale + delta_scale + 256) % 256;
			if(UseDefaultScalingMatrix)
			{
				*UseDefaultScalingMatrix = (BOOL) (scanj==0 && nextScale==0);
			}
		}

		if(scalingList)
		{
			scalingList[scanj] = (nextScale==0) ? lastScale:nextScale;
			lastScale = scalingList[scanj];
		}
		else
		{
			if(0 != nextScale)
				lastScale = nextScale;
		}
	}
}

// 寻找H264的开始码
int H264_FindStartCode(const BYTE* pH264Data, const int iDataSize)
{
	const BYTE* p = pH264Data;
	for(int i=0; i<iDataSize-3; i++, p++)
	{
		if(	(0x00 == p[0] && 0x00 == p[1]) && 
			(0x01 == p[2] || (0x00 == p[2] && 0x01 == p[3]))
		)
		{
			return i;
		}
	}
	return -1;
}

// 从H264序列参数表中获取图像的宽和高
BOOL H264SPS_GetImageSize(const BYTE* pNALHeader, const int iHeadSize, int* pWidth, int* pHeight, int* pFmoFlag, int* pPocType, int* pNumRefFrames)
{
	//
	// 参考 http://blog.csdn.net/heanyu/article/details/6191576, http://blog.csdn.net/heanyu/article/details/6205390
	// 参考代码：JM/ldecod/src/parset.c, vlc.c
	//
	// pic_width_in_mbs_minus1 :
	//     本句法元素加 1 后指明图像宽度，以宏块为单位： PicWidthInMbs = pic_width_in_mbs_minus1 + 1 
	//     通过这个句法元素解码器可以计算得到亮度分量以像素为单位的图像宽度： PicWidthInSamplesL = PicWidthInMbs * 16 
	// pic_height_in_map_units_minus1 :
	//     本句法元素加 1 后指明图像高度： PicHeightInMapUnits = pic_height_in_map_units_minus1 + 1  
	// num_ref_frames : 
	//     指定参考帧队列可能达到的最大长度，解码器依照这个句法元素的值开辟存储区，
	//     这个存储区用于存放已解码的参考帧，H.264 规定最多可用 16 个参考帧，本句法元素的值最大为 16。
	//     值得注意的是这个长度以帧为单位，如果在场模式下，应该相应地扩展一倍。
	// frame_mbs_only_flag :
	//     本句法元素等于 0 时表示本序列中所有图像的编码模式都是帧，没有其他编码模式存在；
	//     本句法元素等于 1 时  ，表示本序列中图像的编码模式可能是帧，也可能是场或帧场自适应，某个图像具体是哪一种要由其他句法元素决定
	// mb_adaptive_frame_field_flag :
	//     指明本序列是否属于帧场自适应模式。
	//     mb_adaptive_frame_field_flag等于1时表明在本序列中的图像如果不是场模式就是帧场自适应模式，
	//     等于0时表示本序列中的图像如果不是场模式就是帧模式 ... 表xx列举了一个序列中可能出现的编码模式
	//

	if(NULL == pNALHeader || iHeadSize < 5)
	{
		return FALSE;
	}

	T_BITSTREAM bs;
	memset(&bs, 0, sizeof(T_BITSTREAM));

	if(	0x00 == pNALHeader[0] && 
		0x00 == pNALHeader[1] &&
		0x00 == pNALHeader[2] &&
		0x01 == pNALHeader[3] )
	{
		Bitstream_Init(&bs, pNALHeader+4, iHeadSize-4);
	}
	else if(0x00 == pNALHeader[0] && 
			0x00 == pNALHeader[1] &&
			0x01 == pNALHeader[2] )
	{
		Bitstream_Init(&bs, pNALHeader+3, iHeadSize-3);
	}
	else
	{
		return FALSE;
	}

	const DWORD forbidden_zero_bit = Bitstream_ReadBits(&bs, 1);
	const DWORD nal_ref_idc = Bitstream_ReadBits(&bs, 2);
	const DWORD nal_unit_type = Bitstream_ReadBits(&bs, 5);
	if(0 != forbidden_zero_bit || 7 != nal_unit_type)
	{
		return FALSE;
	}

	// seq_parameter_set_rbsp()
	const DWORD profile_idc = Bitstream_ReadBits(&bs, 8);
	Bitstream_ReadBits(&bs, 8);
	const DWORD level_idc = Bitstream_ReadBits(&bs, 8);
	const DWORD seq_parameter_set_id = H264_UE(&bs);
	if(profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 144 )
	{
		const DWORD chroma_format_idc = H264_UE(&bs);
		if( chroma_format_idc == 3 )
		{
			Bitstream_ReadBits(&bs, 1);
		}
		H264_UE(&bs);
		H264_UE(&bs);
		Bitstream_ReadBits(&bs, 1);
		const DWORD seq_scaling_matrix_present_flag = Bitstream_ReadBits(&bs, 1);
		if( seq_scaling_matrix_present_flag )
		{
			DWORD seq_scaling_list_present_flag[ 8 ];
			for(int i = 0; i < 8; i++ )
			{
				seq_scaling_list_present_flag[ i ] = Bitstream_ReadBits(&bs, 1);
				if( seq_scaling_list_present_flag[ i ] )
				{
					if( i < 6 ) 
						H264_ScalingList(NULL, 16, NULL, &bs);
					else
						H264_ScalingList(NULL, 64, NULL, &bs);
				}
			}
		}
	}
	const DWORD log2_max_frame_num_minus4 = H264_UE(&bs);
	const DWORD pic_order_cnt_type = H264_UE(&bs);
	if( pic_order_cnt_type == 0 )
	{
		const DWORD log2_max_pic_order_cnt_lsb_minus4 = H264_UE(&bs);
	}
	else if( pic_order_cnt_type == 1 )
	{
		Bitstream_ReadBits(&bs, 1);
		H264_SE(&bs);
		H264_SE(&bs);
		const DWORD num_ref_frames_in_pic_order_cnt_cycle = H264_UE(&bs);
		for(DWORD i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
		{
			H264_SE(&bs);
		}
	}
	const DWORD num_ref_frames = H264_UE(&bs);
	const DWORD gaps_in_frame_num_value_allowed_flag = Bitstream_ReadBits(&bs, 1);
	const DWORD pic_width_in_mbs_minus1 = H264_UE(&bs);
	const DWORD pic_height_in_map_units_minus1 = H264_UE(&bs);

	const DWORD frame_mbs_only_flag = Bitstream_ReadBits(&bs, 1);
	if( !frame_mbs_only_flag )
	{
		const DWORD mb_adaptive_frame_field_flag  = Bitstream_ReadBits(&bs, 1);
	}

	const DWORD direct_8x8_inference_flag = Bitstream_ReadBits(&bs, 1);
	const DWORD frame_cropping_flag = Bitstream_ReadBits(&bs, 1);
	if( frame_cropping_flag )
	{
		const DWORD frame_crop_left_offset = H264_UE(&bs);
		const DWORD frame_crop_right_offset = H264_UE(&bs);
		const DWORD frame_crop_top_offset = H264_UE(&bs);
		const DWORD frame_crop_bottom_offset = H264_UE(&bs);
	}

	const DWORD vui_parameters_present_flag = Bitstream_ReadBits(&bs, 1);
	if( vui_parameters_present_flag )
	{
		//vui_parameters( )
		const DWORD aspect_ratio_info_present_flag = Bitstream_ReadBits(&bs, 1);
		if( aspect_ratio_info_present_flag )
		{
			const DWORD aspect_ratio_idc = Bitstream_ReadBits(&bs, 8);
			if( aspect_ratio_idc == 255/*Extended_SAR*/ )
			{
				const DWORD sar_width = Bitstream_ReadBits(&bs, 16);
				const DWORD sar_height = Bitstream_ReadBits(&bs, 16);
			}
		}
	}

	if(0 == pic_width_in_mbs_minus1 || 0 == pic_height_in_map_units_minus1)
	{
		return FALSE;
	}

	if(pWidth)
	{
		*pWidth = (pic_width_in_mbs_minus1+1)*16;
	}
	if(pHeight)
	{
		*pHeight = (pic_height_in_map_units_minus1+1)*16;
		if(2 == num_ref_frames && 0 == frame_mbs_only_flag)
		{
			*pHeight *= 2;
		}
	}
	if(pFmoFlag)
	{
		*pFmoFlag = frame_mbs_only_flag;
	}
	if(pPocType)
	{
		*pPocType = pic_order_cnt_type;
	}
	if(pNumRefFrames)
	{
		*pNumRefFrames = num_ref_frames;
	}
	return TRUE;
}

// 获取图像编码条带(Slice)的起始宏块编号
BOOL H264Slice_GetFirstMbIndex(const BYTE* pSliceHeader, const int iSliceHeaderLen, DWORD* pFirstMbIndex)
{
	T_BITSTREAM bs;
	memset(&bs, 0, sizeof(T_BITSTREAM));
	Bitstream_Init(&bs, pSliceHeader, iSliceHeaderLen);

	const DWORD forbidden_zero_bit = Bitstream_ReadBits(&bs, 1);
	const DWORD nal_ref_idc = Bitstream_ReadBits(&bs, 2);
	const DWORD nal_unit_type = Bitstream_ReadBits(&bs, 5);
	if(0 != forbidden_zero_bit || (1 != nal_unit_type && 5 != nal_unit_type))
	{
		return FALSE;
	}

	// slice_header()
	const DWORD first_mb_in_slice = H264_UE(&bs);

	if(pFirstMbIndex)
	{
		*pFirstMbIndex = first_mb_in_slice;
	}
	return TRUE;
}

//
// MPEG4
//

#ifndef FF_ASPECT_EXTENDED
#define FF_ASPECT_EXTENDED 15
#endif

#ifndef RECT_SHAPE
#define RECT_SHAPE       0
#define GRAY_SHAPE       3
#endif

static const unsigned char ff_log2_tab[256]={
        0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

static inline int av_log2(unsigned int v)
{
    int n;

    n = 0;
    if (v & 0xffff0000) {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00) {
        v >>= 8;
        n += 8;
    }
    n += ff_log2_tab[v];

    return n;
}

// 寻找MPEG4的开始码
int MPEG4_FindStartCode(const BYTE* pH264Data, const int iDataSize, BYTE cCode)
{
	const BYTE* p = pH264Data;
	for(int i=0; i<iDataSize-3; i++, p++)
	{
		if(0x00 == p[0] && 0x00 == p[1] && 0x01 == p[2] && cCode == p[3])
		{
			return i;
		}
	}
	return -1;
}
int MPEG4_FindStartCode2(const BYTE* pH264Data, const int iDataSize, BYTE cCodeMin, BYTE cCodeMax)
{
	const BYTE* p = pH264Data;
	for(int i=0; i<iDataSize-3; i++, p++)
	{
		if(0x00 == p[0] && 0x00 == p[1] && 0x01 == p[2] && (p[3] >= cCodeMin && p[3] <= cCodeMax))
		{
			return i;
		}
	}
	return -1;
}

// 从MPEG4数据中获取帧类型
BOOL MPEG4_GetFrameType(const BYTE* pData, const int iDataSize, int* pFrameType)
{
	const int iMaxLen = (iDataSize > MAX_HEADER_OFFSET) ? MAX_HEADER_OFFSET : iDataSize;
	const int iOffset = MPEG4_FindStartCode(pData, iMaxLen, 0xB6);
	if(iOffset < 0)
	{
		return FALSE;
	}
	if(pFrameType)
	{
		*pFrameType = (pData[iOffset + 4] >> 6) & 3;
	}
	return TRUE;
}

// 从MPEG4数据中获取图像的宽和高
BOOL MPEG4_GetImageSize(const BYTE* pData, const int iDataSize, int* pWidth, int* pHeight)
{
	const int iMaxLen = (iDataSize > MAX_HEADER_OFFSET) ? MAX_HEADER_OFFSET : iDataSize;
	const int iOffset = MPEG4_FindStartCode2(pData, iMaxLen, 0x20, 0x2F);
	if(iOffset < 0)
	{
		return FALSE;
	}

	DWORD width = 0, height = 0, vo_ver_id = 0;
	T_BITSTREAM bs;
	memset(&bs, 0, sizeof(T_BITSTREAM));
	Bitstream_Init(&bs, pData+iOffset+4, iDataSize-iOffset-4);

    // vol header
	Bitstream_ReadBits(&bs, 1); // random access
	Bitstream_ReadBits(&bs, 8); //vo_type
	if (Bitstream_ReadBits(&bs, 1) != 0) // is_ol_id
	{
		vo_ver_id = Bitstream_ReadBits(&bs, 4); // vo_ver_id
		Bitstream_ReadBits(&bs, 3); // vo_priority 
	}
	else
	{
		vo_ver_id = 1;
	}

	if(Bitstream_ReadBits(&bs, 4) == FF_ASPECT_EXTENDED) // aspect_ratio_info
	{
		Bitstream_ReadBits(&bs, 8); // par_width
		Bitstream_ReadBits(&bs, 8); // par_height
	}

	if(Bitstream_ReadBits(&bs, 1)) // vol control parameter
	{
		Bitstream_ReadBits(&bs, 2); // chroma_format
		Bitstream_ReadBits(&bs, 1); // low_delay
		if(Bitstream_ReadBits(&bs, 1)) // vbv parameters
		{
			Bitstream_ReadBits(&bs, 15);   // first_half_bitrate
			Bitstream_ReadBits(&bs, 1);    // marker
			Bitstream_ReadBits(&bs, 15);   // latter_half_bitrate
			Bitstream_ReadBits(&bs, 1);    // marker
			Bitstream_ReadBits(&bs, 15);   // first_half_vbv_buffer_size
			Bitstream_ReadBits(&bs, 1);    // marker
			Bitstream_ReadBits(&bs, 3);    // latter_half_vbv_buffer_size
			Bitstream_ReadBits(&bs, 11);   // first_half_vbv_occupancy
			Bitstream_ReadBits(&bs, 1);    // marker
			Bitstream_ReadBits(&bs, 15);   // latter_half_vbv_occupancy
			Bitstream_ReadBits(&bs, 1);    // marker
		}
	}

	const DWORD shape = Bitstream_ReadBits(&bs, 2);
	if(shape == GRAY_SHAPE && vo_ver_id != 1)
	{
		Bitstream_ReadBits(&bs, 4);  //video_object_layer_shape_extension
	}
	
	Bitstream_ReadBits(&bs, 1); // check marker
	
	const DWORD time_base_den = Bitstream_ReadBits(&bs, 16);
	if(0 == time_base_den)
	{
		return FALSE;
	}
	
	int time_increment_bits = av_log2(time_base_den - 1) + 1;
	if (time_increment_bits < 1)
	{
		time_increment_bits = 1;
	}

	Bitstream_ReadBits(&bs, 1); // check marker

	if (Bitstream_ReadBits(&bs, 1) != 0) //fixed_vop_rate
	{
		Bitstream_ReadBits(&bs, time_increment_bits); //time_base_num
	}

	if(shape != RECT_SHAPE)
	{
		return FALSE;
	}
	Bitstream_ReadBits(&bs, 1);   // marker
	width = Bitstream_ReadBits(&bs, 13);
	Bitstream_ReadBits(&bs, 1);   // marker
	height = Bitstream_ReadBits(&bs, 13);
	Bitstream_ReadBits(&bs, 1);   // marker

	if(pWidth)
	{
		*pWidth = width;
	}
	if(pHeight)
	{
		*pHeight = height;
	}
	return TRUE;
}
