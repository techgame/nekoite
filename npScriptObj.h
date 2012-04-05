/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
 *~ Copyright (C) 2002-2012  TechGame Networks, LLC.              ~*
 *~                                                               ~*
 *~ This library is free software; you can redistribute it        ~*
 *~ and/or modify it under the terms of the MIT style License as  ~*
 *~ found in the LICENSE file included with this distribution.    ~*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma once
#include <map> /* used to implement NPScriptObjStd */

#include "npPluginObj.h"
#include "npruntime.h"

namespace NPObjFramework {
    /*~ Scriptable Plugin Obj Definition ~~~~~~~~~~~~~~~~~*/

    struct NPScriptablePluginObj : NPPluginObjBase {
        NPScriptablePluginObj(NPP inst, NPNetscapeFuncs* hostApi) 
            : NPPluginObjBase(inst, hostApi) {}

        virtual NPError initStart(NPMIMEType pluginType, uint16_t mode) {
            NPBool bFalse = false;
            host->setValue(NPPVpluginWindowBool, (void *)bFalse);

            return NPERR_NO_ERROR; 
        }

        virtual NPError getValue(NPPVariable variable, void *value) {
            switch (variable) {
            case NPPVpluginNameString:
                return setvoid(value, pluginName());
            case NPPVpluginDescriptionString:
                return setvoid(value, pluginDescription());
            case NPPVpluginScriptableNPObject:
                return setvoid(value, rootScriptObj());
            default: return NPERR_GENERIC_ERROR;
            }
        }

        virtual const char* pluginName() = 0;
        virtual const char* pluginDescription() = 0;
        virtual NPObject* rootScriptObj() = 0;
    };

    /*~ NPObject virtualized skeleton ~~~~~~~~~~~~~~~~~~~~*/

    struct NPScriptObj : NPObject {
        NPScriptObj(NPP npp, NPClass *aClass) { _class = aClass; referenceCount = 1; }

        virtual void deallocate() {}
        virtual void invalidate() {}

