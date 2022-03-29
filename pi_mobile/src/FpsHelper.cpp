/*
 * FpsHelper.cpp
 *
 *  Created on: Aug 20, 2018
 *      Author: markhsieh
 */

#include "FpsHelper.h"
#include "globaldef.h"

CFpsHelper::CFpsHelper() {
	Reset();
}

CFpsHelper::~CFpsHelper() {

}

void CFpsHelper::Reset() {
	uResetFpsCountTick = 0;
	uFpsCount = 0;
}

bool CFpsHelper::Progress(axc_dword& fps) {
	bool bRet = false;
	axc_dword now = CHeatFinderUtility::GetTickCount();
	if (now - uResetFpsCountTick > 1000) {
		fps = uFpsCount;
		uResetFpsCountTick = now;
		uFpsCount = 0;
		bRet = true;
	}
	uFpsCount++;

	return bRet;
}
