#include "tcp/buffer.h"
#include "gtest/gtest.h"

TEST(buffer, append)
{
    Buffer buffer;
    buffer.Append("hello world", 10);
}
