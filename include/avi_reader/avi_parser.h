
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
#ifndef __AVI_PARSER_H__
#define __AVI_PARSER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <avi_reader/avi_types.h>
#include <string.h>

typedef struct
{
	list_header_t strl_list_header;
	avi_stream_header_t avi_stream_header;
	bitmap_info_header_t *bitmap_info_header;
	wave_format_ex_t *wave_format_ex;
	/* We do not handle or store the strd and strn etc. */
} strl_t;

typedef struct
{
	strl_t *array;
	size_t size;
} strl_array;

typedef struct avi_headers_info_struct
{
	riff_header_t riff_header;
	list_header_t hdrl_list_header;
	avi_main_header_t avi_main_header;
	strl_array strl;
	list_header_t movi_list_header;
	long movi_first_chunk_offset;
	chunk_header_t idx1_header;
	long idx1_first_entry_offset;
} avi_headers_info_t;

typedef struct
{
	avi_index_1_entry_t *array;
	size_t size;
} avi_index_1_entry_array;

typedef enum { UncompressedVideo = 0, CompressedVideo, PaletteChange, Audio, Unknown } ChunkInfo;

void AVIParser_InitializeAVIHeadersInfo(avi_headers_info_t *avi_header_info);
void AVIParser_ReleaseAVIHeadersInfo(avi_headers_info_t *avi_header_info);
void AVIParser_FourCC2StreamNumChunkInfo(FOURCC code, int *stream_num, ChunkInfo *chunk_info);
int AVIParser_Start(int fd, avi_headers_info_t *avi_info);
void AVIParser_PrintAVIInfo(const avi_headers_info_t *avi_info);
int AVIParser_ReadAllIndexEntries(int fd, const avi_headers_info_t *avi_info, avi_index_1_entry_array *idx1_entries);
void AVIParser_ReleaseIndexEntries(avi_index_1_entry_array *idx1_entries);
void AVIParser_PrintIndexEntries(const avi_index_1_entry_array *idx1_entries);
int AVIParser_PrintAllMoviChunk(int fd, const avi_headers_info_t *avi_info);

#ifdef __cplusplus
}
#endif

#endif


