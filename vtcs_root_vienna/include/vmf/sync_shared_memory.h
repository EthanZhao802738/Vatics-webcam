
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
#ifndef SYNC_SHARED_MEMORY_H
#define SYNC_SHARED_MEMORY_H

#include <stdint.h>
#include <MemBroker/mem_broker.h>

#ifdef __cplusplus
extern "C" {
#endif

//! can't be larger than 32
#define SSM_BUFF_BAG_SIZE_MAX 20 

 //! can't be larger than 32, and suggested to be less than SSM_BUFF_BAG_SIZE_MAX
#define SSM_RING_SIZE_MAX 16    

#define SSM_RING_SIZE_DEFAULT  2

#define SSM_BUFFER_AUTO_ALLOCATE 0

typedef struct ssm_handle_t ssm_handle_t;
typedef struct ssm_handle_t SSM_HANDLE_T;
typedef void (*SSM_BUFFER_SET)(unsigned char* virt_addr, unsigned int buf_size, void* pUserData);

/*
 * ssm write scheme
 */
typedef enum
{
	//! ssm write scheme: single buffer 
	VMF_SSM_WRITER_SINGLE_BUFFER = 0,

	//! ssm write scheme: multiple buffer 
	VMF_SSM_WRITER_MULTPLE_BUFFER,

	//! ssm write scheme: max
	VMF_SSM_WRITER_SCHEME_MAX
} VMF_SSM_WRITER_SCHEME;

/*
 * A data structure for ssm_buffer
 */
typedef struct
{
	//! A data for ssm buffer index
	int idx;

	//! A data for ssm buffer
	unsigned char* buffer;

	//! A data for ssm buffer physical address
	unsigned char* buffer_phys_addr;
} ssm_buffer_t;
typedef ssm_buffer_t SSM_BUFFER_T;

/*
 * A data structure for ssm write init option
 */
typedef struct
{
	//! A pointer for name whitch specifies the object to be created or opened
	const char* name;   

	//! A data for SSM buffer size
	unsigned int buf_size;

	//! A data for shared info (1: Inter-process, 0: intra-process.)
	unsigned int pshared;

	//! A data for writer scheme (the writer could hold single buffer or multiple buffers)
	VMF_SSM_WRITER_SCHEME writer_scheme;

	//! A data for ring buffer number.
	//! 0: default ring size is SSM_RING_SIZE_DEFAULT, 
	//! non-0: the ring size, can not exceed SSM_RING_SIZE_MAX.
	unsigned int ring_buffer_num;

	//! A data for the pre-allocate buffer number. 
	//! SSM_BUFFER_AUTO_ALLOCATE: pre-allocate buffers of the ring size, and auto-allocate alternate buffers on demand.
	//! other value: the pre-allocate buffer number, should be eq or larger than the ring size, only active when writer_scheme is VMF_SSM_WRITER_SINGLE_BUFFER
	unsigned int max_buffer_num;

	//! A data for alignment type
	vmf_align_type alignment;

	//! A data for callback function (the callback function to setup SSM buffer)
	SSM_BUFFER_SET fp_setup_buffer;

	//! A pointer data for userdata
	void* pUserData;
} ssm_writer_init_option_t;
typedef ssm_writer_init_option_t SSM_WRITER_INIT_OPTION_T;

/**
 * @brief Create a SyncSharedMemory writer
 *
 * @param[in] init_opt The pointer of ssm_writer_init_option_t.
 * @return The handle of sync shared memory.
 */
SSM_HANDLE_T* SSM_Writer_Init(const SSM_WRITER_INIT_OPTION_T* ptInitOpt);

/**
 * @brief Create a SyncSharedMemory reader
 *
 * @param[in] name It specifies the object to be created or opened.
 * @param[in] pshared Sharing memory between processes or not.
 * @return The handle of sync shared memory.
 */
SSM_HANDLE_T* SSM_Reader_Init(const char* pszName, int pshared);

/**
 * @brief Function to explicitly recycle SyncSharedMemory handle.
 *
 * @param[in] handle The handle of sync shared memory.
 */
int SSM_Release(SSM_HANDLE_T* ptHandle);

/**
 * @brief Deliver the current 'ssm buffer'to tose related readers and allocate a new 'ssm buffer'.
 *
 * @param[in] handle The handle of sync shared memory.
 * @param[in] ssm_buf The pointer of ssm_buffer_t structure.
 *
 * @note input parameters are only checked in DEBUG mode by assert
 */
int SSM_Writer_SendGetBuff(SSM_HANDLE_T* ptHandle, SSM_BUFFER_T* ptSsmBuf);
int SSM_Writer_ReturnBuff(SSM_HANDLE_T* ptHandle, SSM_BUFFER_T* ptSsmBuf);

/**
 * @brief Release the current 'ssm buffer'and receive a new 'ssm buffer' from the writer.
 *
 * @param[in] handle The handle of sync shared memory.
 * @param[in] ssm_buf The pointer of ssm_buffer_t structure.
 *
 * @note input parameters are only checked in DEBUG mode by assert
 */
int SSM_Reader_ReturnReceiveBuff(SSM_HANDLE_T* ptHandle, SSM_BUFFER_T* ptSsmBuf);

/**
 * @brief It is used to relase a 'ssm buffer' (For reader only)
 *
 * @param[in] handle The handle of sync shared memory.
 * @param[in] ssm_buf The pointer of ssm_buffer_t structure.
 */
int SSM_Reader_ReturnBuff(SSM_HANDLE_T* ptHandle, SSM_BUFFER_T* ptSsmBuf);

/**
 * @brief Wake up the reader while it is waiting for a buffer coming from the writer.
 *
 * @param[in] handle The handle of sync shared memory.
 */
int SSM_Reader_Wakeup(SSM_HANDLE_T* ptHandle);

/**
 * @brief It is used to reset buffer by callback function
 *
 * @param[in] handle The handle of sync shared memory.
 * @param[in] buf_set_cb The callback function to setup SSM buffer
 */
void SSM_Writer_ResetBuff(SSM_HANDLE_T* ptHandle, SSM_BUFFER_SET pfnBufSetCb);

/**
 * @brief It is used to reset buffer
 *
 * @param[in] handle The handle of sync shared memory.
 */
int SSM_Writer_ClearBuff(ssm_handle_t* handle);

#ifdef __cplusplus
}
#endif

#endif
