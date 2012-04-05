/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
 *~ Copyright (C) 2002-2012  TechGame Networks, LLC.              ~*
 *~                                                               ~*
 *~ This library is free software; you can redistribute it        ~*
 *~ and/or modify it under the terms of the MIT style License as  ~*
 *~ found in the LICENSE file included with this distribution.    ~*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma once

#include "npapi.h"
#include "npfunctions.h"
#include "npHostObj.h"

namespace NPObjFramework {

    struct NPPluginObj {
        virtual NPError initialize(NPMIMEType pluginType, uint16_t mode, 
            int16_t argc, char* argn[], char* argv[], NPSavedData* saved) = 0;
        virtual NPError destroy(NPSavedData** save) = 0;

        virtual NPError getValue(NPPVariable variable, void *value) { return NPERR_NO_ERROR; }
        virtual NPError setValue(NPNVariable variable, void *value) { return NPERR_NO_ERROR; }

        virtual NPError newStream(NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype) { return NPERR_NO_ERROR; }
        virtual NPError destroyStream(NPStream* stream, NPReason reason) { return NPERR_NO_ERROR; }
        virtual int32_t writeReady(NPStream* stream) { return 0; }
        virtual int32_t write(NPStream* stream, int32_t offset, int32_t len, void* buffer) { return -1; }
        virtual void    streamAsFile(NPStream* stream, const char* fname) {}
        virtual void    urlNotify(const char* url, NPReason reason, void* notifyData) {}
        virtual void    urlRedirectNotify(const char* url, int32_t status, void* notifyData) {}

        virtual void    print(NPPrint* platformPrint) {}
        virtual NPError setWindow(NPWindow* window) { return NPERR_NO_ERROR; }
        virtual int16_t handleEvent(void* event) { return 0; }
        virtual NPBool  gotFocus(NPFocusDirection direction) { return false; }
        virtual void    lostFocus() {}
    };

    inline NPPluginObj* asNSPluginObj(NPP instance) {
        return static_cast<NPPluginObj*>(instance?instance->pdata:NULL);
    }
}

/* implement createNPPluginObj to create concrete instance of an NPPluginObj */
NPObjFramework::NPPluginObj* createNPPluginObj(NPP pluginInstance, NPNetscapeFuncs* hostApi);


namespace NPObjFramework {
    struct NPPluginObjBase : NPPluginObj {
        NPHostObj host;

        NPPluginObjBase(NPP inst, NPNetscapeFuncs* hostApi) 
            : host(inst, hostApi) {}
        virtual ~NPPluginObjBase() {}

        virtual NPError initialize(NPMIMEType pluginType, uint16_t mode, 
            int16_t argc, char* argn[], char* argv[], NPSavedData* saved) 
        { // composed method: initMime followed by initArg() for each arg
            NPError err = initStart(pluginType, mode);
            if (err) return err;
            for (int16_t idx=0; idx<argc; idx++) {
                initArg(argn[idx], argv[idx]);
                if (err) return err;
            }
            err = initFinish(pluginType, mode);
            return err;
        }
        virtual NPError initStart(NPMIMEType pluginType, uint16_t mode) { return NPERR_NO_ERROR; }
        virtual NPError initFinish(NPMIMEType pluginType, uint16_t mode) { return NPERR_NO_ERROR; }
        virtual NPError initArg(char* name, char* value) { return NPERR_NO_ERROR; }

        virtual NPError destroy(NPSavedData** save) {
            delete this;
            return NPERR_NO_ERROR;
        }
    };
}

