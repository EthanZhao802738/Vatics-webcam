/*
 *******************************************************************************
 *  Copyright (c) 2010-2017 VATICS Inc. All rights reserved.
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
#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H
#ifdef __cplusplus
extern "C" {
#endif
#include <comm/video_buf.h>

/*
 * encoder type
 */
typedef enum {
	//! codec type: 264
	VMF_VDEC_CODEC_TYPE_H264,
	//! codec type: 265
	VMF_VDEC_CODEC_TYPE_H265,
	//! codec type: jpeg
	VMF_VDEC_CODEC_TYPE_JPEG
} VMF_VDEC_CODEC_TYPE;

typedef struct vmf_vdec_handle_t VMF_VDEC_HANDLE_T;

/*
 * A data structure for video engine init 
 */
typedef struct{
	//! A data for max width (The width of decoded frame. Should be multiple of 16 and less than 43680)
	unsigned int dwMaxWidth;

	//! A data for max height (The height of decoded frame, must be multiple of 16)
	unsigned int dwMaxHeight;
} VMF_VDEC_INITOPT_T;

/*
 * A enum for decode status 
 */
typedef enum {
	//! A flag to set when engine decoded one frame ok 
	VMF_DEC_OK,
	//! A flag to set when engine decoded need more data 
	VMF_DEC_CONTINUE,
	//! A flag to set when engine decoded end 
	VMF_DEC_BREAK,
	//! A flag to set when engine decoded failed 
	VMF_DEC_FAILED 
} VMF_DEC_STATUS_T;

/**
 * @brief Function to initialize H.264/H.265 decoder
 *
 * @param[in] opt The H.264/H.265 decoder's initializing option.
 * @param[in] codecType choose H.264/H.265 codec type.
 * @return The handle of H.264/H.265 decoder (tk).
 */
VMF_VDEC_HANDLE_T* VMF_VDEC_Init(const VMF_VDEC_INITOPT_T* ptOpt, int iCodecType);

/**
 * @brief Function to release video decoder
 *
 * @param[in] handle The handle of video decoder (tk).
 */
void VMF_VDEC_Release(VMF_VDEC_HANDLE_T* ptHandle);

/*
 * A data structure for decoder state
 */
typedef struct {
	//! A data for input buffer size (Size of Input bitstream buffer)
	unsigned int		dwInBufSize;

	//! A pointer data for input buffer (Pointer to Input bitstream buffer)
	unsigned char		*pbyInBuf; 

	//! A pointer data for output buffer (Pointer to output structure, buffer size must be equal to / larger than the size in VMTK_H4DEC_INITOPT_T and H265 buffer width stride will be 32 alignment and height stride will be 8 alignmnet)
	VMF_VIDEO_BUF_T		*ptOutBuf; 
	
	//! A data for error code (System error code presentation)
	unsigned int 		dwErrorCode; 

	//! A data for padded width (decoded output frame padded width (stride))
	unsigned int 		dwPadWidth; 

	//! A data for padded height (decoded output frame padded height)
	unsigned int 		dwPadHeight; 

	//! A data for decoded size (decoded byte size of current decoded frame)
	unsigned int 		dwDecSize; 

	//! A data for window x (decoded output frame display start position in horizontal direction)
	unsigned int 		dwWinX;

	//! A data for window y (decoded output frame display start position in vertical direction)    
	unsigned int 		dwWinY; 

	//! A data for window width (decoded output frame display width)    
	unsigned int 		dwWinWidth;

	//! A data for window height (decoded output frame display height)  
	unsigned int 		dwWinHeight;

	//! A data array for reserved2 
	unsigned int  		reserved2[4];

	//! A data for output frame number 
	unsigned int 		dwOutFrameNum;

	//! A data array for reserved1
	unsigned int  		reserved1[1];

	//! A data for end of bit stream (enable when end of bitstream)
	unsigned int 		dwEndOfBitStream;

	//! A data for decoded frame size (this is only for h265 decoder engine)
	unsigned int        dwSizeYuv;   
} VMF_VDEC_STATE_T;

/**
 * @brief Get the reference of VMF_VDEC_STATE_T structure.
 *
 * @param[in] handle The handle of Video decoder (tk).
 * @return Success: The pointer of VMF_VDEC_STATE_T structure  Fail: NULL.
 */
VMF_VDEC_STATE_T* VMF_VDEC_GetState(VMF_VDEC_HANDLE_T* ptHandle);

/**
 * @brief Function to decode Video data (blocking)
 *
 * @param[in] handle The handle of Video decoder (tk).
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VDEC_ProcessOneFrame(VMF_VDEC_HANDLE_T *ptHandle);

/**
 * @brief Function to decode video data (non-blocking)
 *
 * @param[in] handle The handle of video decoder (tk).
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VDEC_StartOneFrame(VMF_VDEC_HANDLE_T *ptHandle);

/**
 * @brief Wait for the StartOneFrame function completion.
 * @param[in] handle The handle of video decoder (tk).
 * @return Success: The pointer of VMF_VDEC_STATE_T structure. Fail: NULL.
 */
int VMF_VDEC_WaitOneFrameComplete(VMF_VDEC_HANDLE_T *ptHandle);

/**
 * @brief This function is used to inform the sample object of exiting current decoding process.
 *
 * @param[in] handle The handle of Video decoder (tk).
 * @return Success: 0  Fail: negative integer.
 */
int VMF_VDEC_Reset(VMF_VDEC_HANDLE_T* ptHandle);

#ifdef __cplusplus
}
#endif

#endif

