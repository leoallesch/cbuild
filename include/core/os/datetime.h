#pragma once

#include <stddef.h>
#include <stdint.h>

#include "core/alloc/allocator.h"
#include "core/string/string.h"

// Core types: nanoseconds since Unix epoch
typedef int64_t instant_t; // absolute point in time
typedef int64_t duration_t; // time difference

// Duration constants (in nanoseconds)
#define TIME_NANOSECOND 1LL
#define TIME_MICROSECOND 1000LL
#define TIME_MILLISECOND 1000000LL
#define TIME_SECOND 1000000000LL
#define TIME_MINUTE (60LL * TIME_SECOND)
#define TIME_HOUR (60LL * TIME_MINUTE)
#define TIME_DAY (24LL * TIME_HOUR)

// Get current wall clock time
instant_t time_now(void);

// Get monotonic time (for measuring durations, unaffected by clock changes)
instant_t time_monotonic(void);

// Convert to/from Unix epoch seconds
instant_t time_from_unix(int64_t seconds);
int64_t instant_to_unix(instant_t t);

// Duration helpers
duration_t time_since(instant_t start); // time_now() - start
duration_t time_since_mono(instant_t start); // time_monotonic() - start
duration_t time_until(instant_t deadline); // deadline - time_now()
duration_t time_diff(instant_t a, instant_t b); // a - b

// Conversions from duration
double time_as_seconds(duration_t d);
int64_t time_as_millis(duration_t d);
int64_t time_as_micros(duration_t d);

// Broken-down time for display
typedef struct {
  int year; // e.g., 2025
  int month; // 1-12
  int day; // 1-31
  int hour; // 0-23
  int minute; // 0-59
  int second; // 0-59
  int nanos; // 0-999999999
} dateinstant_t;

dateinstant_t instant_to_local(instant_t t);
dateinstant_t instant_to_utc(instant_t t);

// Formatting - returns string_t, caller provides allocator
// Format: "2025-12-18 14:30:45.123"
string_t time_format(allocator_t* allocator, instant_t t);

// Format with custom pattern (subset of strftime)
// %Y=year, %m=month, %d=day, %H=hour, %M=minute, %S=second, %f=millis
string_t time_format_custom(allocator_t* allocator, instant_t t, string_t format);
