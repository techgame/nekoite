/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab */
#pragma once

#include <string>
#include <sstream>
#include <map>

#if defined(_WIN32)
    #ifndef STRICT
    #define STRICT
    #endif

    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501
    #endif

    #include <windows.h>

#endif

#include "npapi/npapi.h"
#include "npapi/npfunctions.h"
#include "npapi/npruntime.h"

namespace NPObjFramework {
    struct NPPlugin;
    struct NPPluginObj;

    void log(const char* fmt, ...);
    void log_v(const char* fmt, va_list args);
    bool waitForDebugger();
}

NPObjFramework::NPPlugin* rootNPPlugin();

#include "npHostObj.h"
#include "npTimers.h"
#include "npPluginObj.h"
#include "npScriptObj.h"

