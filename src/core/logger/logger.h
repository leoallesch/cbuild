#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
  LOG_TRACE,
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL,
} log_level_t;

typedef struct {
  log_level_t level;
  const char* tag;
  const char* file;
  int line;
  const char* message;
  int message_len;
  uint64_t timestamp;
  uint64_t thread_id;
} log_event_t;

typedef void (*log_handler_t)(const log_event_t* event, void* user_data);

void logger_init(void);
void logger_shutdown(void);

void logger_set_level(log_level_t level);
void logger_add_handler(log_handler_t handler, void* user_data);
void logger_enable_colors(bool enable);
void logger_enable_timestamp(bool enable);

void logger_log(log_level_t level, const char* tag, const char* file, int line, const char* fmt, ...);

#define log_trace_tag(tag, ...) logger_log(LOG_TRACE, tag, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug_tag(tag, ...) logger_log(LOG_DEBUG, tag, __FILE__, __LINE__, __VA_ARGS__)
#define log_info_tag(tag, ...)  logger_log(LOG_INFO,  tag, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn_tag(tag, ...)  logger_log(LOG_WARN,  tag, __FILE__, __LINE__, __VA_ARGS__)
#define log_error_tag(tag, ...) logger_log(LOG_ERROR, tag, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal_tag(tag, ...) logger_log(LOG_FATAL, tag, __FILE__, __LINE__, __VA_ARGS__)

#define log_trace(...) logger_log(LOG_TRACE, "", __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) logger_log(LOG_DEBUG, "", __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  logger_log(LOG_INFO,  "", __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  logger_log(LOG_WARN,  "", __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) logger_log(LOG_ERROR, "", __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) logger_log(LOG_FATAL, "", __FILE__, __LINE__, __VA_ARGS__)
