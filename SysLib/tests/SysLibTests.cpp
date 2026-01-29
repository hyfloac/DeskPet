#include <gtest/gtest.h>
#include "SysLib.h"

TEST(StringLengthCTest, EmptyString) {
    EXPECT_EQ(StringLengthC(""), 0u);
}

TEST(StringLengthCTest, SingleChar) {
    EXPECT_EQ(StringLengthC("a"), 1u);
}

TEST(StringLengthCTest, ShortString) {
    EXPECT_EQ(StringLengthC("ab"), 2u);
    EXPECT_EQ(StringLengthC("hello"), 5u);
}

TEST(StringLengthCTest, StringWithSpaces) {
    EXPECT_EQ(StringLengthC("a b"), 3u);
}

TEST(StringLengthCTest, LongerOrSpecial) {
    EXPECT_EQ(StringLengthC("hello world!"), 12u);
    EXPECT_EQ(StringLengthC("foo\tbar\n"), 8u);
}
