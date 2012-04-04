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

extern "C" {

    NPError NP_Initialize(NPNetscapeFuncs *browserApi);
    NPError NP_GetEntryPoints(NPPluginFuncs *pluginApi);
    void NP_Shutdown(void);

    NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char* argn[], char* argv[], NPSavedData* saved);
    NPError NPP_Destroy(NPP instance, NPSavedData** save);
    NPError NPP_SetWindow(NPP instance, NPWindow* window);
    NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype);
    NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason);
    int32_t NPP_WriteReady(NPP instance, NPStream* stream);
    int32_t NPP_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer);
    void    NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname);
    void    NPP_Print(NPP instance, NPPrint* platformPrint);
    int16_t NPP_HandleEvent(NPP instance, void* event);
    void    NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData);
    NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value);
    NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value);
    NPBool  NPP_GotFocus(NPP instance, NPFocusDirection direction);
    void    NPP_LostFocus(NPP instance);
    void    NPP_URLRedirectNotify(NPP instance, const char* url, int32_t status, void* notifyData);
    NPError NPP_ClearSiteData(const char* site, uint64_t flags, uint64_t maxAge);
    char**  NPP_GetSitesWithData(void);
}
