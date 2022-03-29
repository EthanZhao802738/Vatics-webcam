
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
#ifndef __AVI_READER_H__
#define __AVI_READER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <avi_reader/avi_parser.h>

typedef struct avi_reader_handle_t
{
	int fd_; /**< The file descriptor of AVI file. */
	size_t current_movi_chunk_index_; /**< The index to point one movi chunk. */
	avi_headers_info_t *avi_info_; /**< The pointer to point the AVI information structure. */
	avi_index_1_entry_array idx1_entries_; /**< The queue to store the entries of AVI 1.0 index. */
} avi_reader_handle_t;

void AVIReader_Init(avi_reader_handle_t *);
void AVIReader_Release(avi_reader_handle_t *);
int AVIReader_LoadAVIFile(avi_reader_handle_t *, const char* filename);
int AVIReader_GetSample(avi_reader_handle_t *, char* buf, size_t buf_len, size_t *data_len, int *stream_num, ChunkInfo *chunk_info);

#ifdef __cplusplus
}
#endif

#endif

