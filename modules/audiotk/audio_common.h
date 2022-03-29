
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
#ifndef AUDIO_COMMON_H
#define AUDIO_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/// Max number of channels.
#define ATK_AUDIO_MAX_CHANNELS 32

// The max size of header from the audio encoder.
#define MAX_AUDIO_DATA_HEADER_SIZE 256

#define MAX_RING_BUF_SIZE (ATK_AUDIO_MAX_CHANNELS * 8 * 1024 + MAX_AUDIO_DATA_HEADER_SIZE)
#define MAX_ENCODE_DATA_SIZE (MAX_RING_BUF_SIZE - MAX_AUDIO_DATA_HEADER_SIZE)

#define MAKEFOURCC(ch0, ch1, ch2, ch3)  ((unsigned int)(unsigned char)(ch0) | ((unsigned int)(unsigned char)(ch1) << 8) | ((unsigned int)(unsigned char)(ch2) << 16) | ((unsigned int)(unsigned char)(ch3) << 24 ))

#define FOURCC_CONF (MAKEFOURCC('C','O','N','F'))
#define FOURCC_AAC4 (MAKEFOURCC('A','A','C','4'))
#define FOURCC_G711 (MAKEFOURCC('G','7','1','1'))
#define FOURCC_ULAW (MAKEFOURCC('U','L','A','W'))
#define FOURCC_ALAW (MAKEFOURCC('A','L','A','W'))
#define FOURCC_G726 (MAKEFOURCC('G','7','2','6'))
#define FOURCC_GAMR (MAKEFOURCC('G','A','M','R'))
#define FOURCC_GPCM (MAKEFOURCC('G','P','C','M'))
#define FOURCC_EXTEND (MAKEFOURCC('E','X','T','D'))

// For control the audio encoder
#define CMD_FIFO_PATH "/tmp/aenc/c0/command.fifo"

// The period size for audio libraries.
#define PERIOD_SIZE_IN_FRAMES 1024

// The period size for GAMR .
#define PERIOD_SIZE_IN_FRAMES_GAMR 160

// The period size for AAC .
#define PERIOD_SIZE_IN_FRAMES_AAC 1024  //! don't change it (hw only accept 1024)

typedef enum { kAAC4=0, kG711, kG726, kGAMR, kGPCM} ATK_AUDIO_ENCODER_TYPE;

#ifdef __cplusplus
}
#endif

#endif
