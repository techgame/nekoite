/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab */
#pragma once
#include "nekoite.h"

namespace Nekoite {
    /*~ Runtime Plugin Obj Definition ~~~~~~~~~~~~~~~~~*/

    template<typename T>
    struct NPRuntimePluginObj : NPPluginObjBase {
        typedef NPRuntimePluginObj<T> base_t;

        NPRuntimePluginObj(NPP inst) 
          : NPPluginObjBase(inst), _rootObj(NULL) {}
        virtual ~NPRuntimePluginObj() {
            if (_rootObj) _rootObj = host->release(_rootObj);}

        virtual NPError initStart(NPMIMEType pluginType, uint16_t mode) {
            host->setValue(NPPVpluginWindowBool, (void *)false);
            host->setValue(NPPVpluginTransparentBool, (void *)true);
            return NPERR_NO_ERROR; }
        virtual NPError getValue(NPPVariable key, void *value) {
            switch (key) {
            case NPPVpluginScriptableNPObject: {
                return setvoid(value, host->retain(rootScriptObj())); }
            default: return NPERR_GENERIC_ERROR; } }

        virtual NPObject* rootScriptObj() {
            return _rootObj ? _rootObj : _rootObj = T::klass.create(*this); }
        NPObject* _rootObj;
    };

    /*~ NPObject virtualized skeleton ~~~~~~~~~~~~~~~~~~~~*/

    struct NPScriptObj : NPObject {
        NPHostObj* host;

        NPScriptObj(NPP npp, NPClass *aClass) : host(asNPHostObj(npp)) {
            _class = aClass; referenceCount = 1;
            host->trackObj(this);
        }
        virtual ~NPScriptObj() {
            host = host->untrackObj(this);
            _class = NULL; referenceCount = 0; };
        virtual const char* className() { return "NPScriptObj"; }
        bool isNPObjectValid(bool bSkipValid=false) {
            return _class && referenceCount>0 && (bSkipValid || isValid()); }
        virtual bool isValid() { return true; }

        virtual void onPluginDestroyed() { invalidate(); }
        virtual void invalidate() { clearProperties(); }
        virtual void deallocate() { }

