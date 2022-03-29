
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
#ifndef __TEXT_RENDER_H__
#define __TEXT_RENDER_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stddef.h>

#define VTXT_TAG "VMF_VTXT_RENDER"

typedef struct draw_info_t draw_info_t;
typedef struct draw_info_t DRAW_INFO_T;

typedef struct txt_renderer_t txt_renderer_t;
typedef struct txt_renderer_t TXT_RENDERER_T;

typedef struct
{
	unsigned int dwFontColor; /**< The color of font content, Byte 0:Y, 1:Cb, 2:Cr  */
	unsigned int dwBorderColor; /**< The color of font border, Byte 0:Y, 1:Cb, 2:Cr */
	unsigned int dwBackColor; /**< The color of font background, Byte 0:Y, 1:Cb, 2:Cr, 3:alpha */
} font_color_t;
typedef font_color_t FONT_COLOR_T;

typedef struct
{
	const char* pszFontPath;  /**< A path to the font file. */
	const char* pszExtraUTF8; /**< Extra utf8 text which want pre-generate in text renderer. NULL: disable */
	float fOutlineWidth;	/**< The outline width of stroke. */
	
	short nFontSize;	/**< The size of font. */
	short nAscent;	/**< The ascender is the vertical distance from the horizontal baseline to the highest ‘character’ coordinate in a font face. */
	short nDescent;	/**< The descender is the vertical distance from the horizontal baseline to the lowest ‘character’ coordinate in a font face. */
	short nHeight;	/**< The baseline-to-baseline distance. */
	FONT_COLOR_T tColorInfo;	/**< The color setting of font. */
	
} font_info_t;

typedef font_info_t	FONT_INFO_T;

typedef struct
{
	unsigned char *y;	/**< Y planar. */
	unsigned char *u;	/**< U planar. */
	unsigned char *v;	/**< V planar. */
	size_t y_size;		/**< Size of Y planar. */
	size_t uv_size;		/**< Size of each V and U planar. */	
	size_t stride;		/**< Stride of surface. */
} txt_surface_t;

typedef txt_surface_t	TXT_SURFACE_T;

/**
 * @struct      VMTK_TEXT_RENDER_CONTEXT_T
 * @brief       Text Render Contexts
 * @sa		VMTK_DMA2D_CONFIG_T
 */	
typedef struct _txt_render_context_t {
	TXT_SURFACE_T text_surface;
	int textoverlay_w;
	int textoverlay_h;	
} TXT_RENDER_CONTEXT_T;

typedef struct
{
	int first_surface_offset;
	int last_surface_offset;
} draw_result_t;
typedef draw_result_t	DRAW_RESULT_T;

/**
 * @brief Function to create the handle of the text render.
 *
 * @return NULL: Failed. Otherwise: Successful.
 */
TXT_RENDERER_T* CreateTextRender();

/**
 * @brief Function to release the resource inside the information structure.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @return Negative value: Failed. Zero: Successful.
 */
int ReleaseTextRender(TXT_RENDERER_T *ptHandle);

/**
 * @brief Function to set the path and size of the font.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] font_info The information about font.
 * @return Negative value: Failed. Zero: Successful.
 */
int SetFont(TXT_RENDERER_T *ptHandle, const FONT_INFO_T* ptFontInfo);

/**
 * @brief Function to set the font color.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] dwFontColor font color.
 * @param[in] dwBorderColor font border color.
 * @param[in] dwBackColor font background color.
 * @return Negative value: Failed. Zero: Successful.
 */
int SetFontcolor(TXT_RENDERER_T *ptHandle, const unsigned int dwFontColor, const unsigned int dwBorderColor, const unsigned int dwBackColor);


/**
 * @brief Function to set the font backgroundcolor.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] font background color.
 * @return Negative value: Failed. Zero: Successful.
 */
int SetFontBGcolor(TXT_RENDERER_T *ptHandle,  const unsigned int font_bg_color);

/**
 * @brief Function to get the font info.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @return NULL: Failed. Others: Successful.
 */
FONT_INFO_T* GetFontInfo(TXT_RENDERER_T *ptHandle);

