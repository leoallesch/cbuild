#include "logger.h"

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_HANDLERS 8
#define LOG_BUFFER_SIZE 4096

typedef struct {
  log_handler_t fn;
  void* user_data;
} handler_entry_t;

typedef struct {
  atomic_int level;
  handler_entry_t handlers[MAX_HANDLERS];
  atomic_int handler_count;
  pthread_mutex_t mutex;
  bool colors;
  bool timestamps;
  bool initialized;
} logger_state_t;

static logger_state_t g_logger;

static _Thread_local char tl_buffer[LOG_BUFFER_SIZE];

static const char* level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char* level_colors[] = {
  "\x1b[94m", // TRACE - blue
  "\x1b[36m", // DEBUG - cyan
  "\x1b[32m", // INFO  - green
  "\x1b[33m", // WARN  - yellow
  "\x1b[31m", // ERROR - red
  "\x1b[35m", // FATAL - magenta
};

static uint64_t get_thread_id(void)
{
  return (uint64_t)pthread_self();
}

static uint64_t get_timestamp_ms(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void default_handler(const log_event_t* event, void* user_data)
{
  (void)user_data;
  FILE* out = (event->level >= LOG_ERROR) ? stderr : stdout;

  char time_buf[32] = { 0 };
  if(g_logger.timestamps) {
    time_t t = event->timestamp / 1000;
    struct tm* tm_info = localtime(&t);
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);
  }

  const char* color = g_logger.colors ? level_colors[event->level] : "";
  const char* reset = g_logger.colors ? "\x1b[0m" : "";

  if(g_logger.timestamps) {
    fprintf(out,
      "%s ",
      time_buf);
  }

  fprintf(out,
    "%s%-5s%s ",
    color,
    level_strings[event->level],
    reset);

  if(strlen(event->tag)) {
    fprintf(out,
      "[%s] ",
      event->tag);
  }

  if(event->level <= LOG_DEBUG) {
    fprintf(out,
      "%s:%d: ",
      event->file,
      event->line);
  }

  fprintf(out,
    "%.*s\n",
    event->message_len,
    event->message);

  fflush(out);
}

void logger_init(void)
{
  if(g_logger.initialized)
    return;

  atomic_store(&g_logger.level, LOG_INFO);
  atomic_store(&g_logger.handler_count, 0);
  pthread_mutex_init(&g_logger.mutex, NULL);
  g_logger.colors = true;
  g_logger.timestamps = true;
  g_logger.initialized = true;

  logger_add_handler(default_handler, NULL);
}

void logger_shutdown(void)
{
  if(!g_logger.initialized)
    return;
  pthread_mutex_destroy(&g_logger.mutex);
  g_logger.initialized = false;
}

void logger_set_level(log_level_t level)
{
  atomic_store(&g_logger.level, level);
}

void logger_add_handler(log_handler_t handler, void* user_data)
{
  pthread_mutex_lock(&g_logger.mutex);
  int count = atomic_load(&g_logger.handler_count);
  if(count < MAX_HANDLERS) {
    g_logger.handlers[count].fn = handler;
    g_logger.handlers[count].user_data = user_data;
    atomic_store(&g_logger.handler_count, count + 1);
  }
  pthread_mutex_unlock(&g_logger.mutex);
}

void logger_enable_colors(bool enable)
{
  g_logger.colors = enable;
}

void logger_enable_timestamp(bool enable)
{
  g_logger.timestamps = enable;
}

void logger_log(log_level_t level, const char* tag, const char* file, int line, const char* fmt, ...)
{
  if((int)level < atomic_load(&g_logger.level))
    return;

  va_list args;
  va_start(args, fmt);
  int len = vsnprintf(tl_buffer, LOG_BUFFER_SIZE, fmt, args);
  va_end(args);

  if(len < 0)
    len = 0;
  if(len >= LOG_BUFFER_SIZE)
    len = LOG_BUFFER_SIZE - 1;

  log_event_t event = {
    .level = level,
    .tag = tag,
    .file = file,
    .line = line,
    .message = tl_buffer,
    .message_len = len,
    .timestamp = get_timestamp_ms(),
    .thread_id = get_thread_id(),
  };

  pthread_mutex_lock(&g_logger.mutex);
  int count = atomic_load(&g_logger.handler_count);
  for(int i = 0; i < count; i++) {
    g_logger.handlers[i].fn(&event, g_logger.handlers[i].user_data);
  }
  pthread_mutex_unlock(&g_logger.mutex);
}
