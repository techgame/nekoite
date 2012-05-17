/* -*- coding: utf-8 -*- vim: set ts=4 sw=4 expandtab */
#if defined(__APPLE__)

void npObjFramework_log_v(const char* fmt, va_list args) {
    NSString* format = [NSString stringWithCString: fmt encoding: NSASCIIStringEncoding];
    NSLogv(format, args);
}
void npObjFramework_log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    npObjFramework_log_v(fmt, args);
    va_end(args);
}
bool npObjFramework_waitForDebugger() { return false; }
#endif
