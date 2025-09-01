#ifndef ERROR_LOG_H
#define ERROR_LOG_H

#include <stdio.h>

typedef enum {
    FATAL = 0,
    CRITICAL = 1,
    WARNING = 2,
    INFO = 3,
    DEBUG = 4,
    MEMORY = 5
} LogLevel;

void log_init(char* exe_path);
void log_unlock(void);
void log_set_level(int level);
void log_cleanup(void);

#ifdef DEBUG_BUILD
    void _log_write(LogLevel severity, const char* message, const char* filename, int line, ...);
    #define log_write(severity, message, ...) \
        _log_write(severity, message, __FILE__, __LINE__, ##__VA_ARGS__)
    #define log_assert(condition, message, ...) \
        if (!(condition)) \
            _log_write(FATAL, message, __FILE__, __LINE__, ##__VA_ARGS__)
#else
    void _log_write(LogLevel severity, const char* message, ...);
    #define log_write(severity, message, ...) \
        _log_write(severity, message, ##__VA_ARGS__)
    #define log_assert(condition, message, ...) \
        if (!(condition)) \
            _log_write(FATAL, message, ##__VA_ARGS__)
#endif

#endif
