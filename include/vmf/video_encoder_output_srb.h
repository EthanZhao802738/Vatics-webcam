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

#ifndef VIDEO_ENCODER_OUTPUT_SRB_H
#define VIDEO_ENCODER_OUTPUT_SRB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vmf/video_encoder.h>
#include <SyncRingBuffer/sync_ring_buffer.h>
#include <comm/frame_info.h>
#include <comm/vmf_log.h>

#define VMF_VENC_OUTPUT_SRB_HEADER		256           //! SRB header size

#ifdef __cplusplus
extern "C" {
#endif

#define VENC_SRB_TAG "VENC_SRB"

/**
 * A structure for VMF video encoder SRB output initialization.
 */
typedef struct
{
	const char    *pszSrbName;		//! string of SRB name.
	unsigned int  dwSrbNum;			//! SRB buffer number.
	unsigned int  dwSrbSize;		//! SRB buffer size, including bitstream and SRB header.
	unsigned int  bBlockMode;		//! SRB block mode.
} VMF_VENC_OUT_SRB_INITOPT_T;

/**
 * A structure for VMF video encoder SRB output.
 */
typedef struct
{
	SRB_HANDLE_T *pSRBHandle;		//! The pointer to SRB handle.
	SRB_BUFFER_T tSrbBuf;			//! SRB buffer structure.
	unsigned int dwBsBufSize;		//! Bitstream buffer size, maximun size of per encoded frame.
	unsigned int dwSeqNum;			//! Encode frame sequence number.
	unsigned int bBlockMode;		//! Encode register preprocess callback function for block mode.

	// For bitrate profiling
	unsigned int dwFrameCount;		//! the encode frame number on GoP interval
	unsigned int dwEncodeBytes;		//! the totoal encode byte on GoP interval
	unsigned int dwMaxEncodeBytes;	//! the max encode bytes on current bitstream
} VMF_VENC_OUT_SRB_T;

/**
 * @brief Function to get fourcc
 *
 * @param[in] fourcc_str The pointer to store the fourcc.
 * @param[in] fourcc The value of fourcc.
 */
static inline void get_fourcc_str(char* fourcc_str, int fourcc)
{
	if (NULL == fourcc_str)
	 return;
	fourcc_str[0] = (char)(fourcc & 0xFF);
	fourcc_str[1] = (char)((fourcc >> 8) & 0xFF);
	fourcc_str[2] = (char)((fourcc >> 16) & 0xFF);
	fourcc_str[3] = (char)((fourcc >> 24) & 0xFF);
	fourcc_str[4] = 0;
}

/**
 * @brief Function to release VMF video encoder SRB buffer
 * @note Have to stop VMF_VENC before VMF_VENC_OUT_SRB_Init()/VMF_VENC_OUT_SRB_Release().
 *  Have to VMF_VENC_Config() after changing the VMF_VENC_OUT_SRB_T.
 * @param[in] pptVencOutSrb The pointer of VMF_VENC_OUT_SRB_T*.
 * @return Success: 0  Fail: negative integer.
 */
static inline int VMF_VENC_OUT_SRB_Release(VMF_VENC_OUT_SRB_T** pptVencOutSrb)
{
	VMF_VENC_OUT_SRB_T* venc_out = NULL;

	if (!pptVencOutSrb || !(*pptVencOutSrb)) {
		return -1;
	}

	venc_out =  (VMF_VENC_OUT_SRB_T*) *pptVencOutSrb;
	SRB_Release(venc_out->pSRBHandle);
	free(venc_out);
	*pptVencOutSrb = NULL;
	return 0;
}

/**
 * @brief Function to init VMF video encoder SRB buffer
 * @note Have to stop VMF_VENC before VMF_VENC_OUT_SRB_Init()/VMF_VENC_OUT_SRB_Release().
 *  Have to VMF_VENC_Config() after changing the VMF_VENC_OUT_SRB_T.
 * @param[in] pptVencOutSrb The pointer of VMF_VENC_OUT_SRB_T*.
 * @param[in] ptInitOpt A pointer to the VMF_VENC_OUT_SRB_INITOPT_T structure.
 * @return Success: 0  Fail: negative integer.
 */
static inline int VMF_VENC_OUT_SRB_Init(VMF_VENC_OUT_SRB_T** pptVencOutSrb, VMF_VENC_OUT_SRB_INITOPT_T *ptInitOpt)
{
	VMF_VENC_OUT_SRB_T* venc_out = (VMF_VENC_OUT_SRB_T*) calloc(1, sizeof(VMF_VENC_OUT_SRB_T));

	if (!venc_out) {
		return -1;
	}
	//! Init SRB Writer
	venc_out->pSRBHandle = SRB_InitWriter(ptInitOpt->pszSrbName, ptInitOpt->dwSrbSize, ptInitOpt->dwSrbNum);
	if (!venc_out->pSRBHandle) {
		goto VMF_VENC_OUT_SRB_INIT_FAIL;
	}
	//! Get first writer buffer
	SRB_SendGetWriterBuff(venc_out->pSRBHandle, &venc_out->tSrbBuf);
	if (!venc_out->tSrbBuf.buffer) {
		goto VMF_VENC_OUT_SRB_INIT_FAIL;
	}

	venc_out->dwBsBufSize = ptInitOpt->dwSrbSize - VMF_VENC_OUTPUT_SRB_HEADER;
	venc_out->dwSeqNum = 0;
	venc_out->bBlockMode = ptInitOpt->bBlockMode;
	*pptVencOutSrb = venc_out;
	return 0;

VMF_VENC_OUT_SRB_INIT_FAIL:
	VMF_VENC_OUT_SRB_Release(&venc_out);
	return -1;
}

/**
 * @brief Function to set header of H.264 stream.
 * @note  Set SRB streaming header by codecs
 *  The functions are set to fnOnStreamHeaderCallback in VMF_VENC_CONFIG_T, vmf/video_encoder.h
 *  It also needs a pointer to VMF_VENC_OUT_SRB_T set as pOnStreamHeaderCallbackUserData in VMF_VENC_CONFIG_T.
 * @param[in] ptEncodeInfo The pointer to VMF_VENC_ENCODE_INFO_T, passed by VMF_VENC.
 * @param[in] pStreamHeader The pointer to void, passed by VMF_VENC.
 * @param[in] pVencOutSrb The pointer to VMF_VENC_OUT_SRB_T and casted to void*, set in pOnStreamHeaderCallbackUserData of VMF_VENC_CONFIG_T and passed by VMF_VENC.
 * @return Success: 0  Fail: negative integer.
 */
static inline int VMF_VENC_OUT_SRB_H264_Header(VMF_VENC_ENCODE_INFO_T* ptEncodeInfo, void* pStreamHeader, void* pVencOutSrb)
{
	VMF_VENC_OUT_SRB_T       *venc_out           = (VMF_VENC_OUT_SRB_T*) pVencOutSrb;
	VMF_VENC_H264_STREAM_HDR *h264_hdr           = (VMF_VENC_H264_STREAM_HDR *) pStreamHeader;

	if (!venc_out) {
		return -1;
	}
	if (VMF_VENC_CODEC_TYPE_H264 != ptEncodeInfo->eCodecType) {
		return -1;
	}
	if (!h264_hdr) {
		return -1;
	}

	memcpy(venc_out->tSrbBuf.buffer, h264_hdr, 8 * sizeof(unsigned int));
	memcpy(venc_out->tSrbBuf.buffer + 32, h264_hdr->abySpsData, h264_hdr->dwSpsSize);
	memcpy(venc_out->tSrbBuf.buffer + 32 + h264_hdr->dwSpsSize, h264_hdr->abyPpsData, h264_hdr->dwPpsSize);
	SRB_SendGetWriterBuff(venc_out->pSRBHandle, &venc_out->tSrbBuf);
	return 0;
}

/**
 * @brief Function to set header of H.265 stream.
 *
 * @param[in] ptEncodeInfo The pointer to VMF_VENC_ENCODE_INFO_T, passed by VMF_VENC.
 * @param[in] pStreamHeader The pointer to void, passed by VMF_VENC.
 * @param[in] pVencOutSrb The pointer to VMF_VENC_OUT_SRB_T and casted to void*, set in pOnStreamHeaderCallbackUserData of VMF_VENC_CONFIG_T and passed by VMF_VENC.
 * @return Success: 0  Fail: negative integer.
 */
static inline int VMF_VENC_OUT_SRB_H265_Header(VMF_VENC_ENCODE_INFO_T *ptEncodeInfo, void *pStreamHeader, void *pVencOutSrb)
{
	VMF_VENC_OUT_SRB_T       *venc_out           = (VMF_VENC_OUT_SRB_T*) pVencOutSrb;
	VMF_VENC_H265_STREAM_HDR *hdr                = (VMF_VENC_H265_STREAM_HDR *) pStreamHeader;

	if (!venc_out) {
		return -1;
	}
	if (VMF_VENC_CODEC_TYPE_H265 != ptEncodeInfo->eCodecType) {
		return -1;
	}
	if (!hdr) {
		return -1;
	}
	memcpy(venc_out->tSrbBuf.buffer, hdr, 9 * sizeof(unsigned int));
	memcpy(venc_out->tSrbBuf.buffer + 36, hdr->abyVpsData, hdr->dwVpsSize);
	memcpy(venc_out->tSrbBuf.buffer + 36 + hdr->dwVpsSize, hdr->abySpsData, hdr->dwSpsSize);
	memcpy(venc_out->tSrbBuf.buffer + 36 + hdr->dwVpsSize + hdr->dwSpsSize, hdr->abyPpsData, hdr->dwPpsSize);
	SRB_SendGetWriterBuff(venc_out->pSRBHandle, &venc_out->tSrbBuf);
	return 0;
}

/**
 * @brief Function to output header of MJPG stream.
 *
 * @param[in] ptEncodeInfo The pointer to VMF_VENC_ENCODE_INFO_T, passed by VMF_VENC.
 * @param[in] pStreamHeader The pointer to void, passed by VMF_VENC.
 * @param[in] pVencOutSrb The pointer to VMF_VENC_OUT_SRB_T and casted to void*, set in pOnStreamHeaderCallbackUserData of VMF_VENC_CONFIG_T and passed by VMF_VENC.
 * @return Success: 0  Fail: negative integer.
 */
static inline int VMF_VENC_OUT_SRB_MJPG_Header(VMF_VENC_ENCODE_INFO_T* ptEncodeInfo, void* pStreamHeader, void* pVencOutSrb)
{
	VMF_VENC_OUT_SRB_T       *venc_out           = (VMF_VENC_OUT_SRB_T*) pVencOutSrb;
	VMF_VENC_MJPG_STREAM_HDR *mjpg_hdr           = (VMF_VENC_MJPG_STREAM_HDR*) pStreamHeader;

	if (!venc_out) {
		return -1;
	}
	if (VMF_VENC_CODEC_TYPE_MJPG != ptEncodeInfo->eCodecType) {
		return -1;
	}
	if (!mjpg_hdr) {
		return -1;
	}
	memcpy(venc_out->tSrbBuf.buffer, mjpg_hdr, sizeof(VMF_VENC_MJPG_STREAM_HDR));
	SRB_SendGetWriterBuff(venc_out->pSRBHandle, &venc_out->tSrbBuf);
	return 0;
}

/**
 * @brief Function to check whether the writer will over reader.
 *
 * @note Callback before each encoding process, set SRB buffer pointer to encoder output
 *  The functions are set to fnOnPreProcessCallback in VMF_VENC_CONFIG_T, vmf/video_encoder.h
 *  It also needs a pointer to VMF_VENC_OUT_SRB_T set as pOnPreProcessCallbackUserData in VMF_VENC_CONFIG_T.
 *
 * @param[in] pVencOutSrb The pointer to VMF_VENC_OUT_SRB_T and casted to void*,
 *            set in pOnStreamHeaderCallbackUserData of VMF_VENC_CONFIG_T and passed by VMF_VENC.
 * @return Success: 0  Fail: negative integer.
 */
static inline int VMF_VENC_OUT_SRB_Checker(void* pVencOutSrb)
{
	VMF_VENC_OUT_SRB_T* venc_out = (VMF_VENC_OUT_SRB_T*) pVencOutSrb;

	if (!venc_out) {
		return -1;
	}
	if (!venc_out->tSrbBuf.buffer) {
		return -1;
	}
			
	if (SRB_WriterCheckReader(venc_out->pSRBHandle, &venc_out->tSrbBuf)) {
		return -1;
	}
	return 0;
}


/**
 * @brief Function to set encoding stream output.
 *
 * @note Callback before each encoding process, set SRB buffer pointer to encoder output
 *  The functions are set to fnOnSetOutputCallback in VMF_VENC_CONFIG_T, vmf/video_encoder.h
 *  It also needs a pointer to VMF_VENC_OUT_SRB_T set as pOnSetOutputCallbackUserData in VMF_VENC_CONFIG_T.
 *
 * @param[in] ppbyOutBuff The pointer to unsigned char* which point to output buffer virtual address.
 * @param[in] ppbyOutPhysBuff  The pointer to unsigned char* which point to output buffer physical address.
 * @param[in] pdwOutBuffSize The Size of output buffer
 * @param[in] pVencOutSrb The pointer to VMF_VENC_OUT_SRB_T and casted to void*,
 *            set in pOnStreamHeaderCallbackUserData of VMF_VENC_CONFIG_T and passed by VMF_VENC.
 * @return Success: 0  Fail: negative integer.
 */
static inline int VMF_VENC_OUT_SRB_SetOutput(unsigned char** ppbyOutBuff, unsigned char** ppbyOutPhysBuff,
	unsigned int* pdwOutBuffSize, void* pVencOutSrb)
{
	VMF_VENC_OUT_SRB_T* venc_out = (VMF_VENC_OUT_SRB_T*) pVencOutSrb;

	if (!venc_out) {
		return -1;
	}
	if (!venc_out->tSrbBuf.buffer) {
		return -1;
	}
	if (ppbyOutBuff) {
		*ppbyOutBuff = venc_out->tSrbBuf.buffer + VMF_VENC_OUTPUT_SRB_HEADER;
		*ppbyOutPhysBuff = venc_out->tSrbBuf.buffer_phys_addr + VMF_VENC_OUTPUT_SRB_HEADER;
	}
	if (pdwOutBuffSize) {
		*pdwOutBuffSize = venc_out->dwBsBufSize;
	}
	return 0;
}

/**
 * @brief Function to send H.264 encoded data to SRB ring buffer.
 *
 * @note Callback after each encoding precess, fill up encoded data information to output (SRB) buffer
 *  The functions are set to fnOnDataCallback in VMF_VENC_CONFIG_T, vmf/video_encoder.h
 *  It also needs a pointer to VMF_VENC_OUT_SRB_T set as pOnDataCallbackUserData in VMF_VENC_CONFIG_T.
 * @param[in] ptEncodeInfo The pointer to VMF_VENC_ENCODE_INFO_T, passed by VMF_VENC.
 * @param[in] pbyData The pointer to of encoded data.
 * @param[in] dwDataBytes The Size of encoded data.
 * @param[in] pVencOutSrb The pointer to VMF_VENC_OUT_SRB_T and casted to void*,
 *            set in pOnStreamHeaderCallbackUserData of VMF_VENC_CONFIG_T and passed by VMF_VENC.
 * @return Success: 0  Fail: negative integer.
 */
static inline int VMF_VENC_OUT_SRB_H264_Data(VMF_VENC_ENCODE_INFO_T* ptEncodeInfo,
	unsigned char* pbyData __attribute__((unused)), unsigned int dwDataBytes, void* pVencOutSrb)
{
	VMF_VENC_OUT_SRB_T       *venc_out = (VMF_VENC_OUT_SRB_T*) pVencOutSrb;
	VMF_VENC_STREAM_DATA_HDR *hdr      = (VMF_VENC_STREAM_DATA_HDR*) venc_out->tSrbBuf.buffer;

#ifdef DEBUG
	if (!venc_out || !venc_out->pSRBHandle || !venc_out->tSrbBuf.buffer) {
		return -1;
	}
	if (!ptEncodeInfo) {
		return -1;
	}
	if (ptEncodeInfo->dwSeqNum - venc_out->dwSeqNum > 1) {
		LogP(VENC_SRB_TAG, "[%s, %d] Frame skipped, ptEncodeInfo->dwSeqNum(%u), venc_out->dwSeqNum(%u)\n", __func__, __LINE__, ptEncodeInfo->dwSeqNum, venc_out->dwSeqNum);
	}
	venc_out->dwSeqNum = ptEncodeInfo->dwSeqNum;
	if (ptEncodeInfo->dwIsKeyFrame) {
		LogP(VENC_SRB_TAG, "[%s, %d] Key frame produced, ptEncodeInfo->dwIsKeyFrame(%u)\n", __func__, __LINE__, ptEncodeInfo->dwIsKeyFrame);
	}
#endif
	if (dwDataBytes) {
		hdr->dwFourCC    = VMF_FOURCC_H264;
		hdr->dwFrameSec  = ptEncodeInfo->dwSec;
		hdr->dwFrameUSec = ptEncodeInfo->dwUSec;
		hdr->dwDataBytes = dwDataBytes;
		hdr->dwSeqNum    = ptEncodeInfo->dwSeqNum;
		hdr->bIsKeyFrame = ptEncodeInfo->dwIsKeyFrame;
		hdr->dwBufOffset = 0;

		if (vmfDebugMessageLevel & VMF_DML_PROFILE) {
			if (venc_out->dwMaxEncodeBytes < dwDataBytes) venc_out->dwMaxEncodeBytes = dwDataBytes;
				if (ptEncodeInfo->dwIsKeyFrame) {
					if (venc_out->dwFrameCount > 0) {
						LogP(VENC_SRB_TAG, "[%s, %d] Key frame produced, Frames(%u), Bitrate (%u), Max Encoded Frame Bytes(%u)\n", 
							__func__, __LINE__, venc_out->dwFrameCount, venc_out->dwEncodeBytes * 8 / venc_out->dwFrameCount, venc_out->dwMaxEncodeBytes);
					}
					venc_out->dwFrameCount = 0;
					venc_out->dwEncodeBytes = dwDataBytes;
				} else {
					venc_out->dwEncodeBytes += dwDataBytes;
				}
			venc_out->dwFrameCount++;
		}
		
		SRB_SendGetWriterBuff(venc_out->pSRBHandle, &venc_out->tSrbBuf);
	}
	return 0;
}

/**
 * @brief Function to send H.265 encoded data to SRB ring buffer.
 *
 * @note Callback after each encoding precess, fill up encoded data information to output (SRB) buffer
 *  The functions are set to fnOnDataCallback in VMF_VENC_CONFIG_T, vmf/video_encoder.h
 *  It also needs a pointer to VMF_VENC_OUT_SRB_T set as pOnDataCallbackUserData in VMF_VENC_CONFIG_T.
 * @param[in] ptEncodeInfo The pointer to VMF_VENC_ENCODE_INFO_T, passed by VMF_VENC.
 * @param[in] pbyData The pointer to of encoded data.
 * @param[in] dwDataBytes The Size of encoded data.
 * @param[in] pVencOutSrb The pointer to VMF_VENC_OUT_SRB_T and casted to void*,
 *            set in pOnStreamHeaderCallbackUserData of VMF_VENC_CONFIG_T and passed by VMF_VENC.
 * @return Success: 0  Fail: negative integer.
 */
static inline int VMF_VENC_OUT_SRB_H265_Data(VMF_VENC_ENCODE_INFO_T* ptEncodeInfo,
	unsigned char* pbyData __attribute__((unused)), unsigned int dwDataBytes, void* pVencOutSrb)
{
	VMF_VENC_OUT_SRB_T       *venc_out = (VMF_VENC_OUT_SRB_T*) pVencOutSrb;
	VMF_VENC_STREAM_DATA_HDR *hdr      = (VMF_VENC_STREAM_DATA_HDR*) venc_out->tSrbBuf.buffer;

#ifdef DEBUG
	if (!venc_out || !venc_out->pSRBHandle || !venc_out->tSrbBuf.buffer) {
		return -1;
	}
	if (!ptEncodeInfo) {
		return -1;
	}
	if (ptEncodeInfo->dwSeqNum - venc_out->dwSeqNum > 1) {
		printf("[%s, %d] Frame skipped, ptEncodeInfo->dwSeqNum(%u), venc_out->dwSeqNum(%u)\n", __func__, __LINE__, ptEncodeInfo->dwSeqNum, venc_out->dwSeqNum);
	}
	venc_out->dwSeqNum = ptEncodeInfo->dwSeqNum;
	if (ptEncodeInfo->dwIsKeyFrame) {
		printf("[%s, %d] Key frame produced, ptEncodeInfo->dwIsKeyFrame(%u)\n", __func__, __LINE__, ptEncodeInfo->dwIsKeyFrame);
	}
#endif
	if (dwDataBytes) {
		hdr->dwFourCC    = VMF_FOURCC_H265;
		hdr->dwFrameSec  = ptEncodeInfo->dwSec;
		hdr->dwFrameUSec = ptEncodeInfo->dwUSec;
		hdr->dwDataBytes = dwDataBytes;
		hdr->dwSeqNum    = ptEncodeInfo->dwSeqNum;
		hdr->bIsKeyFrame = ptEncodeInfo->dwIsKeyFrame;
		hdr->dwBufOffset = ptEncodeInfo->dwBufOffset;
		if (vmfDebugMessageLevel & VMF_DML_PROFILE) {
			if (venc_out->dwMaxEncodeBytes < dwDataBytes) venc_out->dwMaxEncodeBytes = dwDataBytes;
				if (ptEncodeInfo->dwIsKeyFrame) {
					if (venc_out->dwFrameCount > 0) {
						LogP(VENC_SRB_TAG, "[%s, %d] Key frame produced, Frames(%u), Bitrate (%u), Max Encoded Frame Bytes(%u)\n", 
							__func__, __LINE__, venc_out->dwFrameCount, venc_out->dwEncodeBytes * 8 / venc_out->dwFrameCount, venc_out->dwMaxEncodeBytes);
					}
					venc_out->dwFrameCount = 0;
					venc_out->dwEncodeBytes = dwDataBytes;
				} else {
					venc_out->dwEncodeBytes += dwDataBytes;
				}
			venc_out->dwFrameCount++;
		}
		SRB_SendGetWriterBuff(venc_out->pSRBHandle, &venc_out->tSrbBuf);
	}
	return 0;
}

/**
 * @brief Function to send MJPG encoded data to SRB ring buffer.
 *
 * @note Callback after each encoding precess, fill up encoded data information to output (SRB) buffer
 *  The functions are set to fnOnDataCallback in VMF_VENC_CONFIG_T, vmf/video_encoder.h
 *  It also needs a pointer to VMF_VENC_OUT_SRB_T set as pOnDataCallbackUserData in VMF_VENC_CONFIG_T.
 * @param[in] ptEncodeInfo The pointer to VMF_VENC_ENCODE_INFO_T, passed by VMF_VENC.
 * @param[in] pbyData The pointer to of encoded data.
 * @param[in] dwDataBytes The Size of encoded data.
 * @param[in] pVencOutSrb The pointer to VMF_VENC_OUT_SRB_T and casted to void*,
 *            set in pOnStreamHeaderCallbackUserData of VMF_VENC_CONFIG_T and passed by VMF_VENC.
 * @return Success: 0  Fail: negative integer.
 */
static inline int VMF_VENC_OUT_SRB_MJPG_Data(VMF_VENC_ENCODE_INFO_T* ptEncodeInfo,
	unsigned char* pbyData __attribute__((unused)), unsigned int dwDataBytes, void* pVencOutSrb)
{
	VMF_VENC_OUT_SRB_T       *venc_out = (VMF_VENC_OUT_SRB_T*) pVencOutSrb;
	VMF_VENC_STREAM_DATA_HDR *hdr      = (VMF_VENC_STREAM_DATA_HDR*) venc_out->tSrbBuf.buffer;

#ifdef DEBUG
	if (!venc_out || !venc_out->pSRBHandle || !venc_out->tSrbBuf.buffer) {
		return -1;
	}
	if (!ptEncodeInfo) {
		return -1;
	}
	if (ptEncodeInfo->dwSeqNum - venc_out->dwSeqNum > 1) {
		printf("[%s, %d] Frame skipped, ptEncodeInfo->dwSeqNum(%u), venc_out->dwSeqNum(%u)\n", __func__, __LINE__, ptEncodeInfo->dwSeqNum, venc_out->dwSeqNum);
	}
	venc_out->dwSeqNum = ptEncodeInfo->dwSeqNum;
#endif

	if (dwDataBytes) {
		hdr->dwFourCC    = VMF_FOURCC_JPEG;
		hdr->dwFrameSec  = ptEncodeInfo->dwSec;
		hdr->dwFrameUSec = ptEncodeInfo->dwUSec;
		hdr->dwDataBytes = dwDataBytes;
		hdr->dwSeqNum    = ptEncodeInfo->dwSeqNum;
		hdr->bIsKeyFrame = ptEncodeInfo->dwIsKeyFrame;
		hdr->dwBufOffset = 0;
		if (vmfDebugMessageLevel & VMF_DML_PROFILE) {
			if (venc_out->dwMaxEncodeBytes < dwDataBytes) venc_out->dwMaxEncodeBytes = dwDataBytes;
				if (ptEncodeInfo->dwIsKeyFrame) {
					if (venc_out->dwFrameCount > 0) {
						LogP(VENC_SRB_TAG, "[%s, %d] Key frame produced, Frames(%u), Bitrate (%u), Max Encoded Frame Bytes(%u)\n", 
							__func__, __LINE__, venc_out->dwFrameCount, venc_out->dwEncodeBytes * 8 / venc_out->dwFrameCount, venc_out->dwMaxEncodeBytes);
					}
					venc_out->dwFrameCount = 0;
					venc_out->dwEncodeBytes = dwDataBytes;
				} else {
					venc_out->dwEncodeBytes += dwDataBytes;
				}
			venc_out->dwFrameCount++;
		}
		SRB_SendGetWriterBuff(venc_out->pSRBHandle, &venc_out->tSrbBuf);
	}
	return 0;
}

/**
 * @brief Function to config VMF_VENC_CONFIG_T member by codec input.
 *
 * @param[out] ptVencConfig The pointer to VMF_VENC_ENCODE_INFO_T, passed by VMF_VENC.
 * @param[in] eCodecType Encoding codec type.
 * @param[in] pCodecConfig Encoding codec config related to code type eCodecType.
 * @param[in] pSrb The pointer to VMF_VENC_OUT_SRB_T and casted to void*s.
 * @return Success: 0  Fail: negative integer.
 */
static inline int VMF_VENC_OUT_SRB_Setup_Config(VMF_VENC_CONFIG_T* ptVencConfig,
	VMF_VENC_CODEC_TYPE eCodecType, void* pCodecConfig, void* pSrb)
{
	VMF_VENC_OUT_SRB_T       *venc_out = (VMF_VENC_OUT_SRB_T*) pSrb;
	
	if (!ptVencConfig || !pCodecConfig) return -1;

	ptVencConfig->eCodecType = eCodecType;
	ptVencConfig->pCodecConfig = pCodecConfig;
	ptVencConfig->fnOnSetOutputCallback = VMF_VENC_OUT_SRB_SetOutput;
	
	if (venc_out->bBlockMode) {
		ptVencConfig->fnOnPreProcessCallback = VMF_VENC_OUT_SRB_Checker;
	}

	switch (eCodecType) {
		case VMF_VENC_CODEC_TYPE_H264: {
			ptVencConfig->fnOnDataCallback = VMF_VENC_OUT_SRB_H264_Data;
			ptVencConfig->fnOnStreamHeaderCallback = VMF_VENC_OUT_SRB_H264_Header;
		} break;

		case VMF_VENC_CODEC_TYPE_H265: {
			ptVencConfig->fnOnDataCallback = VMF_VENC_OUT_SRB_H265_Data;
			ptVencConfig->fnOnStreamHeaderCallback = VMF_VENC_OUT_SRB_H265_Header;
		} break;

		case VMF_VENC_CODEC_TYPE_MJPG: {
			ptVencConfig->fnOnDataCallback = VMF_VENC_OUT_SRB_MJPG_Data;
			ptVencConfig->fnOnStreamHeaderCallback = VMF_VENC_OUT_SRB_MJPG_Header;
		} break;
		default: {
			return -1;
		} break;
	}
	ptVencConfig->pOnPreProcessCallbackUserData = (void *) pSrb;
	ptVencConfig->pOnSetOutputCallbackUserData = (void *) pSrb;
	ptVencConfig->pOnDataCallbackUserData =  (void *) pSrb;
	ptVencConfig->pOnStreamHeaderCallbackUserData = (void *) pSrb;
	return 0;
}

#ifdef __cplusplus
}
#endif


#endif //! guard
