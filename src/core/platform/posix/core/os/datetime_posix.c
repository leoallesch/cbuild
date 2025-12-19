#include "core/mem/memory_context.h"
#include "core/os/datetime.h"

#include <stdio.h>
#include <time.h>

instant_t time_now(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (instant_t)ts.tv_sec * TIME_SECOND + ts.tv_nsec;
}

instant_t time_monotonic(void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (instant_t)ts.tv_sec * TIME_SECOND + ts.tv_nsec;
}

instant_t time_from_unix(int64_t seconds)
{
  return seconds * TIME_SECOND;
}

int64_t instant_to_unix(instant_t t)
{
  return t / TIME_SECOND;
}

duration_t time_since(instant_t start)
{
  return time_now() - start;
}

duration_t time_since_mono(instant_t start)
{
  return time_monotonic() - start;
}

duration_t time_until(instant_t deadline)
{
  return deadline - time_now();
}

duration_t time_diff(instant_t a, instant_t b)
{
  return a - b;
}

double time_as_seconds(duration_t d)
{
  return (double)d / (double)TIME_SECOND;
}

int64_t time_as_millis(duration_t d)
{
  return d / TIME_MILLISECOND;
}

int64_t time_as_micros(duration_t d)
{
  return d / TIME_MICROSECOND;
}

dateinstant_t instant_to_local(instant_t t)
{
  time_t unix_secs = instant_to_unix(t);
  struct tm local;
  localtime_r(&unix_secs, &local);

  return (dateinstant_t){
    .year = local.tm_year + 1900,
    .month = local.tm_mon + 1,
    .day = local.tm_mday,
    .hour = local.tm_hour,
    .minute = local.tm_min,
    .second = local.tm_sec,
    .nanos = (int)(t % TIME_SECOND),
  };
}

dateinstant_t instant_to_utc(instant_t t)
{
  time_t unix_secs = instant_to_unix(t);
  struct tm utc;
  gmtime_r(&unix_secs, &utc);

  return (dateinstant_t){
    .year = utc.tm_year + 1900,
    .month = utc.tm_mon + 1,
    .day = utc.tm_mday,
    .hour = utc.tm_hour,
    .minute = utc.tm_min,
    .second = utc.tm_sec,
    .nanos = (int)(t % TIME_SECOND),
  };
}

string_t time_format(allocator_t* allocator, instant_t t)
{
  dateinstant_t dt = instant_to_local(t);
  int millis = dt.nanos / 1000000;

  // "2025-12-18 14:30:45.123" = 23 chars + null
  char* buf = allocator_alloc(allocator, 24, alignof(char));
  int len = snprintf(buf, 24, "%04d-%02d-%02d %02d:%02d:%02d.%03d", dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second, millis);

  return string_from_buffer(buf, len);
}

string_t time_format_custom(allocator_t* allocator, instant_t t, string_t format)
{
  dateinstant_t dt = instant_to_local(t);
  int millis = dt.nanos / 1000000;

  // Estimate output size (format length + some extra for expansions)
  size_t buf_size = format.length + 32;
  char* buf = allocator_alloc(allocator, buf_size, alignof(char));
  char* out = buf;
  char* end = buf + buf_size - 1;

  for(size_t i = 0; i < format.length && out < end; i++) {
    if(format.string[i] == '%' && i + 1 < format.length) {
      char spec = format.string[i + 1];
      int written = 0;

      switch(spec) {
        case 'Y':
          written = snprintf(out, end - out, "%04d", dt.year);
          break;
        case 'm':
          written = snprintf(out, end - out, "%02d", dt.month);
          break;
        case 'd':
          written = snprintf(out, end - out, "%02d", dt.day);
          break;
        case 'H':
          written = snprintf(out, end - out, "%02d", dt.hour);
          break;
        case 'M':
          written = snprintf(out, end - out, "%02d", dt.minute);
          break;
        case 'S':
          written = snprintf(out, end - out, "%02d", dt.second);
          break;
        case 'f':
          written = snprintf(out, end - out, "%03d", millis);
          break;
        case '%':
          *out = '%';
          written = 1;
          break;
        default:
          *out++ = '%';
          *out = spec;
          written = 1;
          break;
      }

      out += written;
      i++; // skip the specifier
    }
    else {
      *out++ = format.string[i];
    }
  }

  *out = '\0';
  return string_from_buffer(buf, out - buf);
}
