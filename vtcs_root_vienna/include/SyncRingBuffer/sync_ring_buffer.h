
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
#ifndef SYNC_RING_BUFFER_H
#define SYNC_RING_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

//#define VIENNA_SSM_CORE

typedef struct
{
	int idx;
    unsigned char* buffer;
	unsigned char* buffer_phys_addr;
} srb_buffer_t;

typedef srb_buffer_t SRB_BUFFER_T;
typedef struct srb_handle_t srb_handle_t;
typedef struct srb_handle_t SRB_HANDLE_T;

/**
 * @brief Create a SyncRingBuffer writer
 *
 * @param[in] name It specifies the object to be created or opened.
 * @param[in] buf_size The buffer size of the object.
 * @param[in] ring_bufer_num The total number of slots within this ring buffer.
 * @return The handle of sync ring buffer.
 */
SRB_HANDLE_T* SRB_InitWriter(const char* name, unsigned int buf_size, int ring_buf_num);

/**
 * @brief Create a SyncRingBuffer reader
 *
 * @param[in] name It specifies the object to be created or opened.
 * @return The handle of sync ring buffer.
 */
SRB_HANDLE_T* SRB_InitReader(const char* name);

/**
 * @brief Function to explicitly recycle SyncRingBuffer handle.
 *
 * @param[in] handle The handle of sync ring buffer.
 */
int SRB_Release(SRB_HANDLE_T* ptHandle);

/**
 * @brief Deliver the current 'srb buffer'to those related readers and allocate a new 'srb buffer'.
 *
 * @param[in] handle The handle of sync shared memory.
 * @param[in] ssm_buf The pointer of srb_buffer_t structure.
 */
int SRB_SendGetWriterBuff(SRB_HANDLE_T* ptHandle, SRB_BUFFER_T* ptSrbBuf);

/**
 * @brief Deliver the current 'srb buffer'to those related readers and check wether writer will over reader.
 *
 * @param[in] handle The handle of sync shared memory.
 * @param[in] ssm_buf The pointer of srb_buffer_t structure.
 */
int SRB_WriterCheckReader(SRB_HANDLE_T* ptHandle, SRB_BUFFER_T* ptSrbBuf);

/**
 * @brief Release the current 'srb buffer'and receive a new 'srb buffer' from the writer.
 *
 * @param[in] handle The handle of sync shared memory.
 * @param[in] ssm_buf The pointer of srb_buffer_t structure.
 */
int SRB_ReturnReceiveReaderBuff(SRB_HANDLE_T* ptHandle, SRB_BUFFER_T* ptSrbBuf);

/**
 * @brief It is used to relase a 'srb buffer' (For reader only)
 *
 * @param[in] handle The handle of sync ring buffer.
 * @param[in] ssm_buf The pointer of srb_buffer_t structure.
 */
int SRB_ReturnReaderBuff(SRB_HANDLE_T* ptHandle, SRB_BUFFER_T* ptSrbBuf);

/**
 * @brief Release the current 'srb buffer'and peek if a new 'srb buffer' is ready, if not ready, just quit.
 *
 * @param[in] ptHandle The handle of sync ring buffer.
 * @param[in] ssm_buf The pointer of srb_buffer_t structure.
 */
int SRB_QueryReaderBuff(srb_handle_t* ptHandle, srb_buffer_t* srb_buf);

/**
 * @brief Wake up the reader while it is waiting for a buffer coming from the writer.
 *
 * @param[in] ptHandle The handle of sync ring buffer.
 */
int SRB_WakeupReader(SRB_HANDLE_T* ptHandle);

/**
 * @brief Clear writer buffer.
 *
 * @param[in] ptHandle The handle of sync ring buffer.
 */
int SRB_ClearWriterBuffer(SRB_HANDLE_T* handle);

#ifdef __cplusplus
}
#endif

#endif
