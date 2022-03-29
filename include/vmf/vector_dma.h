
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

#ifndef VECTOR_DMA_H
#define VECTOR_DMA_H

#include <comm/video_buf.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An enumeration for DMA processing mode
 */
typedef enum {
	DMA_1D = 0,  //! DMA 1D mode
	DMA_2D = 1,  //! DMA 2D mode
	CONSTANT_FILLING = 2, //! DMA constant filling mode
	PRIVACY_MASK = 3,     //! DMA privacy mask mode
	MASK_WRITE = 4,       //! DMA mask write mode
	ALPHA_MASK = 5,       //! DMA alpha mask mode
	ALPHA_BLENDING = 6,   //! DMA alpha blending mode
	VERTICAL_ALPHA_BLENDING = 7, //! DMA vertical alpha blending mode
	INDEX_OSD = 8  //! DMA index osd mode
} VMF_DMA_MODE;

/**
 * An enumeration for DMA mask size
 */
typedef enum {
	DMA_MASK_1X1 = 0,  //! Mask size 1x1, 1 bit to 1 pixel
	DMA_MASK_2X2,      //! Mask size 2x2, 1 bit to 4 pixels
	DMA_MASK_RESERVED_MAX
} VMF_DMA_MASK_TYPE;

/**
 * A structure for DMA 1D mode config
 * DMA_1D
 */
typedef struct {
	unsigned int dwFormatFlag;	//! Buffer format flag, 0: origin, 1: compress
} VMF_DMA_1D_INIT_T;

/**
 * A structure for DMA 2D or constant filling mode config
 * DMA_2D and CONSTANT_FILLING
 */
typedef struct {
	unsigned int dwSrcFormatFlag;	//! Source Buffer format flag, 0: origin, 1: compress
	unsigned int dwDstFormatFlag;	//! Destination Buffer format flag, 0: origin, 1: compress	
	unsigned int dwProcessCbCr;	//! Process UV flag, 0: Y only, 1: process Cb and Cr.
	unsigned char abyColors[3];	//! Color setting For CONSTANT_FILLING
} VMF_DMA_2DCF_INIT_T;

/**
 * A structure for DMA mask info setup config
 * PRIVACY_MASK, MASK_WRITE, ALPHA_MASK, ALPHA_BLENDING, and VERTICAL_ALPHA_BLENDING
 */
typedef struct {

	VMF_DMA_MASK_TYPE type;  //! DMA mask size

	/**! bit-length of mask table (only for 'alpha mask' and 'frame alpha blending'
	 * value range: (0 ~ 3)
	 * it should be set to 0 for other modes.
	 */
	unsigned int dwValueIndexLength;
	/**!
	 * privacy mask:
	 *		idx 0: color value for luma or chroma.
	 *		idx 1: alpha value for luma or chroma.
	 *
	 * alpha mask:
	 *		idx [0-8]: color values (according to value_index_length).
	 *
	 * frame alpha blending:
	 *		idx [0-8]: alpha values (according to value_index_length).
	 */
	unsigned char abyValueTable[8];
	unsigned int dwMaskStride;       //! DMA mask stride
	unsigned char* pbyMaskPhysAddr;  //! DMA mask physical address
} VMF_DMA_MASK_INFO_T;

/**
 * A structure for DMA mask mode config
 */
typedef struct {
	unsigned int dwFormatFlag;		//! Buffer format flag, 0: origin, 1: compress
	unsigned int dwProcessCbCr;		//! Process UV flag, 0: Y only, 1: process Cb and Cr.
	VMF_DMA_MASK_INFO_T info[3]; 	//! The info array is used of YUV planar
} VMF_DMA_MASK_INIT_T;
/*!< For PRIVACY_MASK, MASK_WRITE, ALPHA_MASK, ALPHA_BLENDING, and VERTICAL_ALPHA_BLENDING */

/**
 * A structure for DMA index osd mode config
 */
typedef struct {
	unsigned int dwFormatFlag;	        //! Buffer format flag, 0: origin, 1: compress
	unsigned int dwProcessCbCr;		    //! Process UV flag, 0: Y only, 1: process Cb and Cr.
	unsigned int* pdwOsdPalettePhysAddr; //! OSD palette buffer
	unsigned char* pbyIndexPhysBuffer;	//! OSD index buffer
	unsigned int dwIndexStride;		    //! OSD index buffer stride
	unsigned int dwIndexWidth;          //! OSD index buffer width
	unsigned int dwIndexHeight;         //! OSD index buffer height
	unsigned int dwIndexCoorX;          //! OSD index buffer coordinate x
	unsigned int dwIndexCoorY;          //! OSD index buffer coordinate y
} VMF_DMA_OSD_INIT_T;

typedef struct VMF_DMA_DESCRIPTOR_T VMF_DMA_DESCRIPTOR_T;

