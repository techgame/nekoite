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
#endif