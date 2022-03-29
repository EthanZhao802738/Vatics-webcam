/*
 * glogdbgzone.h
 *
 *  Created on: Apr 10, 2018
 *      Author: markhsieh
 */
#ifndef GLOGDBGZONE_H_
#define GLOGDBGZONE_H_

//  glogdbgzone.h
//
//
//  Created by Gavin Chang on 2018/4/9.
//  Copyright (c) 2018年 ADE Technology Inc., All rights reserved.
//

class LogManager; //gavin add

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug Zone
// 32 bits : 32 cases (-1 = all cases)

#define tAll_MSK							   -1
#define tGavinTrace_MSK							1 << 31
#define tNetworkTCPTrace_MSK					1 << 30
#define tNetworkUDPTrace_MSK					1 << 29
#define tThermalSrcTrace_MSK					1 << 28
#define tVisionSrcTrace_MSK						1 << 27
#define tCaptureThermalTrace_MSK				1 << 26
#define tCaptureVisionTrace_MSK					1 << 25
#define tUtilityTrace_MSK						1 << 24
#define tGPIOTrace_MSK							1 << 23
#define tTIMERTrace_MSK							1 << 22
#define tPLUGINTrace_MSK						1 << 21
#define tCONFIGMANAGERTrace_MSK					1 << 20
#define tMonitorTrace_MSK						1 << 19
#define tMODBUSTrace_MSK						1 << 18
#define tRingBuffer_MSK							1 << 17
#define tH264NAL_MSK							1 << 16

#define tAll								   tAll_MSK
#define tNone								   0
#define tGavinTrace							   (LogManager::sm_glogDebugZoneSeed & tGavinTrace_MSK)
#define tNetworkTCPTrace(X)						   (X & tNetworkTCPTrace_MSK)
#define tNetworkUDPTrace(X)						   (X & tNetworkUDPTrace_MSK)
#define tThermalSrcTrace(X)						   (X & tThermalSrcTrace_MSK)
#define tVisionSrcTrace(X)						   (X & tVisionSrcTrace_MSK)
#define tCaptureThermalTrace(X)					   (X & tCaptureThermalTrace_MSK)
#define tCaptureVisionTrace					   (LogManager::sm_glogDebugZoneSeed & tCaptureVisionTrace_MSK)
#define tUtilityTrace(X)						   (X & tUtilityTrace_MSK)
#define tGPIOTrace(X)						   (X & tGPIOTrace_MSK)
#define tTIMERTrace(X)						   (X & tTIMERTrace_MSK)
#define tPLUGINTrace(X)						   (X & tPLUGINTrace_MSK)
#define tCONFIGMANAGERTrace(X)				   (X & tCONFIGMANAGERTrace_MSK)
#define tMonitorTrace(X)				   (X & tMonitorTrace_MSK)
#define tMODBUSTrace(X)				   (X & tMODBUSTrace_MSK)
#define tRingBuffer							    (LogManager::sm_glogDebugZoneSeed & tRingBuffer_MSK)
#define tH264NAL								(LogManager::sm_glogDebugZoneSeed & tH264NAL_MSK)
//#define tAllFunctionTrace						   (tNetworkTCPTrace_MSK | tNetworkUDPTrace_MSK | tThermalSrcTrace_MSK | tVisionSrcTrace_MSK | tCaptureThermalTrace_MSK | tCaptureVisionTrace_MSK | tUtilityTrace_MSK | tGPIOTrace_MSK | tTIMERTrace_MSK)

#define tCLOSETrace_MSK						0
#define tVERBOSETrace_MSK					1 << 31
#define tNORMALTrace_MSK					1 << 30
#define tDEBUGTrace_MSK						1 << 29
#define tWARNTrace_MSK						1 << 28
#define tERRORTrace_MSK						1 << 27
#define tFATALTrace_MSK						1 << 26

#define tVERBOSETrace(X)				   (X & (tVERBOSETrace_MSK | tNORMALTrace_MSK | tDEBUGTrace_MSK | tWARNTrace_MSK | tERRORTrace_MSK | tFATALTrace_MSK))
#define tNORMALTrace(X)					   (X & (tNORMALTrace_MSK | tDEBUGTrace_MSK | tWARNTrace_MSK | tERRORTrace_MSK | tFATALTrace_MSK))
#define tDEBUGTrace(X)					   (X & (tDEBUGTrace_MSK | tWARNTrace_MSK | tERRORTrace_MSK | tFATALTrace_MSK))
#define tWARNTrace(X)					   (X & (tWARNTrace_MSK | tERRORTrace_MSK | tFATALTrace_MSK))
#define tERRORTrace(X)					   (X & (tERRORTrace_MSK | tFATALTrace_MSK))
#define tFATALTrace(X)					   (X & tFATALTrace_MSK)
#define tSetOutLvlTrace(X,Y)			   ((X & ~(tVERBOSETrace_MSK | tNORMALTrace_MSK | tDEBUTTrace_MSK | tERRORTrace_MSK | tFATALTrace_MSK)) | Y)
#endif /* GLOGDBGZONE_H_ */
