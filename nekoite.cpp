/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab */
#if !defined(_WIN32)
#include <sys/sysctl.h>
#endif
#include "nekoite.h"

namespace Nekoite {
    NPNetscapeFuncs* NPHostObj::api = NULL;
    std::set<NPHostObj*> NPHostObj::_allHosts;

    bool NPHostObj::onPluginDestroyed(bool internal) {
        instance = NULL;
        if (_allObjects.size() == 0) {
            delete this;
            return true;
        } else if (!internal) {
            std::set<NPScriptObj*>::iterator
                iter = _allObjects.begin(),
                end = _allObjects.end();
            for(;iter!=end;iter++)
                (*iter)->onPluginDestroyed();
        }
        return false; }

    void NPHostObj::_on_timer_s(NPP npp, uint32_t timerID) {
        NPHostObj* host = asNPHostObj(npp);
        if (host) host->_dispatchTimerEvent(timerID); }
    void NPHostObj::_dispatchTimerEvent(uint32_t timerID) {
        ObjTimerMap::iterator iter=_allTimers.find(timerID);
        if (iter == _allTimers.end()) return;
        NPScriptObj* obj = iter->second;
        if (obj && _allObjects.count(obj)>0)
            obj->onTimer(timerID);
        else _allTimers.erase(iter); }
    uint32_t NPHostObj::scheduleTimer(NPScriptObj* obj, uint32_t interval, bool repeat) {
        if (!obj) return 0;
        uint32_t timerID = scheduleTimer(interval, repeat, _on_timer_s);
        _allTimers[timerID] = obj;
        return timerID;}
    void NPHostObj::unscheduleTimer(NPScriptObj* obj) {
        ObjTimerMap::iterator iter=_allTimers.begin(), end=_allTimers.end();
        for(;iter!=end;iter++)
            if (iter->second == obj)
                unscheduleTimer(iter->first); }
}
using namespace Nekoite;

extern "C" {
    NPError OSCALL NP_Initialize(NPNetscapeFuncs *browserApi);
    NPError OSCALL NP_GetEntryPoints(NPPluginFuncs *pluginApi);
    NPError OSCALL NP_Shutdown(void);
}

#if !defined(XP_UNIX) || defined(XP_MACOSX) || defined(XP_WIN)
NPError OSCALL NP_Initialize(NPNetscapeFuncs* hostApi) {
    NPHostObj::api = hostApi;
    return NPERR_NO_ERROR;
}
#else
NPError OSCALL NP_Initialize(NPNetscapeFuncs* hostApi, NPPluginFuncs* pluginApi) {
    NPHostObj::api = hostApi;
    NP_GetEntryPoints(pluginApi);
    return NPERR_NO_ERROR;
}
#endif

NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pluginApi) {
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
    if (lastEntry >= offsetof(NPPluginFuncs, getsiteswithdata)) {
        pluginApi->clearsitedata = NPP_ClearSiteData;
        pluginApi->getsiteswithdata = NPP_GetSitesWithData;
    }
    NekoiteRoot* root = nekoiteRoot();
    if (root) root->initPlugin(pluginApi);
    return NPERR_NO_ERROR;
}

NPError OSCALL NP_Shutdown(void) {
    return NPERR_NO_ERROR;
}


NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16_t mode, 
        int16_t argc, char* argn[], char* argv[], NPSavedData* saved)
{
    try {
        NekoiteRoot* root = nekoiteRoot();
        if (!root) return NPERR_GENERIC_ERROR;
        NPPluginObj* obj = root->create(instance);
        if (!obj) return NPERR_GENERIC_ERROR;
        instance->pdata = (void*) obj;
        return obj->initialize(pluginType, mode, argc, argn, argv, saved);
    } catch (NPException exc) { return exc.err; }
}

NPError NPP_Destroy(NPP instance, NPSavedData** save) {
    NPPluginObj* obj = asNPPluginObj(instance);
    if (obj) {
        try {
            obj->destroy(save);
            delete obj; obj = NULL;
            instance->pdata = NULL;
        } catch (NPException exc) {
            instance->pdata = NULL;
            return exc.err;
        }
    }
    return NPERR_NO_ERROR;
}
NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->getValue(variable, value);
    } catch (NPException exc) { return exc.err; }
}
NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->setValue(variable, value);
    } catch (NPException exc) { return exc.err; }
}


NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16_t* stype) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->newStream(type, stream, seekable, stype);
    } catch (NPException exc) { return exc.err; }
}
NPError NPP_DestroyStream(NPP instance, NPStream* stream, NPReason reason) {
    try {
    NPPluginObj* obj = asNPPluginObj(instance);
    return !obj ? NPERR_INVALID_INSTANCE_ERROR
        : obj->destroyStream(stream, reason);
    } catch (NPException exc) { return exc.err; }
}
int32_t NPP_WriteReady(NPP instance, NPStream* stream) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->writeReady(stream);
    } catch (NPException exc) { return exc.err; }
}
int32_t NPP_Write(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buffer) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->write(stream, offset, len, buffer);
    } catch (NPException exc) { return exc.err; }
}
void NPP_StreamAsFile(NPP instance, NPStream* stream, const char* fname) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        if (obj) obj->streamAsFile(stream, fname);
    } catch (NPException) { return; }
}
void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        if (obj) obj->urlNotify(url, reason, notifyData);
    } catch (NPException) { return; }
}
void NPP_URLRedirectNotify(NPP instance, const char* url, int32_t status, void* notifyData) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        if (obj) obj->urlRedirectNotify(url, status, notifyData);
    } catch (NPException) { return; }
}


NPError NPP_SetWindow(NPP instance, NPWindow* window) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->setWindow(window);
    } catch (NPException exc) { return exc.err; }
}
void NPP_Print(NPP instance, NPPrint* platformPrint) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        if (obj) obj->print(platformPrint);
    } catch (NPException) { return; }
}
int16_t NPP_HandleEvent(NPP instance, void* event) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? NPERR_INVALID_INSTANCE_ERROR
            : obj->handleEvent(event);
    } catch (NPException exc) { return exc.err; }
}
NPBool NPP_GotFocus(NPP instance, NPFocusDirection direction) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        return !obj ? false : obj->gotFocus(direction);
    } catch (NPException) { return false; }
}
void NPP_LostFocus(NPP instance) {
    try {
        NPPluginObj* obj = asNPPluginObj(instance);
        if (obj) obj->lostFocus();
    } catch (NPException) { return; }
}

NPError NPP_ClearSiteData(const char* site, uint64_t flags, uint64_t maxAge) {
    try {
        NekoiteRoot* root = nekoiteRoot();
        return root ? root->clearSiteData(site, flags, maxAge) : NPERR_GENERIC_ERROR;
    } catch (NPException) { return NPERR_GENERIC_ERROR; }
}
char** NPP_GetSitesWithData(void) {
    try {
        NekoiteRoot* root = nekoiteRoot();
        return root ? root->getSitesWithData() : NULL;
    } catch (NPException) { return NULL; }
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *~ Utilitiy Implementations and Statics
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

namespace Nekoite {
    #if defined(_WIN32)
    void log_v(const char* fmt, va_list args) {
        char szBuf[4096];
        vsnprintf(szBuf, sizeof(szBuf), fmt, args);
        szBuf[4095] = 0;
        ::OutputDebugStringA(szBuf); }
    #elif !defined(__APPLE__)
    void log_v(const char* fmt, va_list args) {
        vfprintf(stderr, fmt, args); }
    #endif

    void log(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_v(fmt, args);
        va_end(args); }

    #if defined(_WIN32)
    bool waitForDebugger() {
        while (!::IsDebuggerPresent())
            ::Sleep(100);
        return true; }
    #else
    bool waitForDebugger() {
        int mib[4];
        struct kinfo_proc info;
        size_t size;

        info.kp_proc.p_flag = 0;
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PID;
        mib[3] = getpid();

        while (true) {
            size = sizeof(info);
            info.kp_proc.p_flag = 0;
            sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, NULL, 0);
            if (info.kp_proc.p_flag & P_TRACED)
                return true;
            else ::sleep(100);
        }
        return true; }
    #endif
}

