#include "log.h"
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdarg.h>

static FILE* stream;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static const char* severity_string(LogSeverityLevel severity)
{
    switch (severity) {
        case SEVERITY_EMERGENCY: return "EMERGENCY";
        case SEVERITY_ALERT: return "ALERT";
        case SEVERITY_CRITICAL: return "CRITICAL";
        case SEVERITY_ERROR: return "ERROR";
        case SEVERITY_WARNING: return "WARNING";
        case SEVERITY_NOTICE: return "NOTICE";
        case SEVERITY_INFO: return "INFO";
        case SEVERITY_DEBUG: return "DEBUG";
        default: break;
    }
    return "";
}

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
        log_write(SEVERITY_NOTICE, "Failed to create log file, using stderr instead");
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
        log_write(SEVERITY_NOTICE, "Failed to create log directory, using stderr instead");
    } else {
        stream = create_log_file(log_dir);
        if (stream != stderr)
            log_write(SEVERITY_NOTICE, "Created log file");
    }
#endif
}

#ifdef DEBUG_BUILD
void _log_write(LogSeverityLevel severity, const char* message, const char* filename, int line, ...)
{
    int Y, M, D, h, m, s;
    va_list args;
    pthread_mutex_lock(&mutex);
    get_timestamp(&Y, &M, &D, &h, &m, &s);
    fprintf(stderr, "[%4d-%02d-%02d %02d:%02d:%02d] [%s] [%s:%d] ", 
            Y, M, D, h, m, s, 
            severity_string(severity), 
            filename, line);
    va_start(args, line);
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
    if (severity == SEVERITY_EMERGENCY)
        exit(1);
    pthread_mutex_unlock(&mutex);
}
#else
void _log_write(LogSeverityLevel severity, const char* message, ...)
{
    int Y, M, D, h, m, s;
    va_list args;
    pthread_mutex_lock(&mutex);
    get_timestamp(&Y, &M, &D, &h, &m, &s);
    fprintf(stream, "[%4d-%02d-%02d %02d:%02d:%02d] [%s] ", 
            Y, M, D, h, m, s, 
            severity_string(severity));
    va_start(args, message);
    vfprintf(stream, message, args);
    va_end(args);
    fprintf(stream, "\n");
    fflush(stream);
    pthread_mutex_unlock(&mutex);
}
#endif

void log_unlock(void)
{
    mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
}

void log_cleanup(void)
{
    if (stream != stderr && fclose(stream) == EOF)
        fprintf(stderr, "Failed to close log file");
}
