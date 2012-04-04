/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
 *~ Copyright (C) 2002-2012  TechGame Networks, LLC.              ~*
 *~                                                               ~*
 *~ This library is free software; you can redistribute it        ~*
 *~ and/or modify it under the terms of the MIT style License as  ~*
 *~ found in the LICENSE file included with this distribution.    ~*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma once

namespace NPObjFramework {
    template<typename T>
    NPError setvoid(void* dst, T* src) {
        *((T*)dst) = src;
        return NPERR_NO_ERROR;
    }

    struct NPScriptObj : NPPluginObjBase {
        virtual NPError initStart(NPMIMEType pluginType, uint16_t mode) {
            NPBool bFalse = false;
            host.setValue(NPPVpluginWindowBool, (void *)bFalse);

            return NPERR_NO_ERROR; 
        }

        virtual NPError getValue(NPPVariable variable, void *value) {
            switch (variable) {
            case NPPVpluginNameString:
                return setvoid(value, pluginName());
            case NPPVpluginDescriptionString:
                return setvoid(value, pluginDescription());
            case NPPVpluginScriptableNPObject:
                return setvoid(value, plugin->rootScriptObj());
            }
            return NPERR_GENERIC_ERROR;
        }

        virtual char* pluginName() = 0;
        virtual char* pluginDescription() = 0;
        virtual NPObject* rootScriptObj() = 0
    };
}