        virtual bool hasMethod(NPIdentifier name) { return false; }
        virtual bool invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) { return false; }
        virtual bool invokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result) { return false; }
        virtual bool construct(const NPVariant *args, uint32_t argCount, NPVariant *result) { return false; }

        virtual bool hasProperty(NPIdentifier name) { return false; }
        virtual bool getProperty(NPIdentifier name, NPVariant *result) { return false; }
        virtual bool setProperty(NPIdentifier name, const NPVariant *value) { return false; }
        virtual bool removeProperty(NPIdentifier name) { return false; }

        virtual bool enumerate(NPIdentifier **value, uint32_t *count) { return false; }
    };

    inline NPScriptObj* asNSScriptable(NPObject* npobj) {
        return static_cast<NPScriptObj*>(npobj);
    }

    /*~ NPClass providers for NPScriptObj objects ~~~~~~~*/

    struct NPScriptClassBase : NPClass {
        NPScriptClassBase(NPAllocateFunctionPtr _allocate) {
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

    protected:
        static void _deallocate(NPObject *npobj) {
            if (npobj) asNSScriptable(npobj)->deallocate(); }
        static void _invalidate(NPObject *npobj) {
            if (npobj) asNSScriptable(npobj)->invalidate(); }

        static bool _hasMethod(NPObject *npobj, NPIdentifier name) {
            return !npobj ? false : asNSScriptable(npobj)->hasMethod(name); }
        static bool _invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return !npobj ? false : asNSScriptable(npobj)->invoke(name, args, argCount, result); }
        static bool _invokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return !npobj ? false : asNSScriptable(npobj)->invokeDefault(args, argCount, result); }
        static bool _construct(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return !npobj ? false : asNSScriptable(npobj)->construct(args, argCount, result); }

        static bool _hasProperty(NPObject *npobj, NPIdentifier name) {
            return !npobj ? false : asNSScriptable(npobj)->hasProperty(name); }
        static bool _getProperty(NPObject *npobj, NPIdentifier name, NPVariant *result) {
            return !npobj ? false : asNSScriptable(npobj)->getProperty(name, result); }
        static bool _setProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value) {
            return !npobj ? false : asNSScriptable(npobj)->setProperty(name, value); }
        static bool _removeProperty(NPObject *npobj, NPIdentifier name) {
            return !npobj ? false : asNSScriptable(npobj)->removeProperty(name); }

        static bool _enumerate(NPObject *npobj, NPIdentifier **value, uint32_t *count) {
            return !npobj ? false : asNSScriptable(npobj)->enumerate(value, count); }
    };

    template <typename T>
    struct  NPScriptClass : NPScriptClassBase {
        NPScriptClass() : NPScriptClassBase(_allocate) {}
        
        inline T* create(NPP npp) { return createInstance(npp, this); }
    protected:
        inline static T* createInstance(NPP npp, NPClass *aClass) { return new T(npp, aClass); }
        static NPObject* _allocate(NPP npp, NPClass *aClass) { return createInstance(npp, aClass); }
    };


    /*~ Composed ScriptObj derivatives ~~~~~~~~~~~~~~~~~~~*/

    struct NPScriptObjEx : NPScriptObj {
        NPHostObj* host;

        NPScriptObjEx(NPP npp, NPClass *aClass)
          : NPScriptObj(npp, aClass), host(asNSPluginObj(npp)->hostObj())
        {}

        virtual NPObject* domWindow() {
            NPObject* res = NULL;
            return host->getValue(NPNVWindowNPObject, &res) ? NULL : res;
        }
        virtual NPObject* domElement() {
            NPObject* res = NULL;
            return host->getValue(NPNVPluginElementNPObject, &res) ? NULL : res;
        }
    };

    template <typename T>
    struct NPScriptObjStd : NPScriptObjEx {
        NPScriptObjStd(NPP npp, NPClass *aClass)
            : NPScriptObjEx(npp, aClass) {}

        virtual T* self() = 0;

        /*~ Methods ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        typedef bool (T::*ScriptMethod)(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
        typedef struct {const char* utf8Name; ScriptMethod fnMethod; } MethodTableEntry;
        typedef std::map<NPIdentifier, ScriptMethod> MethodMap;
        typedef typename MethodMap::iterator MethodMapIter;
        MethodMap methods;

        virtual bool hasMethod(NPIdentifier name) {
            return methods.count(name) > 0;
        }
        virtual bool invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            if (methods.count(name)) {
                ScriptMethod fn = methods[name];
                if (fn) return (self()->*fn)(name, args, argCount, result);
            }
            return invokeDNU(name, args, argCount, result);
        }
        virtual bool invokeDNU(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return false;
        }

        bool registerMethodTable(MethodTableEntry* methodTable) {
            if (!methodTable) return false;
            for(; !!methodTable->utf8Name; methodTable++)
                registerMethod(methodTable->utf8Name, methodTable->fnMethod);
            return true;
        }
        inline bool registerMethod(const char* utf8Name, ScriptMethod fnMethod) {
            return !utf8Name ? false : registerMethod(host->getStringIdentifier(utf8Name), fnMethod); }
        inline bool registerMethod(int32_t idxName, ScriptMethod fnMethod) {
            return registerMethod(host->getIntIdentifier(idxName), fnMethod); }
        virtual bool registerMethod(NPIdentifier name, ScriptMethod fnMethod) {
            methods[name] = fnMethod;
            return !!fnMethod; }


        /*~ Properties ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        typedef std::map<NPIdentifier, NPVariant> PropertyMap;
        typedef typename PropertyMap::iterator PropertyMapIter;
        PropertyMap properties;
        virtual bool hasProperty(NPIdentifier name) {
            return properties.count(name) > 0;
        }
        virtual bool getProperty(NPIdentifier name, NPVariant *result) {
            if (!properties.count(name)) return false;
            *result = properties[name];
            return true;
        }
        virtual bool setProperty(NPIdentifier name, const NPVariant *value) {
            if (properties.count(name))
                host->releaseVariantValue(&properties[name]);
            properties[name] = *value; 
            return true;
        }
        virtual NPVariant* setPropertyByRef(NPIdentifier name) {
            if (properties.count(name))
                host->releaseVariantValue(&properties[name]);
            return &properties[name];
        }
        virtual bool removeProperty(NPIdentifier name) {
            if (!properties.count(name)) return false;
            host->releaseVariantValue(&properties[name]);
            properties.erase(name);
            return true;
        }

        inline bool hasProperty(const char* utf8Name) {
            return hasProperty(host->getStringIdentifier(utf8Name)); }
        inline bool hasProperty(uint32_t idxName) {
            return hasProperty(host->getIntIdentifier(idxName)); }
        inline bool getProperty(const char* utf8Name, NPVariant *result) {
            return getProperty(host->getStringIdentifier(utf8Name), result); }
        inline bool getProperty(uint32_t idxName, NPVariant *result) {
            return getProperty(host->getIntIdentifier(idxName), result); }
        inline bool setProperty(const char* utf8Name, const NPVariant *value) {
            return setProperty(host->getStringIdentifier(utf8Name), value); }
        inline bool setProperty(uint32_t idxName, const NPVariant *value) {
            return setProperty(host->getIntIdentifier(idxName), value); }
        inline bool removeProperty(const char* utf8Name) {
            return removeProperty(host->getStringIdentifier(utf8Name)); }
        inline bool removeProperty(uint32_t idxName) {
            return removeProperty(host->getIntIdentifier(idxName)); }

        inline NPVariant* setPropertyByRef(const char* utf8Name) {
            return setPropertyByRef(host->getStringIdentifier(utf8Name)); }
        inline NPVariant* setPropertyByRef(uint32_t idxName) {
            return setPropertyByRef(host->getIntIdentifier(idxName)); }

        template <typename V>
        inline bool setProperty(const char* utf8Name, V value) {
            return setVariant(setPropertyByRef(utf8Name), value); }
        template <typename V>
        inline bool setProperty(uint32_t idxName, V value) {
            return setVariant(setPropertyByRef(idxName), value); }

        /*~ Methods and Properties Enumeration ~~~~~~~~~~~~~~~*/

        virtual bool enumerate(NPIdentifier **value, uint32_t *count) {
            NPIdentifier *tip, *buf;
            uint32_t nElems = methods.size() + properties.size();
            tip = buf = (NPIdentifier*) host->memAlloc(nElems*sizeof(NPIdentifier));

            PropertyMapIter itProp=properties.begin(); 
            for (; itProp!=properties.end(); ++itProp) {
                *(tip++) = itProp->first; }

            MethodMapIter itMeth=methods.begin();
            for (; itMeth!=methods.end(); ++itMeth) {
                *(tip++) = itMeth->first; }

            *value = buf; *count = nElems;
            return true;
        }
    };

    /* 
    struct ExampleObj : NPScriptObjStd<ExampleObj> {
        ExampleObj(NPP npp, NPClass *aClass)
            : NPScriptObjStd(npp, aClass)
        {
            registerMethod("example", &ExampleObj::example);
        }
        virtual ExampleObj* self() { return this; }

        virtual bool example(NPIdentifier, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return true;
        }
    };
    NPScriptClass<ExampleObj> exampleClass;
    */
}
