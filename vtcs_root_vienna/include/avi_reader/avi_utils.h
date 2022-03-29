
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
#ifndef __AVI_UTILS_H__
#define __AVI_UTILS_H__

#if 0
#include <endian.h>
#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __BIG_ENDIAN
# define MAKEFOURCC(a,b,c,d) ((((uint32_t)a)<<24) | (((uint32_t)b)<<16) | \
		(((uint32_t)c)<< 8) | ((uint32_t)d))
#elif __BYTE_ORDER == __LITTLE_ENDIAN
# define MAKEFOURCC(a,b,c,d) (((uint32_t)a) | (((uint32_t)b)<<8) | \
		(((uint32_t)c)<<16) | (((uint32_t)d)<<24))
#elif __BYTE_ORDER == __PDP_ENDIAN
# define MAKEFOURCC(a,b,c,d) ((((uint32_t)a)<<16) | (((uint32_t)b)<<24) | \
		((uint32_t)c) | (((uint32_t)d)<<8))
#else
#error "Endian determination failed"
#endif
#endif
#endif

#ifndef MAKEFOURCC
#define MAKEFOURCC(a,b,c,d) (((uint32_t)a) | (((uint32_t)b)<<8) | \
		(((uint32_t)c)<<16) | (((uint32_t)d)<<24))
#endif

#ifndef PRINT_FOURCC
#define PRINT_FOURCC(S) *((char*)&(S)),*(((char*)&(S))+1),*(((char*)&(S))+2),*(((char*)&(S))+3)
#endif

#endif



