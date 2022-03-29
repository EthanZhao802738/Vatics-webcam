
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
#ifndef FEC_LAYOUT_H
#define FEC_LAYOUT_H

#include <vmf/config_fec.h>
#include <vmf/config_ifp.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A structure for FEC stream output config
 */
typedef struct
{
	unsigned int dwOutputId;          //! VSRC stream output index
	VMF_LAYOUT_T tLayout;             //! Layout of stream
	unsigned int bCoeffOnly;          //! Update FEC data, skip reset output
	VMF_FEC_GRID_SIZE_TYPE eGridSize; //! Grid size of FEC transformation
	unsigned int dwClearBackColor;    //! Stream background color, Byte 0: on-off flag, 1: y color, 2:u color, 3:v color
	VMF_FEC_METHOD eLayoutMethod;     //! Method of generating FEC data for FEC transformation. 0: GTR, 1: CGE
	VMF_ENC_SPEC_T tEncSpec;          //! Determine encoding spec of ISP output stream
	unsigned int bDuplexMode;         //! 0: ISP duplex mode off, 1:ISP duplex mode on
} VMF_FEC_LYT_CONFIG_T;

/**
 * A structure for detatil cell config of stream
 */
typedef struct
{
	VMF_FEC_COEF_MODE eFecMode;    //! FEC mode
	void              *pFecConfig; //! FEC config according to eFecMode
	unsigned int      dwFlag;      //! 0: Default, 1: skip when update FEC data only	
	unsigned int      dwIspLinePixels; //! ISPE line pixel number. Range: 0~1280
	                                   //! 0: Default, other value should consult with technical support
} VMF_FEC_CELL_CONFIG_T;

/*!
 * @brief Function to disable fisheye correction. Set frame layout one single view, the original fisheye view.
 *
 * @param[in] ptHandle The handle of VMF fec source
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] ptCellConfig A pointer to VMF_FEC_CELL_CONFIG_T structure, set NULL will output original view
 * @return Success: 0  Fail: negative integer
 */
int VMF_FEC_LYT_Single(VMF_VSRC_HANDLE_T *ptHandle, VMF_FEC_LYT_CONFIG_T *ptLytConfig, VMF_FEC_CELL_CONFIG_T *ptCellConfig);

/*!
 * @brief Function to set frame layout one single view and output subview offset.
 *
 * @param[in] ptHandle The handle of VMF fec source
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] ptCellConfig A pointer to VMF_FEC_CELL_CONFIG_T structure, set NULL will output original view
 * @param[in] rectangle A matrix to set output location
 * @param[in] offset A matrix to set subview start position
 * @return Success: 0  Fail: negative integer
 */
int VMF_FEC_LYT_Single_Ext(VMF_VSRC_HANDLE_T *ptHandle, VMF_FEC_LYT_CONFIG_T *ptLayout, VMF_FEC_CELL_CONFIG_T *ptCellConfig,
		unsigned int rectangle[2], unsigned int offset[2]);
/*!
 * @brief Function to set frame layout to three divisions.(Major Top)
 *
 * @param[in] ptHandle The handle of VMF fec source
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] ptCellConfig A pointer to array of 3 VMF_FEC_CELL_CONFIG_T structure
			1. A cell will output original view if its config pointer is NULL
			2. VMF_FEC_ROI_T* roi in VMF_FEC_*_config_t will be ignored
 * @return Success: 0  Fail: negative integer
 *
 * @note The order of quad cells
 *  -------------
 *  |     0     |
 *  |-----------|
 *  |  1  |  2  |
 *  -------------
 */
int VMF_FEC_LYT_Triple_Major_Top(VMF_VSRC_HANDLE_T *ptHandle, VMF_FEC_LYT_CONFIG_T *ptLytConfig, VMF_FEC_CELL_CONFIG_T *ptCellConfig);

