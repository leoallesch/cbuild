#include "CppUTest/UtestMacros.h"
extern "C" {
#include <string.h>
#include "arena_allocator.h"
#include "i_allocator.h"
}

#include "CppUTest/TestHarness.h"

#define BUFFER_SIZE 1024

TEST_GROUP(ArenaAllocator)
{
  arena_allocator_t arena;
  i_allocator_t* alloc = NULL;

  void setup()
  {
    arena = arena_init(BUFFER_SIZE);
    alloc = &arena.interface;
  }

  void teardown()
  {
    alloc_destroy(alloc);
  }
};

TEST(ArenaAllocator, AllocReturnsNonNullForValidSizes)
{
  void* p1 = alloc_alloc(alloc, 16);
  CHECK(p1 != nullptr);

  void* p2 = alloc_alloc(alloc, 128);
  CHECK(p2 != nullptr);
}

TEST(ArenaAllocator, AllocReturnsDistinctPointersOnMultipleCalls)
{
  void* a = alloc_alloc(alloc, 32);
  void* b = alloc_alloc(alloc, 32);

  CHECK(a != nullptr);
  CHECK(b != nullptr);
  CHECK(a != b);
}

TEST(ArenaAllocator, AllocZeroBytesIsAllowed)
{
  void* p = alloc_alloc(alloc, 0);
  CHECK(p != nullptr);
}

TEST(ArenaAllocator, ResetAllowsReusingMemory)
{
  void* p1 = alloc_alloc(alloc, 64);
  CHECK(p1 != nullptr);

  alloc_reset(alloc);

  void* p2 = alloc_alloc(alloc, 64);
  CHECK(p2 != nullptr);

  CHECK(p1 == p2);
}
