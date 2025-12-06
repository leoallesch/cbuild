extern "C" {
#include "arena_allocator.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(ArenaAllocator)
{
  arena_t arena;

  void setup()
  {
    arena = arena_init(1024);
  }

  void teardown()
  {
    arena_free(&arena);
  }
};

TEST(ArenaAllocator, InitWorks)
{
  CHECK(arena.buffer != NULL);
  CHECK_EQUAL(1024u, arena.capacity);
  CHECK_EQUAL(0u, arena.offset);
}

TEST(ArenaAllocator, BasicAllocIncreasesOffset)
{
  void* p1 = arena_alloc(&arena, 16);
  CHECK(p1 != NULL);
  CHECK_EQUAL(16u, arena.offset);

  void* p2 = arena_alloc(&arena, 8);
  CHECK(p2 != NULL);
  CHECK_EQUAL(24u, arena.offset);
}

TEST(ArenaAllocator, AllocReturnsNullWhenOutOfSpace)
{
  void* p = arena_alloc(&arena, 1024);
  CHECK(p != NULL);

  void* p2 = arena_alloc(&arena, 1);
  CHECK(p2 == NULL);
}

TEST(ArenaAllocator, ResetClearsOffset)
{
  arena_alloc(&arena, 100);
  CHECK_EQUAL(100u, arena.offset);

  arena_reset(&arena);
  CHECK_EQUAL(0u, arena.offset);
}

//
// --- Alignment Tests ---
//

TEST(ArenaAllocator, AlignedAllocPointerIsAligned)
{
  size_t alignment = alignof(uintptr_t);
  void* p = arena_alloc_aligned(&arena, 32, alignment);

  CHECK(p != NULL);
  CHECK(((uintptr_t)p % alignment) == 0);
}

TEST(ArenaAllocator, AlignedAllocMultipleBlocksAreAligned)
{
  size_t alignment = alignof(uintptr_t);

  void* p1 = arena_alloc_aligned(&arena, 8, alignment);
  CHECK(p1 != NULL);
  CHECK(((uintptr_t)p1 % alignment) == 0);

  void* p2 = arena_alloc_aligned(&arena, 8, 32);
  CHECK(p2 != NULL);
  CHECK(((uintptr_t)p2 % alignment) == 0);
}

TEST(ArenaAllocator, AlignedAllocReturnsNullIfNotEnoughSpace)
{
  arena_alloc(&arena, 1000);

  void* p = arena_alloc_aligned(&arena, 128, 64);
  CHECK(p == NULL);
}
