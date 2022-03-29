
/*
 *******************************************************************************
 *  Copyright (c) 2010-2015 VATICS Inc. All rights reserved.
 *
 *  +-----------------------------------------------------------------+
 *  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
 *  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
 *  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
 *  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
 *  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
 *  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
 *  |                                                                 |
 *  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
 *  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
 *  | VATICS INC.                                                     |
 *  +-----------------------------------------------------------------+
 *
 *******************************************************************************
 */ 
#ifndef __AVI_TYPES_H__
#define __AVI_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <avi_reader/avi_utils.h>
#include <stdint.h>

#define PACKED_STRUCT   __attribute__((__packed__))

#define FOURCC_RIFF     (MAKEFOURCC('R','I','F','F'))
#define FOURCC_LIST     (MAKEFOURCC('L','I','S','T'))   /* list structure */
#define FOURCC_AVI_     (MAKEFOURCC('A','V','I',' '))
#define FOURCC_hdrl     (MAKEFOURCC('h','d','r','l'))   /* avih and several strl */
#define FOURCC_avih     (MAKEFOURCC('a','v','i','h'))   /* avi common feature */
#define FOURCC_strl     (MAKEFOURCC('s','t','r','l'))   /* stream header + stream format */
#define FOURCC_strh     (MAKEFOURCC('s','t','r','h'))   /* stream header */
#define FOURCC_strf     (MAKEFOURCC('s','t','r','f'))   /* stream format */
#define FOURCC_movi     (MAKEFOURCC('m','o','v','i'))   /* media data */
#define FOURCC_idx1     (MAKEFOURCC('i','d','x','1'))   /* index, record the position and size in the file for every data */
#define FOURCC_JUNK     (MAKEFOURCC('J','U','N','K'))   /* player will ignore it, we use it for timestamp */

#define FOURCC_vids     (MAKEFOURCC('v','i','d','s'))   /* video */
#define FOURCC_auds     (MAKEFOURCC('a','u','d','s'))   /* audio */

#define FOURCC_BI_RGB 0x00000000
#define FOURCC_BI_BITFIELDS 0x00000003

#define AVIF_COPYRIGHTED 0x00020000
#define AVIF_HASINDEX 0x00000010		/* Index at end of file? */
#define AVIF_ISINTERLEAVED 0x00000100
#define AVIF_MUSTUSEINDEX 0x00000020
#define AVIF_TRUSTCKTYPE 0x00000800		/* Use CKType to find key frames? */
#define AVIF_WASCAPTUREFILE 0x00010000

#define AVISF_DISABLED 0x00000001
#define AVISF_VIDEO_PALCHANGES 0x00010000

#define WAVE_FORMAT_PCM 0x0001			/* PCM */
#define WAVE_FORMAT_MPEG 0x0050			/* MPEG Layer 1,2 */
#define WAVE_FORMAT_MPEGLAYER3 0x0055		/* MPEG Layer 3 */
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE		/* SubFormat */

#define AVIIF_LIST       0x00000001
#define AVIIF_KEYFRAME   0x00000010 
#define AVIIF_NO_TIME    0x00000100
#define AVIIF_COMPRESSOR 0x0FFF0000

typedef uint32_t FOURCC;
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef int32_t SDWORD;
typedef int16_t SHORT;

typedef struct
{
	FOURCC fcc;		/* RIFF */
	DWORD  dwRiffSize;
	DWORD  dwFileType;
} riff_header_t;

typedef struct
{
	FOURCC fcc;		/* LIST */
	DWORD  dwListSize;
	DWORD  dwListType;
} list_header_t;

typedef struct
{
	FOURCC fcc;
	DWORD  dwChunkSize;
} chunk_header_t;

typedef struct
{
	FOURCC fcc;
	DWORD  dwChunkByte;
	DWORD  dwMicroSecPerFrame;
	DWORD  dwMaxBytesPerSec;
	DWORD  dwPaddingGranularity;
	DWORD  dwFlags;
	DWORD  dwTotalFrames;
	DWORD  dwInitialFrames;
	DWORD  dwStreams;
	DWORD  dwSuggestedBufferSize;
	DWORD  dwWidth;
	DWORD  dwHeight;
	DWORD  dwReserved[4];
} avi_main_header_t;

typedef struct
{
	FOURCC fcc;
	DWORD  dwChunkByte;
	FOURCC fccType;
	FOURCC fccHandler;
	DWORD  dwFlags;
	WORD   wPriority;
	WORD   wLanguage;
	DWORD  dwInitialFrames;
	DWORD  dwScale;
	DWORD  dwRate;
	DWORD  dwStart;
	DWORD  dwLength;
	DWORD  dwSuggestedBufferSize;
	SDWORD dwQuality;
	DWORD  dwSampleSize;
	struct {
		SHORT left;
		SHORT top;
		SHORT right;
		SHORT bottom;
	} rcFrame;
} avi_stream_header_t;

typedef struct
{
	FOURCC fcc;
	DWORD  dwChunkByte;
	DWORD  biSize;
	SDWORD biWidth;
	SDWORD biHeight;
	WORD   biPlanes;
	WORD   biBitCount;
	DWORD  biCompression;
	DWORD  biSizeImage;
	SDWORD biXPelsPerMeter;
	SDWORD biYPelsPerMeter;
	DWORD  biClrUsed;
	DWORD  biClrImportant;
} bitmap_info_header_t;

typedef struct {
	FOURCC fcc;
	DWORD  dwChunkByte;
	WORD   wFormatTag;
	WORD   nChannels;
	DWORD  nSamplesPerSec;
	DWORD  nAvgBytesPerSec;
	WORD   nBlockAlign;
	WORD   wBitsPerSample;
	WORD   cbSize;
} PACKED_STRUCT wave_format_ex_t;

typedef struct {
	DWORD dwChunkId;
	DWORD dwFlags;
	DWORD dwOffset;
	DWORD dwSize;
} avi_index_1_entry_t;

#ifdef __cplusplus
}
#endif

#endif

