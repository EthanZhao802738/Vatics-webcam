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
#ifndef __LIVE_MEDIA_SINK_H__
#define __LIVE_MEDIA_SINK_H__

#include "LiveMediaSrc.h"
#include "RtpMapInfo.h"
#include "RtpDataInfo.h"

typedef int (*PFLMSinkBufCB) (void*, DataBufInfo*);
typedef int (*PFLMSinkComposeHeaderCB) (void*, DataBufInfo*, const TRtpMapInfo*, const TRtpDataInfo*);


/*!
 *******************************************************************************
 * LiveMediaSink base struct.
 * \note When you implement LiveMediaSink, the first element of your class must
 * be this base struct.
 *******************************************************************************
 */
typedef struct
{
	PFLMSinkBufCB pfSendAndGetBuf;
	PFLMSinkComposeHeaderCB pfComposeHeader;
} TLiveMediaSink;

#endif // __LIVE_MEDIA_SINK_H__
