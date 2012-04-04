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

    class NPPluginObj {
    public:
        virtual NPError initialize(NPMIMEType pluginType, uint16_t mode, 
            int16_t argc, char* argn[], char* argv[], NPSavedData* saved) = 0
        virtual NPError destroy(NPSavedData** save) = 0;

        virtual NPError getValue(NPPVariable variable, void *value) { return NPERR_NO_ERROR; }
        virtual NPError setValue(NPPVariable variable, void *value) { return NPERR_NO_ERROR; }

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

        virtual NPError clearSiteData(const char* site, uint64_t flags, uint64_t maxAge) { return NPERR_NO_ERROR; }
        virtual char**  getSitesWithData(void) { return NULL; }
    };

    inline NPPluginObj* asNSPluginObj(NPP instance) {
        return (!instance || !instance->npp) ? NULL
            : static_cast<NPPluginObj*>(instance->npp);
    }

}

NPObjFramework::NPPluginObj* createNPPluginObj(NPP pluginInstance, NPNetscapeFuncs* hostApi);

