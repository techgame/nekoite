/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab */
#pragma once
#include "nekoite.h"

namespace Nekoite {
    struct NPException { NPError err; NPException(NPError err_p):err(err_p){} };
    template<NPError err_v> struct NPError_t : NPException { NPError_t():NPException(err_v){} };
    typedef NPError_t<NPERR_INVALID_FUNCTABLE_ERROR> NPHostAPIError;
    typedef NPError_t<NPERR_INVALID_INSTANCE_ERROR> NPHostInstanceError;

    template<typename T> inline T* apiFn(T* fn) {
        if (!fn) throw NPHostAPIError(); return fn; }
    template<typename T> inline T* apiFn(T* fn, NPP instance) {
        if (!fn) throw NPHostAPIError();
        if (!instance) throw NPHostInstanceError();
        return fn; }

    struct NPHostObj {
        NPP instance;
        static NPNetscapeFuncs* api;
        static std::set<NPHostObj*> _allHosts;

        NPHostObj(NPP instance_p) : instance(instance_p)
        { _allHosts.insert(this); }
        ~NPHostObj()
        { _allHosts.erase(this); }

        inline operator NPP() const { return instance; }
        NPPluginObj* plugin() const {
            return !instance ? static_cast<NPPluginObj*>(instance->pdata) : NULL; }

        bool onPluginDestroyed(bool internal=false);

        std::set<NPScriptObj*> _allObjects;
        NPHostObj* trackObj(NPScriptObj* obj) {
            _allObjects.insert(obj);
            return this; }
        NPHostObj* untrackObj(NPScriptObj* obj) {
            _allObjects.erase(obj);
            if (!instance) onPluginDestroyed(true);
            return NULL; }

        /* General API */
        template <typename T, typename R>
        inline R releaseWithResult(T obj, R res) { release(obj); return res; }
        template <typename T>
        inline T* memFreeEx(T* ptr) { memFree((void*)ptr); return NULL; }

        void status(const char* message) { return apiFn(api->status, instance)(instance, message); }
        const char* userAgent() { return apiFn(api->uagent, instance)(instance); }

        void* memAlloc(uint32_t size) { return apiFn(api->memalloc)(size); }
        void memFree(void* ptr) { apiFn(api->memfree)(ptr); }
        uint32_t memFlush(uint32_t size) { return apiFn(api->memflush)(size); }

        void reloadPlugins(NPBool reloadPages) { return apiFn(api->reloadplugins)(reloadPages); }
        NPError getValue(NPNVariable variable, void *value) {
            return apiFn(api->getvalue, instance)(instance, variable, value); }
        NPError setValue(NPPVariable variable, void *value) {
            return apiFn(api->setvalue, instance)(instance, variable, value); }
        void pluginThreadAsyncCall(void (*func) (void *), void *userData) {
            return apiFn(api->pluginthreadasynccall, instance)(instance, func, userData); }

        /* Timer API */
        typedef std::map<uint32_t, NPScriptObj*> ObjTimerMap;
        ObjTimerMap _allTimers;
        void _dispatchTimerEvent(uint32_t timerID); 
        static void _on_timer_s(NPP npp, uint32_t timerID);
        
        uint32_t scheduleTimer(uint32_t interval, NPBool repeat, void (*timerFunc)(NPP npp, uint32_t timerID)) {
            return apiFn(api->scheduletimer, instance)(instance, interval, repeat, timerFunc); }
        uint32_t scheduleTimer(NPScriptObj* obj, uint32_t interval, bool repeat=true);

        void unscheduleTimer(uint32_t timerID) {
            _allTimers.erase(timerID);
            if (instance) apiFn(api->unscheduletimer, instance)(instance, timerID); }
        void unscheduleTimer(NPScriptObj* obj);

        /* Scripting Support */

        NPObject* domWindow(NPObject* res=NULL) {
            return (NPERR_NO_ERROR == getValue(NPNVWindowNPObject, &res)) ? res : NULL; }
        NPObject* domElement(NPObject* res=NULL) {
            return (NPERR_NO_ERROR == getValue(NPNVPluginElementNPObject, &res)) ? res : NULL; }

