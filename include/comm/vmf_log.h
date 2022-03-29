/*
 *******************************************************************************
 *  Copyright (c) 2010-2016 VATICS Inc. All rights reserved.
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
#ifndef __COMM_VMF_LOG_H__
#define __COMM_VMF_LOG_H__
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
#define VMF_DML_ERROR		0x01
#define VMF_DML_WARNING	0x02
#define VMF_DML_INFO		0x04
#define VMF_DML_DEBUG		0x08
#define VMF_DML_TRACE		0x10
#define VMF_DML_PROFILE		0x20
#define VMF_DML_PROFILE2	0x40
#define DEFAULT_DEBUG_MESSAGE_LEVEL	0x0F	

extern int vmfDebugMessageLevel;

void VMF_SetDebugMessageLevel(int level);
int VMF_GetGodshandTimer( char * description );

#define LogE(tag, fmt, ...) \
 if (vmfDebugMessageLevel & VMF_DML_ERROR) do { printf("E/[%s] " fmt, tag, ##__VA_ARGS__); } while (0)
 	
#define LogW(tag, fmt, ...) \
 if (vmfDebugMessageLevel & VMF_DML_WARNING) do { printf("W/[%s] " fmt, tag, ##__VA_ARGS__); } while (0)
 	
#define LogI(tag, fmt, ...) \
 if (vmfDebugMessageLevel & VMF_DML_INFO) do { printf("I/[%s] " fmt, tag, ##__VA_ARGS__); } while (0)
 	
#define LogD(tag, fmt, ...) \
 if (vmfDebugMessageLevel & VMF_DML_DEBUG) do { printf("D/[%s] " fmt, tag, ##__VA_ARGS__); } while (0)
 	
#define LogT(tag, fmt, ...) \
 if (vmfDebugMessageLevel & VMF_DML_TRACE) do { printf("T/[%s] " fmt, tag, ##__VA_ARGS__); } while (0)

#define LogP(tag, fmt, ...) \
 if (vmfDebugMessageLevel & VMF_DML_PROFILE) do { printf("P1/[%s] " fmt, tag, ##__VA_ARGS__); } while (0)


#define LogP2(tag, fmt, ...) \
 if (vmfDebugMessageLevel & VMF_DML_PROFILE2) do { printf("P2/[%s] " fmt, tag, ##__VA_ARGS__); } while (0)


#ifdef __cplusplus
}
#endif

#endif //__COMM_VMF_LOG_H__
