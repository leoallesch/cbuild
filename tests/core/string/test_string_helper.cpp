#include "CppUTest/TestHarness.h"

extern "C" {
#include "string_helper.h" // your string API
}

TEST_GROUP(StringHelper)
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

TEST(StringHelper, CreateStringFromCStr)
{
  string_t s = string_from_cstr(&arena, "hello");

  CHECK_EQUAL(5, s.length);
  STRNCMP_EQUAL("hello", s.string, 5);
}

TEST(StringHelper, CreateStringFromEmptyCstr)
{
  string_t s = string_from_cstr(&arena, "");

  CHECK_EQUAL(0, s.length);
}

TEST(StringHelper, StringCopyCopiesBuffer)
{
  string_t a = string_from_cstr(&arena, "abc");
  string_t b = string_copy(&arena, a);

  CHECK_EQUAL(3, b.length);
  STRNCMP_EQUAL("abc", b.string, 3);
  CHECK(a.string != b.string); // ensure deep copy
}

TEST(StringHelper, StringToCstrProducesNullTerminatedData)
{
  string_t a = string_from_cstr(&arena, "world");
  const char* c = string_to_cstr(&arena, a);

  STRCMP_EQUAL("world", c);
}

TEST(StringHelper, ConcatTwoStrings)
{
  string_t a = string_from_cstr(&arena, "foo");
  string_t b = string_from_cstr(&arena, "bar");

  string_t c = string_concat(&arena, a, b);

  CHECK_EQUAL(6, c.length);
  STRNCMP_EQUAL("foobar", c.string, 6);
}

TEST(StringHelper, ConcatWithEmpty)
{
  string_t a = string_from_cstr(&arena, "");
  string_t b = string_from_cstr(&arena, "bar");

  string_t c = string_concat(&arena, a, b);
  STRNCMP_EQUAL("bar", c.string, 3);
}

TEST(StringHelper, StringSliceMiddle)
{
  string_t s = string_from_cstr(&arena, "abcdef");

  string_t slice = string_slice(&arena, s, 1, 4);

  CHECK_EQUAL(3, slice.length);
  STRNCMP_EQUAL("bcd", slice.string, 3);
}

TEST(StringHelper, StringSliceStartZero)
{
  string_t s = string_from_cstr(&arena, "abcdef");

  string_t slice = string_slice(&arena, s, 0, 3);

  CHECK_EQUAL(3, slice.length);
  STRNCMP_EQUAL("abc", slice.string, 3);
}

TEST(StringHelper, StringSliceEndOfString)
{
  string_t s = string_from_cstr(&arena, "abcdef");

  string_t slice = string_slice(&arena, s, 3, 6);

  CHECK_EQUAL(3, slice.length);
  STRNCMP_EQUAL("def", slice.string, 3);
}

TEST(StringHelper, StringEquals)
{
  string_t a = string_from_cstr(&arena, "test");
  string_t b = string_from_cstr(&arena, "test");

  CHECK(string_equals(a, b));
}

TEST(StringHelper, StringEqualsDifferentLength)
{
  string_t a = string_from_cstr(&arena, "test");
  string_t b = string_from_cstr(&arena, "testing");

  CHECK_FALSE(string_equals(a, b));
}

TEST(StringHelper, StringCompareLess)
{
  string_t a = string_from_cstr(&arena, "abc");
  string_t b = string_from_cstr(&arena, "bcd");

  CHECK(string_compare(a, b) < 0);
}

TEST(StringHelper, StringCompareGreater)
{
  string_t a = string_from_cstr(&arena, "zzz");
  string_t b = string_from_cstr(&arena, "aaa");

  CHECK(string_compare(a, b) > 0);
}

TEST(StringHelper, StringCompareEqual)
{
  string_t a = string_from_cstr(&arena, "same");
  string_t b = string_from_cstr(&arena, "same");

  CHECK(string_compare(a, b) == 0);
}

TEST(StringHelper, BuilderInitializes)
{
  string_builder_t sb = string_builder_init(&arena, 8);

  CHECK(sb.buffer != NULL);
  CHECK_EQUAL(0, sb.length);
  CHECK_EQUAL(8, sb.capacity);
}

TEST(StringHelper, AppendCStr)
{
  string_builder_t sb = string_builder_init(&arena, 8);
  string_builder_append(&sb, "hello");

  CHECK_EQUAL(5, sb.length);
  STRNCMP_EQUAL("hello", sb.buffer, 5);
}

TEST(StringHelper, AppendString)
{
  string_builder_t sb = string_builder_init(&arena, 8);

  string_t a = string(&arena, "foo");
  string_t b = string(&arena, "bar");
  string_t c = string(&arena, "lo");
  string_builder_append_string(&sb, a);
  string_builder_append_string(&sb, b);
  string_builder_append_string(&sb, c);

  CHECK_EQUAL(8, sb.length);
  STRNCMP_EQUAL("foobarlol", sb.buffer, 8);
}

TEST(StringHelper, AppendCausesGrowth)
{
  string_builder_t sb = string_builder_init(&arena, 4);

  string_builder_append(&sb, "hello"); // should grow

  CHECK(sb.capacity >= 5);
  STRNCMP_EQUAL("hello", sb.buffer, 5);
}

TEST(StringHelper, BuildProducesString)
{
  string_builder_t sb = string_builder_init(&arena, 4);

  string_builder_append(&sb, "foo");
  string_builder_append(&sb, "bar");

  string_t built = string_builder_build(&sb);

  CHECK_EQUAL(6, built.length);
  STRNCMP_EQUAL("foobar", built.string, 6);
}

TEST(StringHelper, BuilderBuildNullTerminates)
{
  string_builder_t sb = string_builder_init(&arena, 4);

  string_builder_append(&sb, "hi");
  string_t s = string_builder_build(&sb);

  const char* c = string_to_cstr(&arena, s);
  STRCMP_EQUAL("hi", c);
}