        NPObject* createObject(NPClass *aClass) {
            return apiFn(api->createobject, instance)(instance, aClass); }
        NPObject* retainObject(NPObject *npobj) {
            return apiFn(api->retainobject)(npobj); }
        inline NPObject* retain(NPObject *npobj) { return retainObject(npobj); }
        NPObject* releaseObject(NPObject *npobj) {
            apiFn(api->releaseobject)(npobj); return NULL; }
        inline NPObject* release(NPObject *npobj) { return releaseObject(npobj); }
        bool invoke(NPObject *npobj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return apiFn(api->invoke, instance)(instance, npobj, methodName, args, argCount, result); }
        bool invokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return apiFn(api->invokeDefault, instance)(instance, npobj, args, argCount, result); }
        bool evalElement(const NPString *script, NPVariant *result) { NPObject* domElem = domElement();
            return releaseWithResult(domElem, apiFn(api->evaluate, instance)(instance, domElem, (NPString*)script, result)); }
        bool evalWindow(const NPString *script, NPVariant *result) { NPObject* domWin = domWindow();
            return releaseWithResult(domWin, apiFn(api->evaluate, instance)(instance, domWin, (NPString*)script, result)); }
        bool evaluate(NPObject *npobj, const NPString *script, NPVariant *result) {
            if (npobj) return apiFn(api->evaluate, instance)(instance, npobj, (NPString*)script, result);
            else if ((intptr_t)npobj == 1) return evalElement(script, result);
            else return evalWindow(script, result); }
        bool getProperty(NPObject *npobj, NPIdentifier propertyName, NPVariant *result) {
            return apiFn(api->getproperty, instance)(instance, npobj, propertyName, result); }
        bool setProperty(NPObject *npobj, NPIdentifier propertyName, const NPVariant *value) {
            return apiFn(api->setproperty, instance)(instance, npobj, propertyName, value); }
        bool removeProperty(NPObject *npobj, NPIdentifier propertyName) {
            return apiFn(api->removeproperty, instance)(instance, npobj, propertyName); }
        bool hasProperty(NPObject *npobj, NPIdentifier propertyName) {
            return apiFn(api->hasproperty, instance)(instance, npobj, propertyName); }
        bool hasMethod(NPObject *npobj, NPIdentifier methodName) {
            return apiFn(api->hasproperty, instance)(instance, npobj, methodName); }
        bool enumerate(NPObject *npobj, NPIdentifier **identifier, uint32_t *count) {
            return apiFn(api->enumerate, instance)(instance, npobj, identifier, count); }
        bool construct(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return apiFn(api->construct, instance)(instance, npobj, args, argCount, result); }
        void setException(NPObject *npobj, const NPUTF8 *message) {
            return apiFn(api->setexception)(npobj, message); }

        NPVariant* releaseVariantValue(NPVariant *variant) {
            apiFn(api->releasevariantvalue)(variant); return NULL; }
        inline void release(NPVariant& variant) { releaseVariantValue(&variant); }
        inline NPVariant* release(NPVariant* variants, size_t count=1) {
            for (size_t i=0; i<count; i++) releaseVariantValue(&variants[i]);
            return NULL; }
        inline NPVariant* retain(NPVariant *variant) {
            if (variant && variant->type==NPVariantType_Object)
                retain(variant->value.objectValue);
            return variant; }
        inline void retain(NPVariant& variant) { retain(&variant); }
        NPIdentifier getStringIdentifier(const NPUTF8 *name) {
            return apiFn(api->getstringidentifier)(name); }
        void getStringIdentifiers(const NPUTF8 **names, int32_t nameCount, NPIdentifier *identifiers) {
            return apiFn(api->getstringidentifiers)(names, nameCount, identifiers); }
        NPIdentifier getIntIdentifier(int32_t intid) {
            return apiFn(api->getintidentifier)(intid); }
        bool identifierIsString(NPIdentifier identifier) {
            return apiFn(api->identifierisstring)(identifier); }
        NPUTF8* utf8FromIdentifier(NPIdentifier identifier) {
            return apiFn(api->utf8fromidentifier)(identifier); }
        int32_t intFromIdentifier(NPIdentifier identifier) {
            return apiFn(api->intfromidentifier)(identifier); }


