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
#ifndef MEMSET_2D_H
#define MEMSET_2D_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A data Structure for MEMSET 2D Option
 */
typedef struct
{
	//! 0: mono, 1:color
	unsigned int dwColor;  
	//! 0: General, 1:Vatics format
	unsigned int dwFormat;  
	//! Input Width
	unsigned int dwWidth;    
	//! Input Height  
	unsigned int dwHeight;    
	//! Input stride 
	unsigned int dwStride; 
	//! Input physical address(Y/U/V)
	unsigned char  *apbyPhysAddr[3]; 
}VMF_MEMSET_2D_OPTION_T;

/**
 * @brief Function to Process 2D Memset
 *
 * @param[in] ptOption The option of 2D.
 * @return Success: 0  Fail: negative integer.
 */
int VMF_MEMSET_2D_Process(VMF_MEMSET_2D_OPTION_T* ptOption);

/**
 * @brief Function to initialize 2D Memset
 *
 * @return Success: 0  Fail: negative integer.
 */
void VMF_MEMSET_2D_Init(void);

/**
 * @brief Function to release 2D Memset
 *
 * @return Success: 0  Fail: negative integer.
 */
void VMF_MEMSET_2D_Release(void);

#ifdef __cplusplus
}
#endif

#endif