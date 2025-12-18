extern "C" {
#include "arena_allocator.h"
#include "heap_mem.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(ArenaAllocator)
{
  arena_t arena;
  i_mem_t* mem;

  void setup()
  {
    mem = heap_mem_init();
    arena = arena_init(mem, 1024);
  }

  void teardown()
  {
    arena.interface.destroy(&arena.interface);
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
  void* p1 = arena.interface.alloc(&arena.interface, 16, 0);
  CHECK(p1 != NULL);
  CHECK(arena.offset >= 16u);

  void* p2 = arena.interface.alloc(&arena.interface, 8, 0);
  CHECK(p2 != NULL);
  CHECK(arena.offset >= 24u);
}

TEST(ArenaAllocator, AllocReturnsNullWhenOutOfSpace)
{
  void* p = arena.interface.alloc(&arena.interface, 1024, 0);
  CHECK(p != NULL);

  void* p2 = arena.interface.alloc(&arena.interface, 1, 0);
  CHECK(p2 == NULL);
}

TEST(ArenaAllocator, FreeAllClearsOffset)
{
  arena.interface.alloc(&arena.interface, 100, 0);
  CHECK(arena.offset >= 100u);

  arena.interface.free_all(&arena.interface);
  CHECK_EQUAL(0u, arena.offset);
}

//
// --- Alignment Tests ---
//

TEST(ArenaAllocator, AlignedAllocPointerIsAligned)
{
  size_t alignment = 16;
  void* p = arena.interface.alloc(&arena.interface, 32, alignment);

  CHECK(p != NULL);
  CHECK(((uintptr_t)p % alignment) == 0);
}

TEST(ArenaAllocator, AlignedAllocMultipleBlocksAreAligned)
{
  size_t alignment = 32;

  void* p1 = arena.interface.alloc(&arena.interface, 8, alignment);
  CHECK(p1 != NULL);
  CHECK(((uintptr_t)p1 % alignment) == 0);

  void* p2 = arena.interface.alloc(&arena.interface, 8, alignment);
  CHECK(p2 != NULL);
  CHECK(((uintptr_t)p2 % alignment) == 0);
}

TEST(ArenaAllocator, AlignedAllocReturnsNullIfNotEnoughSpace)
{
  arena.interface.alloc(&arena.interface, 1024, 0);

  void* p = arena.interface.alloc(&arena.interface, 200, 64);
  CHECK(p == NULL);
}

TEST(ArenaAllocator, AllocReturnsZeroedMemory)
{
  void* p = arena.interface.alloc(&arena.interface, 16, 0);
  CHECK(p != NULL);

  uint8_t* bytes = (uint8_t*)p;
  for(int i = 0; i < 16; i++) {
    CHECK_EQUAL(0u, bytes[i]);
  }
}
