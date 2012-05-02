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

#if !defined(XP_UNIX) || defined(XP_MACOSX) || defined(XP_WIN)
NPError OSCALL NP_Initialize(NPNetscapeFuncs* hostApi) {
    npObjFramework_log("NP_Initialize");
    g_hostApi = hostApi;
    return NPERR_NO_ERROR;
}
#else
NPError OSCALL NP_Initialize(NPNetscapeFuncs* hostApi, NPPluginFuncs* pluginApi) {
    npObjFramework_log("NP_Initialize (unix)");
    NP_GetEntryPoints(pluginApi);
    g_hostApi = hostApi;
    return NPERR_NO_ERROR;
}
#endif

NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pluginApi) {
    npObjFramework_log("NP_GetEntryPoints");
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
    return NPERR_NO_ERROR;
}

NPError OSCALL NP_Shutdown(void) {
    npObjFramework_log("NP_Shutdown");
    return NPERR_NO_ERROR;
}


NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, 
        int16_t argc, char* argn[], char* argv[], NPSavedData* saved)
{
    npObjFramework_log("NPP_New");
    try {
        NPPluginObj* obj = createNPPluginObj(instance, g_hostApi);
        if (!obj) return NPERR_GENERIC_ERROR;
        instance->pdata = (void*) obj;
        return obj->initialize(pluginType, mode, argc, argn, argv, saved);
    } catch (NPException exc) { return exc.err; }
}

NPError NPP_Destroy(NPP instance, NPSavedData** save) {
    npObjFramework_log("NPP_Destroy");
    NPPluginObj* obj = asNPPluginObj(instance);
    instance->pdata = NULL;
    if (obj) {
        try {
            obj->destroy(save);
            obj = destroyNPPluginObj(instance, obj);
        } catch (NPException exc) {
            return exc.err;
        }
    }
    return NPERR_NO_ERROR;
}
NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value) {
    npObjFramework_log("NPP_GetValue");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->getValue(variable, value);
    } catch (NPException exc) { return exc.err; }
}
NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value) {
    npObjFramework_log("NPP_SetValue");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->setValue(variable, value);
    } catch (NPException exc) { return exc.err; }
}


NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype) {
    npObjFramework_log("NPP_NewStream");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->newStream(type, stream, seekable, stype);
    } catch (NPException exc) { return exc.err; }
}
NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason) {
    npObjFramework_log("NPP_DestroyStream");
    try {
    NPPluginObj* obj = asNPPluginObj(instance);
    return !obj ? NPERR_INVALID_INSTANCE_ERROR
        : obj->destroyStream(stream, reason);
    } catch (NPException exc) { return exc.err; }
}
int32_t NPP_WriteReady(NPP instance, NPStream* stream) {
    npObjFramework_log("NPP_WriteReady");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->writeReady(stream);
    } catch (NPException exc) { return exc.err; }
}
int32_t NPP_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer) {
    npObjFramework_log("NPP_Write");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->write(stream, offset, len, buffer);
    } catch (NPException exc) { return exc.err; }
}
void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname) {
    npObjFramework_log("NPP_StreamAsFile");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        if (obj) obj->streamAsFile(stream, fname);
    } catch (NPException) { return; }
}
void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData) {
    npObjFramework_log("NPP_URLNotify");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        if (obj) obj->urlNotify(url, reason, notifyData);
    } catch (NPException) { return; }
}
void NPP_URLRedirectNotify(NPP instance, const char* url, int32_t status, void* notifyData) {
    npObjFramework_log("NPP_URLRedirectNotify");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        if (obj) obj->urlRedirectNotify(url, status, notifyData);
    } catch (NPException) { return; }
}


NPError NPP_SetWindow(NPP instance, NPWindow* window) {
    npObjFramework_log("NPP_SetWindow");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->setWindow(window);
    } catch (NPException exc) { return exc.err; }
}
void NPP_Print(NPP instance, NPPrint* platformPrint) {
    npObjFramework_log("NPP_Print");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        if (obj) obj->print(platformPrint);
    } catch (NPException) { return; }
}
int16_t NPP_HandleEvent(NPP instance, void* event) {
    //npObjFramework_log("NPP_HandleEvent");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->handleEvent(event);
    } catch (NPException exc) { return exc.err; }
}
NPBool NPP_GotFocus(NPP instance, NPFocusDirection direction) {
    npObjFramework_log("NPP_GotFocus");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? false : obj->gotFocus(direction);
    } catch (NPException) { return false; }
}
void NPP_LostFocus(NPP instance) {
    npObjFramework_log("NPP_LostFocus");
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        if (obj) obj->lostFocus();
    } catch (NPException) { return; }
}

#if !defined(__APPLE__)
void npObjFramework_log_v(const char* fmt, va_list args) {
    char szBuf[16384];
    vsprintf(szBuf, fmt, args);
#if defined(_WIN32)
    ::OutputDebugStringA(szBuf);
#endif
#if !defined(_WINDOWS)
    puts(szBuf);
#endif
}
void npObjFramework_log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    npObjFramework_log_v(fmt, args);
    va_end(args);
}
#endif
