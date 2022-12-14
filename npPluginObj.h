/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab */
#pragma once
#include "nekoite.h"

namespace Nekoite {
    struct NPPluginObj {
        virtual ~NPPluginObj() {}
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

        virtual NPHostObj* hostObj() const = 0;
        inline operator NPP() const { return hostObj()->instance; }
    };

    inline NPPluginObj* asNPPluginObj(NPP instance) {
        return static_cast<NPPluginObj*>(instance?instance->pdata:NULL); }
    inline NPHostObj* asNPHostObj(NPP instance) {
        NPPluginObj* plugin = asNPPluginObj(instance);
        return plugin ? plugin->hostObj() : NULL; }


    struct NPPluginObjBase : NPPluginObj {
        NPPluginObjBase(NPP inst) : host(new NPHostObj(inst)) {}
        virtual ~NPPluginObjBase() { }

        NPHostObj* host;
        virtual NPHostObj* hostObj() const { return host; }

        virtual NPError initialize(NPMIMEType pluginType, uint16_t mode, 
            int16_t argc, char* argn[], char* argv[], NPSavedData* saved) 
        { // composed method: initMime followed by initArg() for each arg
            NPError err = initStart(pluginType, mode);
            if (!err && saved) err = initSaved(saved);
            if (!err) err = initArgList(argc, argn, argv);
            if (!err) err = initFinish(pluginType, mode);
            return err;
        }
        virtual NPError initStart(NPMIMEType pluginType, uint16_t mode) { return NPERR_NO_ERROR; }
        virtual NPError initSaved(NPSavedData* saved) { return NPERR_NO_ERROR; }
        virtual NPError initArgList(int16_t argc, char* argn[], char* argv[]) { return NPERR_NO_ERROR; }
        virtual NPError initFinish(NPMIMEType pluginType, uint16_t mode) { return NPERR_NO_ERROR; }

        virtual NPError destroy(NPSavedData** save) {
            host->onPluginDestroyed(); 
            return NPERR_NO_ERROR; }
    };


    /*~ Utility methods ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

    template<typename T>
    inline NPError setvoid(void* dst, T* src) { *((T**)dst)=src; return NPERR_NO_ERROR; }

    inline bool* getVariant(NPVariant* r, bool** out) {
        bool* res = NULL;
        if (r && r->type == NPVariantType_Bool)
            res = &r->value.boolValue;
        return out ? *out = res : res; }
    inline int32_t* getVariant(NPVariant* r, int32_t** out) {
        int32_t* res = NULL;
        if (r && r->type == NPVariantType_Int32)
            res = &r->value.intValue;
        return out ? *out = res : res; }
    inline double* getVariant(NPVariant* r, double** out) {
        double* res = NULL;
        if (r && r->type == NPVariantType_Double)
            res = &r->value.doubleValue;
        return out ? *out = res : res; }
    inline NPObject* getVariant(NPVariant* r, NPObject** out) {
        NPObject* res = NULL;
        if (r && r->type == NPVariantType_Object)
            res = r->value.objectValue;
        return out ? *out = res : res; }
    inline NPString* getVariant(NPVariant* r, NPString** out) {
        NPString* res = NULL;
        if (r && r->type == NPVariantType_String)
            res = &r->value.stringValue;
        return out ? *out = res : res; }
    template<typename T>
    inline T getVariant(NPVariant& r, T* out) { return getVariant(&r, out); }

    inline const NPVariant* asVariantArgAt(uint32_t argIdx, const NPVariant* args, uint32_t argCount, NPVariantType typeKey) {
        return (argIdx<argCount && (typeKey == args[argIdx].type)) ? &args[argIdx] : NULL; }

    inline bool variantArg(const bool** out, uint32_t argIdx, const NPVariant *args, uint32_t argCount) {
        args = asVariantArgAt(argIdx, args, argCount, NPVariantType_Bool);
        if (out) *out = (args ? &args->value.boolValue : NULL);
        return !!args; }
    inline bool variantArg(const int32_t** out, uint32_t argIdx, const NPVariant *args, uint32_t argCount) {
        args = asVariantArgAt(argIdx, args, argCount, NPVariantType_Int32);
        if (out) *out = (args ? &args->value.intValue : NULL);
        return !!args; }
    inline bool variantArg(const double** out, uint32_t argIdx, const NPVariant *args, uint32_t argCount) {
        args = asVariantArgAt(argIdx, args, argCount, NPVariantType_Double);
        if (out) *out = (args ? &args->value.doubleValue : NULL);
        return !!args; }
    inline bool variantArg(NPObject* const** out, uint32_t argIdx, const NPVariant *args, uint32_t argCount) {
        args = asVariantArgAt(argIdx, args, argCount, NPVariantType_Object);
        if (out) *out = (args ? &args->value.objectValue : NULL);
        return !!args; }
    inline bool variantArg(const NPString** out, uint32_t argIdx, const NPVariant *args, uint32_t argCount) {
        args = asVariantArgAt(argIdx, args, argCount, NPVariantType_String);
        if (out) *out = (args ? &args->value.stringValue : NULL);
        return !!args; }
}
