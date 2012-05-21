/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab */
#if defined(__APPLE__)
namespace Nekoite {
    void log_v(const char* fmt, va_list args) {
        NSString* format = [NSString stringWithCString: fmt encoding: NSASCIIStringEncoding];
        NSLogv(format, args);
    }
    void log(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log_v(fmt, args);
        va_end(args);
    }
    bool waitForDebugger() { return false; }
}
#endif
