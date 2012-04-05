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

namespace NPObjFramework {

    struct NPException { 
        NPException(NPError err_p):err(err_p){};
        NPError err;
    };
    struct NPHostAPIError : NPException {
        NPHostAPIError() : NPException(NPERR_INVALID_FUNCTABLE_ERROR) {}
    };

    template<typename T> inline T* apiFn(T* fn) { if (!fn) throw NPHostAPIError(); return fn; }

    struct NPHostObj {
        NPP instance;
        NPNetscapeFuncs* api;

        NPHostObj(NPP instance_p, NPNetscapeFuncs* api_p) 
            : instance(instance_p), api(api_p) {}

        /* General API */
        void status(const char* message) { return apiFn(api->status)(instance, message); }
        const char* userAgent() { return apiFn(api->uagent)(instance); }

        void* memAlloc(uint32_t size) { return apiFn(api->memalloc)(size); }
        void memFree(void* ptr) { apiFn(api->memfree)(ptr); }
        uint32_t memFlush(uint32_t size) { return apiFn(api->memflush)(size); }

        void reloadPlugins(NPBool reloadPages) { return apiFn(api->reloadplugins)(reloadPages); }

        NPError getValue(NPNVariable variable, void *value) {
            return apiFn(api->getvalue)(instance, variable, value); }
        NPError setValue(NPPVariable variable, void *value) {
            return apiFn(api->setvalue)(instance, variable, value); }

        uint32_t scheduleTimer(uint32_t interval, NPBool repeat, void (*timerFunc)(NPP npp, uint32_t timerID)) {
            return apiFn(api->scheduletimer)(instance, interval, repeat, timerFunc); }
        void unscheduleTimer(uint32_t timerID) {
            return apiFn(api->unscheduletimer)(instance, timerID); }

        void pluginThreadAsyncCall(void (*func) (void *), void *userData) {
            return apiFn(api->pluginthreadasynccall)(instance, func, userData); }


        /* Scripting Support */
        NPObject* createObject(NPClass *aClass) {
            return apiFn(api->createobject)(instance, aClass); }
        NPObject* retainObject(NPObject *npobj) {
            return apiFn(api->retainobject)(npobj); }
        void releaseObject(NPObject *npobj) {
            apiFn(api->releaseobject)(npobj); }
        bool invoke(NPObject *npobj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return apiFn(api->invoke)(instance, npobj, methodName, args, argCount, result); }
        bool invokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return apiFn(api->invokeDefault)(instance, npobj, args, argCount, result); }
        bool evaluate(NPObject *npobj, NPString *script, NPVariant *result) {
            return apiFn(api->evaluate)(instance, npobj, script, result); }
        bool getProperty(NPObject *npobj, NPIdentifier propertyName, NPVariant *result) {
            return apiFn(api->getproperty)(instance, npobj, propertyName, result); }
        bool setProperty(NPObject *npobj, NPIdentifier propertyName, const NPVariant *value) {
            return apiFn(api->setproperty)(instance, npobj, propertyName, value); }
        bool removeProperty(NPObject *npobj, NPIdentifier propertyName) {
            return apiFn(api->removeproperty)(instance, npobj, propertyName); }
        bool hasProperty(NPObject *npobj, NPIdentifier propertyName) {
            return apiFn(api->hasproperty)(instance, npobj, propertyName); }
        bool hasMethod(NPObject *npobj, NPIdentifier methodName) {
            return apiFn(api->hasproperty)(instance, npobj, methodName); }
        bool enumerate(NPObject *npobj, NPIdentifier **identifier, uint32_t *count) {
            return apiFn(api->enumerate)(instance, npobj, identifier, count); }
        bool construct(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result) {
            return apiFn(api->construct)(instance, npobj, args, argCount, result); }
        void setException(NPObject *npobj, const NPUTF8 *message) {
            return apiFn(api->setexception)(npobj, message); }

        void releaseVariantValue(NPVariant *variant) {
            return apiFn(api->releasevariantvalue)(variant); }
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
            return apiFn(api->geturl)(instance, url, target); }
        NPError getURLNotify(const char* url, const char* target, void* notifyData) {
            return apiFn(api->geturlnotify)(instance, url, target, notifyData); }
        NPError postURL(const char* url, const char* target, 
                uint32_t len, const char* buf, NPBool file) {
            return apiFn(api->posturl)(instance, url, target, len, buf, file); }
        NPError postURLNotify(const char* url, const char* target, 
                uint32_t len, const char* buf, NPBool file, void* notifyData) {
            return apiFn(api->posturlnotify)(instance, url, target, len, buf, file, notifyData); }

        void urlRedirectResponse(void* notifyData, NPBool allow) {
            return apiFn(api->urlredirectresponse)(instance, notifyData, allow); }

        NPError requestRead(NPStream* stream, NPByteRange* rangeList) {
            return apiFn(api->requestread)(stream, rangeList); }
        NPError newStream(NPMIMEType type, const char* target, NPStream** stream) {
            return apiFn(api->newstream)(instance, type, target, stream); }
        int32_t write(NPStream* stream, int32_t len, void* buffer) {
            return apiFn(api->write)(instance, stream, len, buffer); }
        NPError destroyStream(NPStream* stream, NPReason reason) {
            return apiFn(api->destroystream)(instance, stream, reason); }

        NPError getValueForURL(NPNURLVariable variable, const char *url, 
                char **value, uint32_t *len) {
            return apiFn(api->getvalueforurl)(instance, variable, url, value, len); }
        NPError setValueForURL(NPNURLVariable variable, const char *url, 
                const char *value, uint32_t len) {
            return apiFn(api->setvalueforurl)(instance, variable, url, value, len); }

        NPError getAuthenticationInfo(
                const char *protocol, const char *host, int32_t port,
                const char *scheme, const char *realm,
                char **username, uint32_t *ulen,
                char **password, uint32_t *plen) {
            return apiFn(api->getauthenticationinfo)(instance, 
                    protocol, host, port, scheme, realm, 
                    username, ulen, password, plen); }


        /* Drawing, windowing, printing, and events */
        void invalidateRect(NPRect *invalidRect) {
            return apiFn(api->invalidaterect)(instance, invalidRect); }
        void invalidateRegion(NPRegion invalidRegion) {
            return apiFn(api->invalidateregion)(instance, invalidRegion); }
        void forceRedraw() { 
            return apiFn(api->forceredraw)(instance); }
        void pushPopupsEnabledState(NPBool enabled) {
            return apiFn(api->pushpopupsenabledstate)(instance, enabled); }
        void popPopupsEnabledState() {
            return apiFn(api->poppopupsenabledstate)(instance); }

        NPError popUpContextMenu(NPMenu* menu) {
            return apiFn(api->popupcontextmenu)(instance, menu); }
        NPBool convertPoint(double sourceX, double sourceY, NPCoordinateSpace sourceSpace, double *destX, double *destY, NPCoordinateSpace destSpace) {
            return apiFn(api->convertpoint)(instance, sourceX, sourceY, sourceSpace, destX, destY, destSpace); }
        NPBool handleEvent(void *event, NPBool handled) {
            return apiFn(api->handleevent)(instance, event, handled); }
        NPBool unfocusInstance(NPFocusDirection direction) {
            return apiFn(api->unfocusinstance)(instance, direction); }
    };
}
