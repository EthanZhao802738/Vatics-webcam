/*
 * adhfpluginloader.cpp
 *
 *  Created on: Dec 7, 2017
 *      Author: markhsieh
 */

#include <dlfcn.h>
#include <unistd.h>
#include "adhfpluginloader.h"

adhfplugin_loader::adhfplugin_loader() 
{
	// TODO Auto-generated constructor stub
    m_pHFPlugin = NULL;
    m_pHeatFinder = NULL;
    m_pfuncDestroy = NULL;
    m_pfuncCreate = NULL;
}

adhfplugin_loader::~adhfplugin_loader() 
{
	// TODO Auto-generated destructor stub
    if (m_pHeatFinder && m_pfuncDestroy)
        m_pfuncDestroy(m_pHeatFinder);

    if (m_pHFPlugin)
        dlclose(m_pHFPlugin);
}

CHeatFinderPlugin *adhfplugin_loader::DynamicLoading(char *path)
{
    CHeatFinderPlugin *pRst = NULL;

    m_pHFPlugin = dlopen(path, RTLD_LAZY);
    //void *pT = dlopen("/home/pi/Projects/build-adeHFMQTTPlugin-Desktop-Debug/libadeHFMQTTPlugin.so.1.0.0", RTLD_LAZY);
    if (!m_pHFPlugin)
    {
    	printf("E: [Plugin Loader] Cann't load plugin %s \n", dlerror());
        return pRst;
    }

    // reset errors
    dlerror();

    m_pfuncCreate = (HFPluginCreator_t*) dlsym(m_pHFPlugin, "HFPluginCreator");
    const char* dlsym_error = dlerror();
    if (dlsym_error)
    {
    	printf("E: [Plugin Loader] Cannot load symbol create: %s\n", dlsym_error);
        return pRst;
    }

    m_pfuncDestroy = (HFPluginDestroy_t*) dlsym(m_pHFPlugin, "HFPluginDestroy");
    dlsym_error = dlerror();
    if (dlsym_error)
    {
    	printf("E: [Plugin Loader] Cannot load symbol destroy: %s\n", dlsym_error);
        return pRst;
    }

    pRst = m_pfuncCreate();
    m_pHeatFinder = pRst;

    return pRst;
}