/*!
 * @brief Function to set frame layout to three divisions.(Major Bottom)
 *
 * @param[in] ptHandle The handle of VMF fec source
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] ptCellConfig A pointer to array of 3 VMF_FEC_CELL_CONFIG_T structure
			1. A cell will output original view if its config pointer is NULL
			2. VMF_FEC_ROI_T* roi in VMF_FEC_*_config_t will be ignored
 * @return Success: 0  Fail: negative integer
 *
 * @note The order of quad cells
 *  -------------
 *  |  0  |  1  |
 *  |-----------|
 *  |     2     |
 *  -------------
 */
int VMF_FEC_LYT_Triple_Major_Bottom(VMF_VSRC_HANDLE_T *ptHandle, VMF_FEC_LYT_CONFIG_T *ptLytConfig,
	VMF_FEC_CELL_CONFIG_T*ptCellConfig);

/*!
* @brief Function to set frame layout to three divisions.(Major Right)
 *
 * @param[in] ptHandle The handle of VMF fec source
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] ptCellConfig A pointer to array of 3 VMF_FEC_CELL_CONFIG_T structure
			1. A cell will output original view if its config pointer is NULL
			2. VMF_FEC_ROI_T* roi in VMF_FEC_*_config_t will be ignored
 * @param[in] fLeftRatio The ratio of left width (0.0f ~ 1.0f)
 * @return Success: 0  Fail: negative integer
 *
 * @note The order of quad cells
 *  -------------
 *  |  0  |     |
 *  |-----|  2  |
 *  |  1  |     |
 *  -------------
 */
int VMF_FEC_LYT_Triple_Major_Right(VMF_VSRC_HANDLE_T *ptHandle, VMF_FEC_LYT_CONFIG_T *ptLytConfig,
	VMF_FEC_CELL_CONFIG_T *ptCellConfig, float fLeftRatio);

/*!
 * @brief Function to set frame layout to four divisions
 *
 * @param[in] ptHandle The handle of VMF fec source
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] ptCellConfig A pointer to array of 4 VMF_FEC_CELL_CONFIG_T structure
			1. A cell will output original view if its config pointer is NULL
			2. VMF_FEC_ROI_T* roi in VMF_FEC_*_config_t will be ignored
 * @return Success: 0  Fail: negative integer
 *
 * @note The order of quad cells
 *  -------------
 *  |  0  |  1  |
 *  |-----+-----|
 *  |  2  |  3  |
 *  -------------
 */
int VMF_FEC_LYT_Quad(VMF_VSRC_HANDLE_T *ptHandle, VMF_FEC_LYT_CONFIG_T *ptLytConfig, VMF_FEC_CELL_CONFIG_T *ptCellConfig);

/*!
 * @brief Function to set frame layout to four divisions of H cuts
 *
 * @param[in] ptHandle The handle of VMF fec source
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] ptCellConfig A pointer to array of 4 VMF_FEC_CELL_CONFIG_T structure
			1. A cell will output original view if its config pointer is NULL
			2. VMF_FEC_ROI_T* roi in VMF_FEC_*_config_t will be ignored
 * @return Success: 0  Fail: negative integer
 *
 * @note The order of quad cells
 *  -------------
 *  |  |  2  |  |
 *  |1 |-----|4 |
 *  |  |  3  |  |
 *  -------------
 */
int VMF_FEC_LYT_Quad_Hcut(VMF_VSRC_HANDLE_T *ptHandle, VMF_FEC_LYT_CONFIG_T *ptLytConfig, VMF_FEC_CELL_CONFIG_T *ptCellConfig);

/*!
 * @brief Function to set frame layout to panorama 360 view. For easy to show on 16:9/4:3 form, we cut one panorama 360 view into two separated strips lying up and down.
 *
 * @param[in] ptHandle The handle of VMF fec source
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] ptCellConfig A pointer to VMF_FEC_CELL_CONFIG_T structure
 * @return Success: 0  Fail: negative integer
 */
