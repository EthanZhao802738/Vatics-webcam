
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
#ifndef MEM_BROKER_H
#define MEM_BROKER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	VMF_ALIGN_TYPE_DEFAULT = 0,
	VMF_ALIGN_TYPE_2_BYTE,
	VMF_ALIGN_TYPE_4_BYTE,
	VMF_ALIGN_TYPE_8_BYTE,
	VMF_ALIGN_TYPE_16_BYTE,
	VMF_ALIGN_TYPE_32_BYTE,
	VMF_ALIGN_TYPE_64_BYTE,
	VMF_ALIGN_TYPE_128_BYTE,
	VMF_ALIGN_TYPE_256_BYTE,
	VMF_ALIGN_TYPE_512_BYTE,
	VMF_ALIGN_TYPE_1024_BYTE,
	VMF_ALIGN_TYPE_2048_BYTE,
	VMF_ALIGN_TYPE_4096_BYTE,
	VMF_ALIGN_TYPE_8192_BYTE,
	VMF_ALIGN_TYPE_16384_BYTE,
	VMF_ALIGN_TYPE_32768_BYTE
} vmf_align_type;
typedef vmf_align_type VMF_ALIGN_TYPE;

/**
 * @brief Allocate memory from EDMC.
 *
 * @param[in] size Allocates size bytes
 * @param[in] alignment the start address alignment. Please check the hardware restriction.
 * @return A pointer to the allocated memory
 */
void* MemBroker_GetMemory(unsigned int size, VMF_ALIGN_TYPE alignment);

/**
 * @brief Free memory to EDMC.
 *
 * @param[in] A pointer to the allocated memory.
 */
void MemBroker_FreeMemory(void*);

/**
 * @brief Translate a virtual address to a physical address
 *
 * @param[in] A pointer to the virtual address of this allocated memory.
 * @return A pointer to the physical address of this allocated memory.
 */
void* MemBroker_GetPhysAddr(void*);

/**
 * @brief Translate a physical address to a virtual address
 *
 * @param[in] A pointer to the physical address of this allocated memory.
 * @return A pointer to the virtual address of this allocated memory.
 */
void* MemBroker_GetVirtAddr(void*);

/**
 * @brief Map(Build) a virtual address from a physical address. (it is used for a memory segment not allocated from "MemBroker_GetMemory".
 *
 * @param[in] A pointer to the physical address of this allocated memory.
 * @param[in] size  The size bytes of this allocated buffer.
 * @return A pointer to the virtual address of this allocated memory.
 */
void* MemBroker_MapPhysAddr(void* ptr, unsigned int size);

/**
 * @brief 'Flush' the data in CPU cache to DRAM.
 *
 * @param[in] A pointer to the virtual address of this allocated memory.
 * @param[in] size  The size bytes of this allocated buffer.
 */
int MemBroker_CacheCopyBack(void*, unsigned int size);

/**
 * @brief invalidate those cache tags with the specified memory section.
 *
 * @param[in] A pointer to the virtual address of this allocated memory.
 * @param[in] size  The size bytes of this allocated buffer.
 */
int MemBroker_CacheInvalidate(void*, unsigned int size);

/**
 * @brief 'Flush' the data in CPU cache to DRAM  and invalidate those cache tags with the specified memory section.
 *
 * @param[in] A pointer to the virtual address of this allocated memory.
 * @param[in] size  The size bytes of this allocated buffer.
 */
int MemBroker_CacheFlush(void*, unsigned int size);


#ifdef __cplusplus
}
#endif

#endif