        /* Networking, URL and stream support */
        NPError getURL(const char* url, const char* target) {
            return apiFn(api->geturl, instance)(instance, url, target); }
        NPError getURLNotify(const char* url, const char* target, void* notifyData) {
            return apiFn(api->geturlnotify, instance)(instance, url, target, notifyData); }
        NPError postURL(const char* url, const char* target, 
                uint32_t len, const char* buf, NPBool file) {
            return apiFn(api->posturl, instance)(instance, url, target, len, buf, file); }
        NPError postURLNotify(const char* url, const char* target, 
                uint32_t len, const char* buf, NPBool file, void* notifyData) {
            return apiFn(api->posturlnotify, instance)(instance, url, target, len, buf, file, notifyData); }

        void urlRedirectResponse(void* notifyData, NPBool allow) {
            return apiFn(api->urlredirectresponse, instance)(instance, notifyData, allow); }

        NPError requestRead(NPStream* stream, NPByteRange* rangeList) {
            return apiFn(api->requestread)(stream, rangeList); }
        NPError newStream(NPMIMEType type, const char* target, NPStream** stream) {
            return apiFn(api->newstream, instance)(instance, type, target, stream); }
        int32_t write(NPStream* stream, int32_t len, void* buffer) {
            return apiFn(api->write, instance)(instance, stream, len, buffer); }
        NPError destroyStream(NPStream* stream, NPReason reason) {
            return apiFn(api->destroystream, instance)(instance, stream, reason); }

        NPError getValueForURL(NPNURLVariable variable, const char *url, 
                char **value, uint32_t *len) {
            return apiFn(api->getvalueforurl, instance)(instance, variable, url, value, len); }
        NPError setValueForURL(NPNURLVariable variable, const char *url, 
                const char *value, uint32_t len) {
            return apiFn(api->setvalueforurl, instance)(instance, variable, url, value, len); }

        NPError getAuthenticationInfo(
                const char *protocol, const char *host, int32_t port,
                const char *scheme, const char *realm,
                char **username, uint32_t *ulen,
                char **password, uint32_t *plen) {
            return apiFn(api->getauthenticationinfo, instance)(instance, 
                    protocol, host, port, scheme, realm, 
                    username, ulen, password, plen); }


        /* Drawing, windowing, printing, and events */
        void invalidateRect(NPRect *invalidRect) {
            return apiFn(api->invalidaterect, instance)(instance, invalidRect); }
        void invalidateRegion(NPRegion invalidRegion) {
            return apiFn(api->invalidateregion, instance)(instance, invalidRegion); }
        void forceRedraw() { 
            return apiFn(api->forceredraw, instance)(instance); }
        void pushPopupsEnabledState(NPBool enabled) {
            return apiFn(api->pushpopupsenabledstate, instance)(instance, enabled); }
        void popPopupsEnabledState() {
            return apiFn(api->poppopupsenabledstate, instance)(instance); }

        NPError popUpContextMenu(NPMenu* menu) {
            return apiFn(api->popupcontextmenu, instance)(instance, menu); }
        NPBool convertPoint(double sourceX, double sourceY, NPCoordinateSpace sourceSpace, double *destX, double *destY, NPCoordinateSpace destSpace) {
            return apiFn(api->convertpoint, instance)(instance, sourceX, sourceY, sourceSpace, destX, destY, destSpace); }
        NPBool handleEvent(void *event, NPBool handled) {
            return apiFn(api->handleevent, instance)(instance, event, handled); }
        NPBool unfocusInstance(NPFocusDirection direction) {
            return apiFn(api->unfocusinstance, instance)(instance, direction); }
            
