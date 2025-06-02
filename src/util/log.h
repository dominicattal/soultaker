#ifndef ERROR_LOG_H
#define ERROR_LOG_H

#include <stdio.h>

typedef enum {
    SEVERITY_EMERGENCY,
    SEVERITY_ALERT,
    SEVERITY_CRITICAL,
    SEVERITY_ERROR,
    SEVERITY_WARNING,
    SEVERITY_NOTICE,
    SEVERITY_INFO,
    SEVERITY_DEBUG
} LogSeverityLevel;

void log_init(char* exe_path);
void log_unlock(void);
void log_cleanup(void);

#ifdef DEBUG_BUILD
    void _log_write(LogSeverityLevel severity, const char* message, const char* filename, int line, ...);
    #define log_write(severity, message, ...) \
        _log_write(severity, message, __FILE__, __LINE__, ##__VA_ARGS__)
#else
    void _log_write(LogSeverityLevel severity, const char* message, ...);
    #define log_write(severity, message, ...) \
        _log_write(severity, message, ##__VA_ARGS__)
#endif

#endif