/**
 * @brief Function to generate the drawing information of one string.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] str The string needs to be generated the drawing information about it.
 * @param[in,out] draw_infos The output drawing information about the input character (It needs to be released by ReleaseDrawInfo function).
 * @see ReleaseDrawInfo(DRAW_INFO_T **, size_t *)
 * @param[in,out] num_of_draw_info The number of drawing information in the draw_infos.
 * @return Negative value: Failed. Zero: Successful.
 */
int GenerateDrawInfo(TXT_RENDERER_T *ptHandle, const char *str, DRAW_INFO_T **draw_infos, size_t *num_of_draw_info);

/**
 * @brief Function to generate extra the drawing information into text_render for further usage.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] str The string needs to be generated the drawing information about it.
 * @return Negative value: Failed. Zero: Successful.
 */
int SetExtraDrawInfo(TXT_RENDERER_T *ptHandle, const char *str);

/**
 * @brief Function to release the resource of the drawing information.
 *
 * @param[in,out] draw_infos The drawing information needs to be released, it will be reset to NULL after we release it successfully.
 * @param[in,out] num_of_draw_info The number of drawing information in the draw_infos, it will be reset to zero after we release it successfully.
 */
void ReleaseDrawInfo(DRAW_INFO_T **draw_infos, size_t *num_of_draw_info);

/**
 * @brief Function to draw one string on the surface according to the drawing information.
 *
 * @param[in] surface The surface needs to be drawn one string on.
 * @param[in] draw_infos The drawing information about the string.
 * @param[in] num_of_draw_info The number of drawing information in the draw_info.
 * @param[in,out] x_offset The x offset of drawing the character (It will be updated to new x offset for drawing next string).
 * @param[in,out] y_offset The y offset of drawing the character (It will be updated to new y offset for drawing next string).
 * @param[in,out] result Some results after drawing.
 */
void RenderDrawInfo(const TXT_SURFACE_T *ptSurface, const DRAW_INFO_T *draw_infos, const size_t num_of_draw_info, 
	int* x_offset, int* y_offset, DRAW_RESULT_T *result);

/**
 * @brief Function to draw one ASCII string on the surface according to the drawing information.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] surface The surface needs to be drawn one string on.
 * @param[in] str The string needs to be drawn.
 * @param[in,out] x_offset The x offset of drawing the character (It will be updated to new x offset for drawing next string).
 * @param[in,out] y_offset The y offset of drawing the character (It will be updated to new y offset for drawing next string).
 * @param[in,out] result Some results after drawing.
 */
void DrawASCIIText(TXT_RENDERER_T *ptHandle, const TXT_SURFACE_T *ptSurface, const char *str, int* x_offset, int	* y_offset, DRAW_RESULT_T *result);

/**
 * @brief Function to get the width and height from the ASCII text.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] str The string needs to be calculated.
 * @param[in] str_len The length of input string.
 * @param[out] width The width of the drawing information for one string.
 * @param[out] height The max height of the drawing information for one string.
 */
void ASCIITextGeometric(TXT_RENDERER_T *ptHandle, const char *str, size_t str_len, int *width, int *height);

/**
 * @brief Function to get the width and height from the UTF8 text.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] str The UTF8 string needs to be calculated.
 * @param[out] width The width of the drawing information for one string.
 * @param[out] height The max height of the drawing information for one string.
 */
void UTF8TextGeometric(TXT_RENDERER_T *ptHandle, const char *str, int *width, int *height);

/**
 * @brief Function to get the width and height from the drawing information.
 *
 * @param[in] draw_info The drawing information about the string.
 * @param[in] num_of_draw_info The number of drawing information in the draw_info.
 * @param[out] width The width of the drawing information for one string.
 * @param[out] height The max height of the drawing information for one string.
 */
void DrawInfoGeometric(const DRAW_INFO_T *draw_infos, size_t num_of_draw_info, int *width, int *height);

/**
 * @brief Function to get the max width of the range of ASCII code or specified ASCII code.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] range_idx_array The array for the range of ASCII code ( ex: {min1, max1, min2, max2} = {48, 57, 97, 100}; // 0~9, a~d ).
 * @param[in] range_idx_array_size The number of elements in the range_idx_array.
 * @param[in] idx_array The array for the ASCII codes ( ex: {75, 68, 81}; // K, D, Q ).
 * @param[in] idx_array_size The number of elements in the idx_array.
 * @return The max width of all specified ASCII texts (-1: Failed).
 */
