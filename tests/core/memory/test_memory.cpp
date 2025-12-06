extern "C" {
#include "memory.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(MemoryContext)
{
  memory_context_t mem;

  void setup()
  {
    mem = memory_context_create(1024, 512); // 1 KB global, 512 B scratch
  }

  void teardown()
  {
    memory_context_destroy(&mem);
  }
};

// ----------------- Global allocation -----------------
TEST(MemoryContext, GlobalAlloc)
{
  void* a = arena_alloc(&mem.global, 100);
  void* b = arena_alloc(&mem.global, 200);

  // Allocations should not overlap
  CHECK(a != b);
  // Offset should have advanced correctly
  LONGS_EQUAL(300, mem.global.offset);
}

// ----------------- Scratch allocation -----------------
TEST(MemoryContext, ScratchAlloc)
{
  scratch_t s = scratch_begin(&mem);
  void* t1 = arena_alloc(s.arena, 128);
  void* t2 = arena_alloc(s.arena, 256);

  CHECK(t1 != t2);
  LONGS_EQUAL(384, s.arena->offset);

  scratch_end(&s);
  // Offset should reset
  LONGS_EQUAL(0, s.arena->offset);
}

// ----------------- Nested scratch scopes -----------------
TEST(MemoryContext, NestedScratchScopes)
{
  scratch_t s0 = scratch_begin(&mem);
  void* buf0 = arena_alloc(s0.arena, 100);

  // Nested scratch scope
  scratch_t s1 = scratch_begin(&mem);
  void* buf1 = arena_alloc(s1.arena, 200);

  // s1 should use the other scratch arena
  CHECK(s0.arena != s1.arena);

  // Allocations should not overlap
  CHECK(buf0 != buf1);

  scratch_end(&s1);
  // s0 arena offset unchanged
  LONGS_EQUAL(100, s0.arena->offset);

  scratch_end(&s0);
  LONGS_EQUAL(0, s0.arena->offset);
}

// ----------------- Scratch scope macro -----------------
TEST(MemoryContext, ScratchScopeMacro)
{
  scratch_scope(&mem, s0)
  {
    char* tmp = (char*)arena_alloc(s0.arena, 128);
    (void)tmp;
    LONGS_EQUAL(128, s0.arena->offset);

    scratch_scope(&mem, s1)
    {
      char* nested = (char*)arena_alloc(s1.arena, 256);
      (void)nested;
      LONGS_EQUAL(256, s1.arena->offset);
    }

    // After inner scope, outer arena offset unchanged
    LONGS_EQUAL(128, s0.arena->offset);
  }

  // After outer scope, arena reset
  LONGS_EQUAL(0, mem.scratch_pool[0].offset);
  LONGS_EQUAL(0, mem.scratch_pool[1].offset);
}

// ----------------- Scratch rotation -----------------
TEST(MemoryContext, ScratchRotation)
{
  scratch_t s0 = scratch_begin(&mem);
  scratch_t s1 = scratch_begin(&mem);

  // Two consecutive scratch_begin calls should rotate arenas
  CHECK(s0.arena != s1.arena);

  scratch_end(&s1);
  scratch_end(&s0);
}