        /* Utilities and access methods */
        inline NPIdentifier ident(NPIdentifier name) { return name; }
        inline NPIdentifier ident(const char* utf8Name) { return getStringIdentifier(utf8Name); }
        inline NPIdentifier ident(uint32_t idxName) { return getIntIdentifier(idxName); }
        std::string identStr(NPIdentifier name, const char* absent="[null]") {
            NPUTF8* szName = utf8FromIdentifier(name);
            if (szName) {
                std::string res(szName);
                szName = memFreeEx(szName);
                return res; }
            std::stringstream oss;
            oss << intFromIdentifier(name);
            return oss.str(); }
        const char* variantStr(const NPVariant* r) {
            if (!r)                     return "<NPVariant INVALID>";
            switch (r->type) {
            case NPVariantType_Void:    return "<NPVariant void>";
            case NPVariantType_Null:    return "<NPVariant null>";
            case NPVariantType_Bool:    return "<NPVariant bool>";
            case NPVariantType_Int32:   return "<NPVariant int32>";
            case NPVariantType_Double:  return "<NPVariant double>";
            case NPVariantType_String:  return "<NPVariant string>";
            case NPVariantType_Object:  return "<NPVariant object>";
            default:                    return "<NPVariant ???>";
            }}

        NPString* setString(NPString* szTgt, const NPUTF8* str, size_t len=0, size_t maxlen=1024) {
            if (!szTgt) return NULL;
            if (len == 0) len = ::strnlen(str, maxlen);
            char* buf = (char*) memAlloc(len);
            ::strncpy(buf, str, len);
            szTgt->UTF8Characters = buf;
            szTgt->UTF8Length = len;
            return szTgt; }
        NPString setString(const NPUTF8* str, size_t len=0, size_t maxlen=1024) {
            NPString res = {0,0}; setString(&res, str, len, maxlen); return res; }
        const NPUTF8* release(const NPUTF8* str) { return str ? memFreeEx(str) : NULL; }
        NPString* release(NPString* str) {
            if (str) { str->UTF8Characters = memFreeEx(str->UTF8Characters); str->UTF8Length = 0; }
            return NULL; }

        NPVariant* setVariantVoid(NPVariant* r) {
            r->type = NPVariantType_Void; r->value.objectValue = NULL; return r; }
        void setVariantVoid(NPVariant* r, uint16_t count) {
            while (--count>=0) setVariantVoid(r++); }
        NPVariant* setVariantNull(NPVariant* r) {
            r->type = NPVariantType_Null; r->value.objectValue = NULL; return r; }
        void setVariantNull(NPVariant* r, uint16_t count) {
            while (--count>=0) setVariantNull(r++); }
        
        NPVariant* setVariant(NPVariant* r, bool v) {
            r->type = NPVariantType_Bool; r->value.boolValue = v; return r; }
        NPVariant* setVariant(NPVariant* r, uint32_t v) {
            r->type = NPVariantType_Int32; r->value.intValue = v; return r; }
        NPVariant* setVariant(NPVariant* r, int32_t v) {
            r->type = NPVariantType_Int32; r->value.intValue = v; return r; }
        NPVariant* setVariant(NPVariant* r, double v) {
            r->type = NPVariantType_Double; r->value.doubleValue = v; return r; }
        NPVariant* setVariant(NPVariant* r, NPObject* v) {
            r->type = NPVariantType_Object; r->value.objectValue = v; return r; }
        NPVariant* setVariant(NPVariant* r, NPString v) {
            r->type = NPVariantType_String; r->value.stringValue = v; return r; }
        NPVariant* setVariant(NPVariant* r, const NPUTF8* str, size_t len=0, size_t maxlen=1024) {
            r->type = NPVariantType_String;
            setString(&r->value.stringValue, str, len, maxlen);
            return r; }

        NPVariant _evaluate(NPObject *npobj, const NPUTF8* script, size_t len=0, size_t maxlen=16384) {
            NPVariant v_result = {NPVariantType_Void, NULL};
            NPString e_script = {0,0};
            if (setString(&e_script, script, len, maxlen)) {
                if (!evaluate(npobj, &e_script, &v_result))
                    releaseVariantValue(&v_result);
                release(&e_script);
            }
            return v_result; }
        inline NPVariant evalWindow(const NPUTF8* script, size_t len=0, size_t maxlen=16384) {
            return _evaluate(NULL, script, len, maxlen); }
        inline NPVariant evalElement(const NPUTF8* script, size_t len=0, size_t maxlen=16384) {
            return _evaluate((NPObject*)1, script, len, maxlen); }
    };
}
