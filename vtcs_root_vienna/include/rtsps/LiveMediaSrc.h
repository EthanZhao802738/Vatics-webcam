
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
#ifndef __LIVE_MEDIA_SOURCE_H__
#define __LIVE_MEDIA_SOURCE_H__

typedef struct 
{
	unsigned char* pbyHdr;
	unsigned char* pbyPayload;
} DataBufInfo;

/*! 
 *********************************************************************
 * Enumeration of event type.
 *********************************************************************
*/
typedef enum lmsrc_event_type
{
	//! Need one Intra frame from this source
	letNeedIntra,
	//! Need one buffer conf from this source
	letNeedConf,
	//! Start needing bitstream from this source
	letBitstrmStart,
	//! Stop needing bitstream from this source
	letBitstrmStop
} ELMSrcEventType;

typedef int (*PFLMSrcBufCB) (void*, DataBufInfo*);
typedef int (*PFLMSrcEvntCB)    (void*, ELMSrcEventType);
typedef int (*PFLMSrcWakeUpToTerminate)  (void*);

typedef int (*PFLMSrcBufInitCB) (void** phShrdBufSrc, char* szSharedBuffer, char* szCmdFiFoPath);
typedef int (*PFLMSrcBufReleaseCB) (void** phShrdBufSrc);

/*!
 *******************************************************************************
 * LiveMediaSrc base struct.
 * \note When you implement LiveMediaSrc, the first element of your class must
 * be this base struct.
 *******************************************************************************
 */
typedef struct
{
    PFLMSrcBufCB pfReleaseAndGetBuf;
    PFLMSrcBufCB pfReleaseBuf;
    PFLMSrcEvntCB pfEvntHandler;
    PFLMSrcWakeUpToTerminate pfWakeUpToTerminate;
} TLiveMediaSrc;

#endif // __LIVE_MEDIA_SOURCE_H__
