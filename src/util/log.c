#include "log.h"
#include "thread.h"
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdarg.h>

static FILE* stream;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int level = DEBUG;

#ifdef DEBUG_BUILD
static const char* severity_string(LogLevel severity)
{
    switch (severity) {
        case FATAL: return "\033[31mFATAL\033[0m";
        case CRITICAL: return "\033[38;2;255;165;0mCRITICAL\033[0m";
        case WARNING: return "\033[33mWARNING\033[0m";
        case INFO: return "\033[32mINFO\033[0m";
        case DEBUG: return "\033[34mDEBUG\033[0m";
        case MEMORY: return "\033[38;2;128;68;255mMEMORY\033[0m";
        default: break;
    }
    return "";
}
#else
static const char* severity_string(LogLevel severity)
{
    switch (severity) {
        case FATAL: return "FATAL";
        case CRITICAL: return "CRITICAL";
        case WARNING: return "WARNING";
        case INFO: return "INFO";
        case DEBUG: return "DEBUG";
        default: break;
    }
    return "";
}
#endif

static void get_timestamp(int* Y, int* M, int* D, int* h, int* m, int* s)
{
    time_t cur_time;
    struct tm local_time;
    cur_time = time(NULL);
    local_time = *localtime(&cur_time);
    *Y = local_time.tm_year + 1900;
    *M = local_time.tm_mon + 1;
    *D = local_time.tm_mday;
    *h = local_time.tm_hour;
    *m = local_time.tm_min;
    *s = local_time.tm_sec;
}

#ifdef RELEASE_BUILD
static FILE* create_log_file(const char* log_dir)
{
    return stderr;
    int n = strlen(log_dir);
    char log_path[n+50];
    char basename[50];
    int Y, M, D, h, m, s;
    get_timestamp(&Y, &M, &D, &h, &m, &s);
    snprintf(basename, sizeof(basename), "\\log_%4d-%02d-%02d_%02d-%02d-%02d", Y, M, D, h, m, s);
    strncpy(log_path, log_dir, n);
    strncpy(&log_path[n], basename, 50);
    FILE* file = fopen(log_path, "w");
    if (file == NULL) {
        log_write(INFO, "Failed to create log file, using stderr instead");
        return stderr;
    }
    return file;
}
#endif

void log_init(char* exe_path)
{
    char* dir = dirname(exe_path);
    int n = strlen(dir);
    char log_dir[n+10];
    strncpy(log_dir, dir, n+1);
    strncpy(&log_dir[n], "\\logs", 6);
    stream = stderr;
#ifdef RELEASE_BUILD
    if (mkdir(log_dir) != 0 && errno != EEXIST) {
        log_write(INFO, "Failed to create log directory, using stderr instead");
    } else {
        stream = create_log_file(log_dir);
        if (stream != stderr)
            log_write(INFO, "Created log file");
    }
#endif
}

#ifdef DEBUG_BUILD
void _log_write(LogLevel severity, const char* message, const char* filename, int line, ...)
{
    if ((int)severity > level)
        return;
    int Y, M, D, h, m, s;
    va_list args;
    pthread_mutex_lock(&mutex);
    get_timestamp(&Y, &M, &D, &h, &m, &s);
    fprintf(stderr, "\033[35;2;70;140;70m%4d-%02d-%02d %02d:%02d:%02d %s \033[36m%s \033[96m%s:%d\033[0m\n- ", 
            Y, M, D, h, m, s, 
            severity_string(severity), 
            thread_get_self_name(), filename, line);
    va_start(args, line);
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n\n");
    fflush(stderr);
    if (severity == FATAL)
        abort();
    pthread_mutex_unlock(&mutex);
}
#else
void _log_write(LogLevel severity, const char* message, ...)
{
    int Y, M, D, h, m, s;
    va_list args;
    pthread_mutex_lock(&mutex);
    get_timestamp(&Y, &M, &D, &h, &m, &s);
    fprintf(stream, "[%4d-%02d-%02d %02d:%02d:%02d][%s] ", 
            Y, M, D, h, m, s, 
            severity_string(severity));
    va_start(args, message);
    vfprintf(stream, message, args);
    va_end(args);
    fprintf(stream, "\n");
    fflush(stream);
    if (severity == FATAL)
        abort();
    pthread_mutex_unlock(&mutex);
}
#endif

void log_set_level(int _level)
{
    level = _level;
}

void log_unlock(void)
{
    mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
}

void log_cleanup(void)
{
    if (stream != stderr && fclose(stream) == EOF)
        fprintf(stderr, "Failed to close log file");
}
