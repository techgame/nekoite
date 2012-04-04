/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*
 *~ Copyright (C) 2002-2012  TechGame Networks, LLC.              ~*
 *~                                                               ~*
 *~ This library is free software; you can redistribute it        ~*
 *~ and/or modify it under the terms of the MIT style License as  ~*
 *~ found in the LICENSE file included with this distribution.    ~*
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#pragma once

namespace NPObjFramework {

    struct NPException { 
        NPError err;
        NPException(err):_err(err){};
    };
    struct NPHostAPIError : NPException {
        NPHostAPIError() : NPException(NPERR_INVALID_FUNCTABLE_ERROR) {}
    };

    template<T> inline T* apiFn(T* fn) { if (!fn) throw NPHostAPIError(); return fn; }

    class NPHostObj {
    public:
        NPNetscapeFuncs* api;
        NPP instance;

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

        NPError requestRead(NPStream* stream, NPByteRange* rangeList) {
            return apiFn(api->requestread)(stream, rangeList); }
        NPError newStream(NPMIMEType type, const char* target, NPStream** stream) {
            return apiFn(api->newstream)(instance, type, target, stream); }
        int32_t write(NPStream* stream, int32_t len, void* buffer) {
            return apiFn(api->write)(instance, stream, len, buffer); }
        NPError destroyStream(NPStream* stream, NPReason reason) {
            return apiFn(api->destroystream)(instance, reason); }

        void status(const char* message) { return apiFn(api->status)(instance, message); }
        const char* userAgent() { return apiFn(api->useragent)(instance); }

        void* memAlloc(uint32_t size) { return apiFn(api->memalloc)(size); }
        void memFree(void* ptr) { apiFn(api->memfree)(ptr); }
        uint32_t memFlush(uint32_t size) { return apiFn(api->memflush)(size); }

        void reloadPlugins(NPBool reloadPages) { return apiFn(api->reloadplugins)(reloadPages); }

        NPError getValue(NPNVariable variable, void *value) {
            return apiFn(api->getvalue)(instance, variable, value); }
        NPError setValue(NPPVariable variable, void *value) {
            return apiFn(api->setvalue)(instance, variable, value); }

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
        void pluginThreadAsyncCall(void (*func) (void *), void *userData) {
            return apiFn(api->pluginthreadasynccall)(instance, func, userData); }

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
            return apiFn(api->getauthenticationinfo)(instance, protocol, host, port
                    scheme, realm, username, ulen, password, plen); }

        uint32_t scheduleTimer(uint32_t interval, NPBool repeat, void (*timerFunc)(NPP npp, uint32_t timerID)) {
            return apiFn(api->scheduletimer)(instance, interval, repeat, timerFunc); }
        void unscheduleTimer(uint32_t timerID) {
            return apiFn(api->unscheduletimer)(instance, timerID); }

        NPError popUpContextMenu(NPMenu* menu) {
            return apiFn(api->popupcontextmenu)(instance, menu); }
        NPBool convertPoint(double sourceX, double sourceY, NPCoordinateSpace sourceSpace, double *destX, double *destY, NPCoordinateSpace destSpace) {
            return apiFn(api->convertpoint)(instance, sourceX, sourceY, sourceSpace, destX, destY, destSpace); }
        NPBool handleEvent(void *event, NPBool handled) {
            return apiFn(api->handleevent)(instance, event, handled); }
        NPBool unfocusInstance(NPFocusDirection direction) {
            return apiFn(api->unfocusinstance)(instance, direction); }

        void urlRedirectResponse(void* notifyData, NPBool allow) {
            return apiFn(api->urlredirectresponse)(instance, notifyData, allow); }
    };
}