int VMF_FEC_LYT_P360_Separated(VMF_VSRC_HANDLE_T *ptHandle, VMF_FEC_LYT_CONFIG_T *ptLytConfig,
	VMF_FEC_CELL_CONFIG_T *ptCellConfig);

/*!
 * @brief Function to set frame layout to two divisions.
 *
 * @param[in] ptHandle The handle of VMF fec source
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] ptCellConfig A pointer to array of 2 VMF_FEC_CELL_CONFIG_T structure
			1. A cell will output original view if its config pointer is NULL
			2. VMF_FEC_ROI_T* roi in VMF_FEC_*_config_t will be ignored
 * @return Success: 0  Fail: negative integer
 *
 * @note The order of quad cells
 *  -------------
 *  |     0     |
 *  |-----------|
 *  |     1     |
 *  -------------
 */
int VMF_FEC_LYT_Double_Horizontal( VMF_VSRC_HANDLE_T *ptHandle, VMF_FEC_LYT_CONFIG_T *ptLytConfig,
	VMF_FEC_CELL_CONFIG_T *ptCellConfig);

/*!
 * @brief Function to set frame layout to two vertical divisions
 *
 * @param[in] ptHandle The handle of VMF fec source
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] cell_configs A pointer to array of 2 VMF_FEC_CELL_CONFIG_T structure
			1. A cell will output original view if its config pointer is NULL
			2. VMF_FEC_ROI_T* roi in VMF_FEC_*_config_t will be ignored
 * @return Success: 0  Fail: negative integer
 *
 * @note The order of two vertical cells
 *  -------------
 *  |     |     |
 *  |  0  |  1  |
 *  |     |     |
 *  -------------
 */
int VMF_FEC_LYT_Double_Vertical(VMF_VSRC_HANDLE_T *ptHandle, VMF_FEC_LYT_CONFIG_T *ptLytConfig,
	VMF_FEC_CELL_CONFIG_T *ptCellConfig);

/*!
 * @brief Function to set frame layout to two divisions by input parameters.
 *
 * @param[in] ptHandle The handle of VMF_VSRC_HANDLE_T
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] ptCellConfig A pointer to array of 2 VMF_FEC_CELL_CONFIG_T structure
			1. A cell will output original view if its config pointer is NULL
			2. VMF_FEC_ROI_T* roi in VMF_FEC_*_config_t will be ignored
 * @param[in] rectangle The width and height of the two divisions.
 * @param[in] offset The position of the two divisions.
 * @return Success: 0  Fail: negative integer
 */
int VMF_FEC_LYT_Double_Customize(VMF_VSRC_HANDLE_T *ptHandle, VMF_FEC_LYT_CONFIG_T *ptLytConfig,
	VMF_FEC_CELL_CONFIG_T *ptCellConfig, const unsigned int  rectangle[][2], const unsigned int  offset[][2]);

/*!
 * @brief Function to get FEC info needed for pixel lookup .
 *
 * @param[in] ptHandle The handle of VMF_VSRC_HANDLE_T
 * @param[in] ptLytConfig A pointer to the VMF_FEC_LYT_CONFIG_T structure
 * @param[in] ptCellConfig A pointer to VMF_FEC_CELL_CONFIG_T structure
 * @return a pointer to VMF_FEC_INFO_T structure
 */
VMF_FEC_INFO_T* VMF_LYT_FEC_Info_Init (VMF_VSRC_HANDLE_T     *ptHandle,VMF_FEC_LYT_CONFIG_T  *ptLayout,
	VMF_FEC_CELL_CONFIG_T *ptCellConfig);

/*!
 * @brief Function to release FEC info.
 *
 * @param[in] ptFecInfo A pointer to the VMF_FEC_INFO_T structure
 * @return Success: 0  Fail: negative integer
 */
int VMF_LYT_FEC_Info_Release(VMF_FEC_INFO_T* ptFecInfo);

#ifdef __cplusplus
}
#endif

#endif
