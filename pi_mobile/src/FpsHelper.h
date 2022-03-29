/*
 * FpsHelper.h
 *
 *  Created on: Aug 20, 2018
 *      Author: Gavin
 */

#ifndef FPSHELPER_H_
#define FPSHELPER_H_

#include "axclib_os_include.h"
#include "axclib.h"

class CFpsHelper {
public:
	CFpsHelper();
	virtual ~CFpsHelper();

	void Reset();
	bool Progress(axc_dword& fps);
	axc_dword Fps() { return uFpsCount; }

private:
	axc_dword uResetFpsCountTick;
	axc_dword uFpsCount;

};

#endif /* FPSHELPER_H_ */
