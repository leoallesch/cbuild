#include "CppUTest/TestHarness.h"

TEST_GROUP(Default)
{
};

TEST(Default, PassMe)
{
   CHECK_TRUE(1 == 1);
}