int GetASCIITextMaxWidth(TXT_RENDERER_T *ptHandle, const size_t *range_idx_array, size_t range_idx_array_size, 
	const size_t *idx_array, size_t idx_array_size);

/**
 * @brief Function to get the max height of the range of ASCII code or specified ASCII code.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] range_idx_array The array for the range of ASCII code ( ex: {min1, max1, min2, max2} = {48, 57, 97, 100}; // 0~9, a~d ).
 * @param[in] range_idx_array_size The number of elements in the range_idx_array.
 * @param[in] idx_array The array for the ASCII codes ( ex: {75, 68, 81}; // K, D, Q ).
 * @param[in] idx_array_size The number of elements in the idx_array.
 * @return The max height of all specified ASCII texts (-1: Failed).
 */
int GetASCIITextMaxHeight(TXT_RENDERER_T *ptHandle, const size_t *range_idx_array, size_t range_idx_array_size,
	const size_t *idx_array, size_t idx_array_size);

/**
 * @brief Function to create a bitmap buf and draw one ASCII string on this buffer.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] str The string needs to be drawn.
 */
void CreateASCIIBitmap(TXT_RENDERER_T *ptHandle, const char *str);

/**
 * @brief Function to draw one ASCII string on the surface according to the drawing information.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] surface The surface needs to be drawn one string on.
 * @param[in] x The x offset of drawing the bitmap.
 * @param[in] y The y offset of drawing the bitmap.
 */
void DrawBitmap(TXT_RENDERER_T *ptHandle, const TXT_SURFACE_T *ptSurface, int x, int y);

/**
 * @brief Function to create an alpha mask and draw one string on this mask according to the drawing information.
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] surface The surface needs to be drawn one string on.
 * @param[in] draw_infos The drawing information about the string.
 * @param[in] num_of_draw_info The number of drawing information in the draw_info.
 * @param[in,out] x_offset The x offset of drawing the character (It will be updated to new x offset for drawing next string).
 * @param[in,out] y_offset The y offset of drawing the character (It will be updated to new y offset for drawing next string).
 * @param[in] extra_space Extra space between characters. 
 */
void RenderMask(TXT_RENDERER_T *ptHandle, const TXT_SURFACE_T *ptSurface, const DRAW_INFO_T *draw_infos, 
	size_t num_of_draw_info, int* x_offset, int* y_offset, int extra_space);

/**
 * @brief Function to draw one ASCII string on this mask. (with user defined color input).
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] surface The surface needs to be drawn one string on.
 * @param[in] color_info Assigned color for mask.
 * @param[in] str The string needs to be drawn.
 * @param[in,out] x_offset The x offset of drawing the character (It will be updated to new x offset for drawing next string).
 * @param[in,out] y_offset The y offset of drawing the character (It will be updated to new y offset for drawing next string).  
 * @param[in] extra_space Extra space between characters.
 */
void DrawASCIIMask(TXT_RENDERER_T *ptHandle, const TXT_SURFACE_T *ptSurface, FONT_COLOR_T* ptColorInfo, 
	const char *str, int* x_offset, int* y_offset, int extra_space);

/**
 * @brief Function to draw one UTF8 string on this mask. (with user defined color input).
 *
 * @param[in] ptHandle The pointer of information structure about the font.
 * @param[in] ptSurface The surface needs to be drawn one string on.
 * @param[in] color_info Assigned color for mask.
 * @param[in] str The string needs to be drawn.
 * @param[in,out] x_offset The x offset of drawing the character (It will be updated to new x offset for drawing next string).
 * @param[in,out] y_offset The y offset of drawing the character (It will be updated to new y offset for drawing next string).
 * @param[in,out] result Some results after drawing.
 */
void DrawUTF8Mask(TXT_RENDERER_T *ptHandle, const TXT_SURFACE_T *ptSurface, FONT_COLOR_T* ptColorInfo, 
	const char *str, int* x_offset, int* y_offset);

#ifdef __cplusplus
}
#endif

#endif

