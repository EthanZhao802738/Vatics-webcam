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
#ifndef CONFIG_OSD_H
#define CONFIG_OSD_H

#ifdef __cplusplus
extern "C" {
#endif
#define MAX_WEEK_DAY 7

/*
 * overlay mode
 */
typedef enum
{
	//! overlay mode: mono
	VMF_OVERLAY_MONO = 0,

	//! overlay mode: color
	VMF_OVERLAY_COLOR,
} VMF_OVERLAY_MODE;

/*
 * overlay align hint
 */
typedef enum 
{ 
	//! text align: none
	VMF_TEXT_ALIGN_NONE = 0,

	//! text align: left
	VMF_TEXT_ALIGN_LEFT = (1<<0),

	//! text align: right
	VMF_TEXT_ALIGN_RIGHT = (1<<1),

	//! text align: top
	VMF_TEXT_ALIGN_TOP = (1<<2),

	//! text align: bottom
	VMF_TEXT_ALIGN_BOTTOM = (1<<3),

	//! inner text align: left
	VMF_INNER_TEXT_ALIGN_LEFT = (1<<4)
} VMF_OVERLAY_ALIGN_HINT;

/*
 * A data structure for overlay text config
 */
typedef struct
{
	//! A data for the position x
	unsigned int dwPosX; 

	//! A data for the position y
	unsigned int dwPosY;

	//! A data for align. (The aligment hint for text. If it is zero, the position x and y will be used. Otherwise, the alignment hint will be used in x and y position.)
	unsigned int dwAlign;

	//! A pointer data for text (The text which will be drawn)
	const char* pszText;

	//! A data for auto brightness (Auto adjust osd buffer brightness by background buffer brightness)
	unsigned int bAutoBrightness;
} vmf_overlay_text_config_t;
typedef vmf_overlay_text_config_t VMF_OVERLAY_TEXT_CONFIG_T;

/*
 * A data structure for overlay buffer config
 */
typedef struct
{
	//! A data for the position x of overlay buffer
	unsigned int dwPosX;

	//! A data for the position y of overlay buffer
	unsigned int dwPosY;

	//! A pointer data for the overlay buffer which will be drawn
	unsigned char* pbyBuf;

	//! A data for the width of overlay buffer
	unsigned int dwWidth;

	//! A data for the height of overlay buffer
	unsigned int dwHeight;

	//! A data for auto brightness (Auto adjust osd buffer brightness by background buffer brightness)
	unsigned int bAutoBrightness;
} vmf_overlay_buf_config_t;
typedef vmf_overlay_buf_config_t VMF_OVERLAY_BUF_CONFIG_T;

/*
 * A data structure for overlay extra info
 */
typedef struct
{	
	//! A data for date time auto brightness (Auto adjust osd buffer brightness by background buffer brightness)
	unsigned int bDatetimeAutoBrightness;

	//! A data for date time (0: See datetime_format_string as ascii string, Others: See datetime_format_string as UTF8 string.) 
	unsigned int bDatetimeExtraUtf8;

	//! A data for offset seconds (offset seconds for setting offset of datetime)
	long offset_seconds;

	//! A pointer data to weekday (Weekday utf8 text which used to inform vmf the weekday utf8 code. NULL: disable)
	const char* apszWeekdayUTF8[MAX_WEEK_DAY]; 
} vmf_overlay_extra_info_t;
typedef vmf_overlay_extra_info_t VMF_OVERLAY_EXTRA_INFO_T;

/*
 * A data structure for overlay config
 */
typedef struct
{
	//! A data for overlay mode
	VMF_OVERLAY_MODE eMode;

	//! A pointer data for date time format (NULL: disable date time)
	const char* pszDatetimeFormat; 

	//! A data for horizontal position of date time
	unsigned int dwDatetimePosX;   

	//! A data for vertical position of date time
	unsigned int dwDatetimePosY;  

	//! A data for date time align (Aligment hint for overlay. 0: Use position x and y. Other: Use alignment hint)
	unsigned int dwDateTimeAlign;
	
	//! A data for text count
	unsigned int dwTextCount;

	//! A pointer data for overlay text config (The array of text. NULL item: disable)
	const VMF_OVERLAY_TEXT_CONFIG_T* ptTextArray;

	//! A data for buffer count	
	unsigned int dwBufferCount;

	//! A pointer data for overlay buffer config (The array of buffer. NULL item: disable)
	const VMF_OVERLAY_BUF_CONFIG_T* ptBufArray;

	//! A data for extra info
	VMF_OVERLAY_EXTRA_INFO_T tExtraInfo;
} vmf_overlay_config_t;
typedef vmf_overlay_config_t VMF_OVERLAY_CONFIG_T;

#ifdef __cplusplus
}
#endif

#endif
