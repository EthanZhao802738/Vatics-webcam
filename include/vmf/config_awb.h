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
#ifndef CONFIG_AWB_H
#define CONFIG_AWB_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Auto white balance option flag enumeration
 */
typedef enum
{
	//! AWB option: reset
	AWB_OPTION_RESET,

	//! AWB option: converge speed
	AWB_OPTION_CONVERGE_SPEED,

	//! AWB option: control
	AWB_OPTION_CONTROL,

	//! AWB option: auto scene
	AWB_OPTION_AUTOSCENE,
} VMF_AWB_OPTION_FLAG_T;

/*
 * A data structure for AWB general options
 */
typedef struct
{
	//! A data for awb option flag 
    VMF_AWB_OPTION_FLAG_T eOptionFlags;

    //! A data for AWB handle index (Available in dual lens 360 mode)
	unsigned int dwIndex;

    //! A pointer data for user data
    void *pData; 
} VMF_AWB_OPTION_T;

/*
 * A data structure for AWB ctrl option, AWB_OPTION_CONTROL
 */
typedef struct
{
	//! A data for awb control mode
    unsigned int dwMode;
  
    //! A data for awb control's gain in r channel
    unsigned int dwGainR;

    //! A data for awb control's gain in b channel
    unsigned int dwGainB;

    //! A data for update wiin priority
    unsigned int bUpdateWinPriority;

    //! A data for wiin priority
    unsigned int* pdwWinPriority;
} VMF_AWB_CTRL_OPTION_T;

/*
 * A data structure for AWB converge speed option, AWB_OPTION_CONVERGE_SPEED
 */
typedef struct
{
	//! A data for awb spped
    unsigned int dwSpeed;
} VMF_AWB_SPEED_OPTION_T;

/*
 * A data structure for AWB autoscene option, AWB_OPTION_AUTOSCENE
 */
typedef struct
{
	//! A data for AWB mode : DWORD (0: Auto, 1: Full, 2: Customized, 3: Push hold) 
	unsigned int dwMode;

	//! A data for AWB lock : BOOL (1: AWB lock, 0: AWB unlock)
	unsigned int bLock;

	//! A data for customized WB's R gain when dwMode = 2 : DWORD (range 1~8191,  1024 = 1x)
	unsigned int dwCustomGainR;

	//! A data for customized WB's B gain when dwMode = 2 : DWORD (range 1~8191,  1024 = 1x)
	unsigned int dwCustomGainB;
} VMF_AWB_ASC_PARAM_T;

#ifdef __cplusplus
}
#endif

#endif

