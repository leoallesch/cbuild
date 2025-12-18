#pragma once

#include <stdalign.h>

#define ALIGN_UP(size, alignment) (((size) + (alignment)-1) & ~((alignment)-1))

#define KB(x) ((x)*1024ULL)
#define MB(x) ((x)*1024ULL * 1024ULL)
#define GB(x) ((x)*1024ULL * 1024ULL * 1024ULL)

#define DEFAULT_ALIGNMENT (alignof(max_align_t))
#define DEFAULT_PAGE_SIZE (KB(4))
