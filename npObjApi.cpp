/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
 *~ Copyright (C) 2002-2012  TechGame Networks, LLC.              ~*
 *~                                                               ~*
 *~ This library is free software; you can redistribute it        ~*
 *~ and/or modify it under the terms of the MIT style License as  ~*
 *~ found in the LICENSE file included with this distribution.    ~*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include "npObjFramework.h"
using namespace NPObjFramework;

static NPNetscapeFuncs* g_hostApi;

#if !defined(XP_UNIX) || defined(XP_MACOSX)
NPError NP_Initialize(NPNetscapeFuncs* hostApi) {
    g_hostApi = hostApi;
    return NPERR_NO_ERROR;
}
#else
NPError NP_Initialize(NPNetscapeFuncs* hostApi, NPPluginFuncs* pluginApi) {
    NP_GetEntryPoints(pluginApi);
    g_hostApi = hostApi;
    return NPERR_NO_ERROR;
}
#endif

NPError NP_GetEntryPoints(NPPluginFuncs* pluginApi) {
    uint16_t lastEntry = pluginApi->size - sizeof(void*);
    if (lastEntry < offsetof(NPPluginFuncs, setvalue))
        return NPERR_INVALID_FUNCTABLE_ERROR;

    pluginApi->newp = NPP_New;
    pluginApi->destroy = NPP_Destroy;
    pluginApi->setwindow = NPP_SetWindow;
    pluginApi->newstream = NPP_NewStream;
    pluginApi->destroystream = NPP_DestroyStream;
    pluginApi->asfile = NPP_StreamAsFile;
    pluginApi->writeready = NPP_WriteReady;
    pluginApi->write = (NPP_WriteProcPtr)NPP_Write;
    pluginApi->print = NPP_Print;
    pluginApi->event = NPP_HandleEvent;
    pluginApi->urlnotify = NPP_URLNotify;
    pluginApi->getvalue = NPP_GetValue;
    pluginApi->setvalue = NPP_SetValue;

    if (lastEntry >= offsetof(NPPluginFuncs, lostfocus)) {
        pluginApi->gotfocus = NPP_GotFocus;
        pluginApi->lostfocus = NPP_LostFocus;
    }
    if (lastEntry >= offsetof(NPPluginFuncs, urlredirectnotify)) {
        pluginApi->urlredirectnotify = NPP_URLRedirectNotify;
    }
    if (lastEntry >= offsetof(NPPluginFuncs, clearsitedata)) {
        pluginApi->clearsitedata = NPP_ClearSiteData;
    }
    if (lastEntry >= offsetof(NPPluginFuncs, getsiteswithdata)) {
        pluginApi->getsiteswithdata = NPP_GetSitesWithData;
    }
    return NPERR_NO_ERROR;
}

void NP_Shutdown(void) {
}


NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, 
        int16_t argc, char* argn[], char* argv[], NPSavedData* saved);
{
    //NPBool bFalse = false;
    //g_hostApi->setvalue(instance, NPPVpluginWindowBool, (void *)bFalse);

    try {
        NPPluginObj* obj = createNPPluginObj(instance, g_hostApi);
        if (!obj) return NPERR_GENERIC_ERROR;
        instance->pdata = (void*) obj;
        return obj->initialize(pluginType, mode, argc, argn, argv, saved);
    } catch (NPException exc) { return exc->err; }
}

NPError NPP_Destroy(NPP instance, NPSavedData** save) {
    NPPluginObj* obj = asNSPluginObj(instance);
    if (obj) {
        try {
            obj->destroy(save);
            instance->npp = NULL;
        } catch (NPException exc) {
            instance->npp = NULL;
            return exc->err;
        }
    }
    return NPERR_NO_ERROR;
}
NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->getValue(variable, value);
    } catch (NPException exc) { return exc->err; }
}
NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->setValue(variable, value);
    } catch (NPException exc) { return exc->err; }
}


NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->newStream(type, stream, seekable, stype);
    } catch (NPException exc) { return exc->err; }
}
NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason) {
    try {
    NPPluginObj* obj = asNSPluginObj(instance);
    return !obj?NPERR_INVALID_INSTANCE_ERROR
        : obj->destroyStream(stream, reason);
    } catch (NPException exc) { return exc->err; }
}
int32_t NPP_WriteReady(NPP instance, NPStream* stream) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->writeReady(stream);
    } catch (NPException exc) { return exc->err; }
}
int32_t NPP_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->write(stream, offset, len, buffer);
    } catch (NPException exc) { return exc->err; }
}
void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->streamAsFile(stream, fname);
    } catch (NPException exc) { return exc->err; }
}
void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->urlNotify(url, reason, notifyData);
    } catch (NPException exc) { return exc->err; }
}
void NPP_URLRedirectNotify(NPP instance, const char* url, int32_t status, void* notifyData) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->urlRedirectNotify(url, status, notifyData);
    } catch (NPException exc) { return exc->err; }
}


NPError NPP_SetWindow(NPP instance, NPWindow* window) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->setWindow(window);
    } catch (NPException exc) { return exc->err; }
}
void NPP_Print(NPP instance, NPPrint* platformPrint) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->print(platformPrint);
    } catch (NPException exc) { return exc->err; }
}
int16_t NPP_HandleEvent(NPP instance, void* event) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->handleEvent(event);
    } catch (NPException exc) { return exc->err; }
}
NPBool NPP_GotFocus(NPP instance, NPFocusDirection direction) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->gotFocus(direction);
    } catch (NPException exc) { return exc->err; }
}
void NPP_LostFocus(NPP instance) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->lostFocus();
    } catch (NPException exc) { return exc->err; }
}


NPError NPP_ClearSiteData(const char* site, uint64_t flags, uint64_t maxAge) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->clearSiteData(site, flags, maxAge);
    } catch (NPException exc) { return exc->err; }
}
char** NPP_GetSitesWithData(void) {
    try {
        NPPluginObj* obj = asNSPluginObj(instance);
        return !obj?NPERR_INVALID_INSTANCE_ERROR
            : obj->getSitesWithData();
    } catch (NPException exc) { return exc->err; }
}

