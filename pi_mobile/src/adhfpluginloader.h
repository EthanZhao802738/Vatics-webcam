/*
 * adhfpluginloader.h
 *
 *  Created on: Dec 7, 2017
 *      Author: markhsieh
 */

#ifndef ADHFPLUGINLOADER_H_
#define ADHFPLUGINLOADER_H_

#include "HFPlugin.h"

class adhfplugin_loader {
public:
	adhfplugin_loader();
	virtual ~adhfplugin_loader();

	CHeatFinderPlugin *DynamicLoading(char *path);
    CHeatFinderPlugin *m_pHeatFinder;

protected:
	void *m_pHFPlugin;


	HFPluginCreator_t *m_pfuncCreate;
	HFPluginDestroy_t *m_pfuncDestroy;
};

#endif /* ADHFPLUGINLOADER_H_ */
