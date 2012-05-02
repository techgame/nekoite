/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
 *~ Copyright (C) 2002-2012  TechGame Networks, LLC.              ~*
 *~                                                               ~*
 *~ This library is free software; you can redistribute it        ~*
 *~ and/or modify it under the terms of the MIT style License as  ~*
 *~ found in the LICENSE file included with this distribution.    ~*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma once
#include "npObjFramework.h"

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
        virtual ~NPScriptObj() {};
        virtual bool isValid() { return true; }

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
        NPScriptObj* res = static_cast<NPScriptObj*>(npobj);
        return res && res->isValid() ? res : NULL;
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
            NPScriptObj* obj = asNSScriptable(npobj);
            if (obj) obj->deallocate(); }
        static void _invalidate(NPObject *npobj) {
            NPScriptObj* obj = asNSScriptable(npobj);
            if (obj) asNSScriptable(npobj)->invalidate(); }

        static bool _hasMethod(NPObject *npobj, NPIdentifier name) {
            NPScriptObj* obj = asNSScriptable(npobj);
            return !obj ? false : obj->hasMethod(name); }
        static bool _invoke(NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            NPScriptObj* obj = asNSScriptable(npobj);
            return !obj ? false : obj->invoke(name, args, argCount, result); }
        static bool _invokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            NPScriptObj* obj = asNSScriptable(npobj);
            return !obj ? false : obj->invokeDefault(args, argCount, result); }
        static bool _construct(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            NPScriptObj* obj = asNSScriptable(npobj);
            return !obj ? false : obj->construct(args, argCount, result); }

        static bool _hasProperty(NPObject *npobj, NPIdentifier name) {
            NPScriptObj* obj = asNSScriptable(npobj);
            return !obj ? false : obj->hasProperty(name); }
        static bool _getProperty(NPObject *npobj, NPIdentifier name, NPVariant *result) {
            NPScriptObj* obj = asNSScriptable(npobj);
            return !obj ? false : obj->getProperty(name, result); }
        static bool _setProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value) {
            NPScriptObj* obj = asNSScriptable(npobj);
            return !obj ? false : obj->setProperty(name, value); }
        static bool _removeProperty(NPObject *npobj, NPIdentifier name) {
            NPScriptObj* obj = asNSScriptable(npobj);
            return !obj ? false : obj->removeProperty(name); }

        static bool _enumerate(NPObject *npobj, NPIdentifier **value, uint32_t *count) {
            NPScriptObj* obj = asNSScriptable(npobj);
            return !obj ? false : obj->enumerate(value, count); }
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

        inline NPIdentifier ident(NPIdentifier name) { return name; }
        inline NPIdentifier ident(const char* utf8Name) { return host->getStringIdentifier(utf8Name); }
        inline NPIdentifier ident(uint32_t idxName) { return host->getIntIdentifier(idxName); }
    };

    template <typename T>
    struct NPScriptObjStd : NPScriptObjEx {
        typedef bool (T::*ScriptMethod)(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
        typedef struct {bool isProperty; const char* utf8Name; ScriptMethod fnMethod; } MethodTableEntry;
        typedef std::map<NPIdentifier, ScriptMethod> MethodMap;
        typedef typename MethodMap::iterator MethodMapIter;
        typedef std::map<NPIdentifier, NPVariant> PropertyMap;
        typedef typename PropertyMap::iterator PropertyMapIter;

        MethodMap methods;
        PropertyMap properties;
        MethodMap propertyMethods;

        NPScriptObjStd(NPP npp, NPClass *aClass) : NPScriptObjEx(npp, aClass) { }
        virtual ~NPScriptObjStd() {}

        virtual T* self() = 0;

        /*~ Methods ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

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
                registerMethod(methodTable->utf8Name, methodTable->fnMethod, methodTable->isProperty);
            return true;
        }
        template <typename N>
        inline bool registerMethod(N name, ScriptMethod fnMethod, bool isProperty=false) {
            return registerMethod(ident(name), fnMethod, isProperty); }
        virtual bool registerMethod(NPIdentifier name, ScriptMethod fnMethod, bool isProperty=false) {
            if (isProperty)
                propertyMethods[name] = fnMethod;
            else methods[name] = fnMethod;
            return !!fnMethod;
        }


        /*~ hasProperty ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        virtual bool hasProperty(NPIdentifier name) {
            if (properties.count(name) > 0) return true;
            if (propertyMethods.count(name) > 0) return true;
            return false;
        }
        template <typename N>
        inline bool hasProperty(N name) {
            return hasProperty(ident(name)); }


        /*~ getProperty ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        virtual NPVariant* getPropertyRef(NPIdentifier name) {
            return properties.count(name) ? &properties[name] : NULL;
        }
        template <typename N>
        inline NPVariant* getPropertyRef(N name) {
            return getPropertyRef(ident(name)); }
        virtual bool getProperty(NPIdentifier name, NPVariant *result) {
            if (properties.count(name)>0) {
                *result = properties[name];
                return true;
            } else if (propertyMethods.count(name)>0) {
                ScriptMethod fn = propertyMethods[name];
                return fn ? (self()->*fn)(name, NULL, 0, result) : false;
            } else return false;
        }
        template <typename N>
        inline bool getProperty(N name, NPVariant *result) {
            return getProperty(ident(name), result); }

        template <typename V, typename N>
        inline V* getProperty(N name, V** out) {
            return getVariant(getPropertyRef(ident(name)), out); }


        /*~ setProperty ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        virtual NPVariant* setPropertyRef(NPIdentifier name) {
            if (properties.count(name))
                host->releaseVariantValue(&properties[name]);
            return &properties[name];
        }
        template <typename N>
        inline NPVariant* setPropertyRef(N name) {
            return setPropertyRef(ident(name)); }
        virtual bool setProperty(NPIdentifier name, const NPVariant *value) {
            if (propertyMethods.count(name)>0) {
                ScriptMethod fn = propertyMethods[name];
                return fn ? (self()->*fn)(name, value, 1, NULL) : false;
            } else if (properties.count(name))
                host->releaseVariantValue(&properties[name]);
            properties[name] = *value; 
            return true;
        }
        template <typename N>
        inline bool setProperty(N name, const NPVariant *value) {
            return setProperty(ident(name), value); }

        template <typename V, typename N>
        inline bool setProperty(N name, V value) {
            return setVariant(setPropertyRef(ident(name)), value); }


        /*~ removeProperty ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        virtual bool removeProperty(NPIdentifier name) {
            if (!properties.count(name)) return false;
            host->releaseVariantValue(&properties[name]);
            properties.erase(name);
            return true;
        }
        template <typename N>
        inline bool removeProperty(N name) {
            return removeProperty(ident(name)); }


        /*~ Methods and Properties Enumeration ~~~~~~~~~~~~~~~*/
        virtual bool enumerate(NPIdentifier **value, uint32_t *count) {
            NPIdentifier *tip, *buf;
            uint32_t nElems = properties.size()
                + propertyMethods.size() + methods.size();
            tip = buf = (NPIdentifier*) host->memAlloc(nElems*sizeof(NPIdentifier));

            PropertyMapIter itProp=properties.begin(); 
            for (; itProp!=properties.end(); ++itProp) {
                *(tip++) = itProp->first; }

            MethodMapIter
                itMeth=propertyMethods.begin(),
                itMethEnd=propertyMethods.end();
            for (; itMeth!=itMethEnd; ++itMeth) {
                *(tip++) = itMeth->first; }

            itMeth=methods.begin();
            itMethEnd=methods.end();
            for (; itMeth!=itMethEnd; ++itMeth) {
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