        virtual bool hasMethod(NPIdentifier name) { return false; }
        virtual bool invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) { return false; }
        virtual bool invokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result) { return false; }
        virtual bool construct(const NPVariant *args, uint32_t argCount, NPVariant *result) { return false; }

        virtual bool hasProperty(NPIdentifier name) { return false; }
        virtual bool getProperty(NPIdentifier name, NPVariant *result) { return false; }
        virtual bool setProperty(NPIdentifier name, const NPVariant *value) { return false; }
        virtual bool removeProperty(NPIdentifier name) { return false; }
        virtual void clearProperties() { }

        virtual bool enumerate(NPIdentifier **value, uint32_t *count) { return false; }
        
        /* NPHost interface utilities */
        inline void retain(NPVariant& v) { host->retain(v); }
        inline NPObject* retain() { return host->retain(this); }
        template <typename T> inline T retain(T v) { return host->retain(v); }

        inline void release(NPVariant& v) { host->release(v); }
        template <typename T> inline T release(T v) { return host->release(v); }
        template <typename T> inline T release(T v, size_t count) { return host->release(v, count); }

        template <typename T> inline NPIdentifier ident(T name) { return host->ident(name); }
        inline std::string identStr(NPIdentifier name) { return host->identStr(name); }
        inline NPVariant* setVariantVoid(NPVariant* r) { return host->setVariantVoid(r); }
        inline void setVariantVoid(NPVariant* r, uint16_t count) { host->setVariantVoid(r, count); }
        inline NPVariant* setVariantNull(NPVariant* r) { return host->setVariantNull(r); }
        inline void setVariantNull(NPVariant* r, uint16_t count) { host->setVariantNull(r, count); }
        template <typename V> inline NPVariant* setVariant(NPVariant* r, V v) { return host->setVariant(r, v); }
        inline const char* variantStr(const NPVariant* r) { return host->variantStr(r); }

        virtual void onTimer(uint32_t timerID) {};
        uint32_t scheduleTimer(uint32_t interval, bool repeat=true) {
            return host->scheduleTimer(this, interval, repeat); }
        void unscheduleTimer() { host->unscheduleTimer(this); }
    };

    inline NPScriptObj* asNPScriptObj(NPObject* npobj, bool bSkipValid=false) {
        NPScriptObj* obj = static_cast<NPScriptObj*>(npobj);
        return obj && obj->isNPObjectValid(bSkipValid) ? obj : NULL;
    }


    /*~ Composed ScriptObj derivatives ~~~~~~~~~~~~~~~~~~~*/

    template <typename T>
    struct NPScriptObjStd : NPScriptObj {
        typedef NPScriptObjStd<T> base_t;
        typedef bool (T::*ScriptMethod)(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
        typedef struct {bool isProperty; const char* utf8Name; ScriptMethod fnMethod; } MethodTableEntry;
        typedef std::map<NPIdentifier, ScriptMethod> MethodMap;
        typedef std::map<NPIdentifier, NPVariant> PropertyMap;

        MethodMap methods;
        PropertyMap properties;
        MethodMap propertyMethods;

        NPScriptObjStd(NPP npp, NPClass *aClass) : NPScriptObj(npp, aClass) {}
        virtual ~NPScriptObjStd() {}

        virtual T* self() = 0;
        virtual const char* className() { return T::s_className(); }

        /*~ Method Registration ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        virtual bool registerMethod(NPIdentifier name, ScriptMethod fnMethod, bool isProperty=false) {
            if (isProperty)
                propertyMethods[name] = fnMethod;
            else methods[name] = fnMethod;
            return !!fnMethod; }
        template <typename N>
        inline bool registerMethod(N name, ScriptMethod fnMethod, bool isProperty=false) {
            return registerMethod(ident(name), fnMethod, isProperty); }

        bool registerMethodTable(MethodTableEntry* methodTable) {
            if (!methodTable) return false;
            for(; !!methodTable->utf8Name; methodTable++)
                registerMethod(methodTable->utf8Name, methodTable->fnMethod, methodTable->isProperty);
            return true; }


        /*~ Methods ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        virtual bool hasMethod(NPIdentifier name) {
            return methods.count(name) > 0; }
        virtual bool invokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return invoke(ident("__call__"), args, argCount, result); }
        virtual bool invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            if (methods.count(name)) {
                ScriptMethod fn = methods[name];
                if (fn) return (self()->*fn)(name, args, argCount, result);
            }
            return invokeDNU(name, args, argCount, result); }
        virtual bool invokeDNU(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return false; }


        /*~ Property Has/Get/Set/Remove ~~~~~~~~~~~~~~~~~~~~~~*/
        virtual bool hasProperty(NPIdentifier name) {
            if (properties.count(name) > 0) return true;
            if (propertyMethods.count(name) > 0) return true;
            return false; }
        virtual bool getProperty(NPIdentifier name, NPVariant *result) {
            if (properties.count(name)>0) {
                retain(*result = properties[name]);
                return true;
            } else if (propertyMethods.count(name)>0) {
                ScriptMethod fn = propertyMethods[name];
                return fn ? (self()->*fn)(name, NULL, 0, result) : false;
            } else return false; }
        virtual bool setProperty(NPIdentifier name, const NPVariant *value) {
            if (propertyMethods.count(name)>0) {
                NPVariant res = {NPVariantType_Void, NULL};
                ScriptMethod fn = propertyMethods[name];
                return fn ? (self()->*fn)(name, value, 1, &res) : false;
            } else if (properties.count(name))
                release(&properties[name]);
            retain(properties[name] = *value);
            return true; }
        virtual bool removeProperty(NPIdentifier name) {
            if (!properties.count(name)) return false;
            release(&properties[name]);
            properties.erase(name);
            return true; }

        template <typename M>
        void _releaseEach(M iter, M end) {
            for (; iter!=end; ++iter) release(iter->second); }
        virtual void clearProperties() {
            _releaseEach(properties.begin(), properties.end());
            properties.clear(); }


        /*~ Methods and Properties Enumeration ~~~~~~~~~~~~~~~*/
        template <typename M>
        NPIdentifier* _enumMap(NPIdentifier *tip, M iter, M end) {
            for (; iter!=end; ++iter) *(tip++) = iter->first;
            return tip; }
        virtual bool enumerate(NPIdentifier **value, uint32_t *count) {
            uint32_t nElems = properties.size() + propertyMethods.size() + methods.size();
            NPIdentifier *tip = (NPIdentifier*) host->memAlloc(nElems*sizeof(NPIdentifier));
            *value = tip; *count = nElems;
            tip = _enumMap(tip, properties.begin(), properties.end());
            tip = _enumMap(tip, propertyMethods.begin(), propertyMethods.end());
            tip = _enumMap(tip, methods.begin(), methods.end());
            return true;
        }

        /*~ Adaptors for NPIdentifier and NPVariants ~~~~~~~~~*/
        template <typename N>
        inline bool hasProperty(N name) {
            return hasProperty(ident(name)); }
        template <typename N>
        inline bool getProperty(N name, NPVariant *result) {
            return getProperty(ident(name), result); }
        template <typename N, typename V>
        inline bool setProperty(N name, V value) {
            NPVariant vv; setVariant(&vv, value);
            return setProperty(ident(name), (const NPVariant*)&vv); }            
        template <typename N>
        inline bool removeProperty(N name) {
            return removeProperty(ident(name)); }
    };


    /*~ NPClass providers for NPScriptObj objects ~~~~~~~*/

    template <typename T>
    struct NPScriptClass : NPClass {
        NPScriptClass() {
            structVersion = NP_CLASS_STRUCT_VERSION_CTOR;
            allocate = _allocate;
            deallocate = _deallocate;
            invalidate = _invalidate;
            hasMethod = _hasMethod;
            invoke = _invoke;
            invokeDefault = _invokeDefault;
            hasProperty = _hasProperty;
            getProperty = _getProperty;
            setProperty = _setProperty;
            removeProperty = _removeProperty;
            enumerate = _enumerate;
            construct = _construct;
        }

        T* create(NPP npp) {
            return static_cast<T*>(asNPHostObj(npp)->createObject(this)); }
        static const char* className() { return T::s_className(); }

    protected:
        static NPObject* _allocate(NPP npp, NPClass *aClass) {
            T* obj = new T(npp, aClass);
            if (!obj->isNPObjectValid()) {
                _deallocate(obj);
                delete obj; obj=NULL;}
            return obj; }
        static void _deallocate(NPObject *npobj) {
            NPScriptObj* obj = asNPScriptObj(npobj, true);
            if (!obj) return;
            obj->deallocate();
            delete obj; obj = NULL;}
        static void _invalidate(NPObject *npobj) {
            NPScriptObj* obj = asNPScriptObj(npobj, true);
            if (obj) obj->invalidate(); }
        static bool _hasMethod(NPObject *npobj, NPIdentifier name) {
            NPScriptObj* obj = asNPScriptObj(npobj);
            return obj ? obj->hasMethod(name) : false; }
        static bool _invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            NPScriptObj* obj = asNPScriptObj(npobj);
            return obj ? obj->invoke(name, args, argCount, result) : false; }
        static bool _invokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            NPScriptObj* obj = asNPScriptObj(npobj);
            return obj ? obj->invokeDefault(args, argCount, result) : false; }
        static bool _construct(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            NPScriptObj* obj = asNPScriptObj(npobj);
            return obj ? obj->construct(args, argCount, result) : false; }
        static bool _hasProperty(NPObject *npobj, NPIdentifier name) {
            NPScriptObj* obj = asNPScriptObj(npobj);
            return obj ? obj->hasProperty(name) : false; }
        static bool _getProperty(NPObject *npobj, NPIdentifier name, NPVariant *result) {
            NPScriptObj* obj = asNPScriptObj(npobj);
            return obj ? obj->getProperty(name, result) : false; }
        static bool _setProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value) {
            NPScriptObj* obj = asNPScriptObj(npobj);
            return obj ? obj->setProperty(name, value) : false; }
        static bool _removeProperty(NPObject *npobj, NPIdentifier name) {
            NPScriptObj* obj = asNPScriptObj(npobj);
            return obj ? obj->removeProperty(name) : false; }
        static bool _enumerate(NPObject *npobj, NPIdentifier **value, uint32_t *count) {
            NPScriptObj* obj = asNPScriptObj(npobj);
            return obj ? obj->enumerate(value, count) : false; }
    };
}