/**
 * @brief Function to dma source and destination address.
 *
 * @note DMA_1D(0) : VMF_DMA_1D_INIT_T.
 *       DMA_2D(1) : VMF_DMA_2DCF_INIT_T
 *       CONSTANT_FILLING(2) : VMF_DMA_2DCF_INIT_T
 *       PRIVACY_MASK(3) : VMF_DMA_MASK_INIT_T
 *       MASK_WRITE(4) : VMF_DMA_MASK_INIT_T
 *       ALPHA_MASK(5) : VMF_DMA_MASK_INIT_T
 *       ALPHA_BLENDING(6) : VMF_DMA_MASK_INIT_T
 *       VERTICAL_ALPHA_BLENDING(7) : VMF_DMA_MASK_INIT_T
 *       INDEX_OSD(8) : VMF_DMA_OSD_INIT_T
 * @param[in] eMode The DMA processing mode.
 * @param[in] pInit The DMA config according to eMode.
 * @return Success: 0  Fail: negative integer.
 */
VMF_DMA_DESCRIPTOR_T* VMF_DMA_Descriptor_Create(VMF_DMA_MODE eMode, void* pInit);

/**
 * A structure for DMA address update
 */
typedef struct {
	unsigned int dwSrcStride;  //! Source buffer stride
	unsigned char* pbySrcYPhysAddr;   //! Source Y buffer physical address
	unsigned char* pbySrcCbPhysAddr;  //! Source Cb buffer physical address
	unsigned char* pbySrcCrPhysAddr;  //! Source Cr buffer physical address
	unsigned int dwDstStride;  //! Destination buffer stride
	unsigned char* pbyDstYPhysAddr;   //! Destination Y buffer physical address
	unsigned char* pbyDstCbPhysAddr;  //! Destination Cb buffer physical address
	unsigned char* pbyDstCrPhysAddr;  //! Destination Cr buffer physical address

	unsigned int dwTransSize;	 //! Transfer data size, only used in 1D DMA mode
	unsigned int dwCopyWidth;    //! DMA Copy width
	unsigned int dwCopyHeight;   //! DMA Copy height
} VMF_DMA_ADDR_T;

typedef struct VMF_DMA_HANDLE_T VMF_DMA_HANDLE_T;

/**
 * @brief Function to dma source and destination address
 *
 * @param[in] ptDesc Pointer of descriptor.
 * @param[in] ptConfig The config of VMF_DMA_ADDR_T.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_DMA_Descriptor_Update_Addr(VMF_DMA_DESCRIPTOR_T* ptDesc, const VMF_DMA_ADDR_T* ptConfig);

/**
 * @brief Function to set colors of descriptor. Only for constant filling currently.
 *
 * @param[in] ptDesc Pointer of descriptor.
 * @param[in] pbyColors Colors of descriptor.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_DMA_Descriptor_SetColor(VMF_DMA_DESCRIPTOR_T* ptDesc, const unsigned char* pbyColors);

/**
 * @brief Function to set mask info of descriptor.
 *
 * @param[in] ptDesc Pointer of descriptor.
 * @param[in] ptInfo Colors of descriptor.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_DMA_Descriptor_SetMaskInfo(VMF_DMA_DESCRIPTOR_T* ptDesc, const VMF_DMA_MASK_INFO_T* ptInfo);

/**
 * @brief Function to release the descriptor.
 *
 * @param[in] ptDesc Pointer of descriptor.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_DMA_Descriptor_Destroy(VMF_DMA_DESCRIPTOR_T* ptDesc);

/**
 * @brief Function to initialize Vector DMA.
 *
 * @param[in] dwDmacIdx The index of DMA hardware.
 * @param[in] dwBurstLength The burst length of Vector DMA.
 * @return The handle of Vector DMA.
 */
VMF_DMA_HANDLE_T* VMF_DMA_Init(unsigned int dwDmacIdx, unsigned int dwBurstLength);

/**
 * @brief Function to release Vector DMA.
 *
 * @param[in] ptHandle The handle of Vector DMA.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_DMA_Release(VMF_DMA_HANDLE_T* ptHandle);

/**
 * @brief Function to setup the descriptors into vector DMA.
 *
 * @param[in] ptHandle The handle of Vector DMA.
 * @param[in] pptDescArray The array of descriptor pointer.
 * @param[in] dwDescNum The number of descriptor array.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_DMA_Setup(VMF_DMA_HANDLE_T* ptHandle, VMF_DMA_DESCRIPTOR_T** pptDescArray, unsigned int dwDescNum);

/**
 * @brief process the content of source buffer to the destination buffer (blocking).
 *
 * @param[in] ptHandle The handle of Vector DMA.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_DMA_Process(VMF_DMA_HANDLE_T* ptHandle);

/**
 * @brief Process the content of source buffer to the destination buffer (non-blocking).
 *
 * @param[in] ptHandle The handle of Vector DMA.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_DMA_StartProcess(VMF_DMA_HANDLE_T* ptHandle);

/**
 * @brief Wait the StartProcess function to complete.
 *
 * @param[in] ptHandle The handle of Vector DMA.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_DMA_WaitComplete(VMF_DMA_HANDLE_T* ptHandle);

#ifdef __cplusplus
}
#endif

#endif
