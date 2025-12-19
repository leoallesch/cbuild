#pragma once

// ============================================================================
// Internal Utility Macros
// ============================================================================
//
// These are implementation details not exposed in the public API.
//

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)

#define OFFSET_OF(type, member) ((size_t)(&((type*)0)->member))
#define CONTAINER_OF(ptr, type, member) ((type*)((char*)(ptr) - OFFSET_OF(type, member)))

#define NUM_ELEMENTS(arr) (sizeof(arr) / sizeof(arr[0]))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))