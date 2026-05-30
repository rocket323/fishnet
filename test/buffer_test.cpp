#include "tcp/buffer.h"
#include "gtest/gtest.h"

TEST(BufferTest, Basic) {
    Buffer buffer;
    const size_t initial_size = 1024;
    EXPECT_EQ(buffer.ReadableBytes(), 0);
    EXPECT_EQ(buffer.WritableBytes(), initial_size);

    const std::string str(200, 'x');
    buffer.Append(str);
    EXPECT_EQ(buffer.ReadableBytes(), 200);
    EXPECT_EQ(buffer.WritableBytes(), initial_size - 200);

    std::string retrieved = buffer.RetrieveAllToString();
    EXPECT_EQ(retrieved, str);
    EXPECT_EQ(buffer.ReadableBytes(), 0);
    EXPECT_EQ(buffer.WritableBytes(), initial_size);
}

TEST(BufferTest, AppendAndRetrieve) {
    Buffer buffer;
    buffer.Append("hello");
    EXPECT_EQ(buffer.ReadableBytes(), 5);
    
    EXPECT_EQ(std::string(buffer.Peek(), 2), "he");
    buffer.Retrieve(2);
    EXPECT_EQ(buffer.ReadableBytes(), 3);
    EXPECT_EQ(std::string(buffer.Peek(), 3), "llo");

    buffer.Append(" world");
    EXPECT_EQ(buffer.ReadableBytes(), 9);
    EXPECT_EQ(buffer.RetrieveAllToString(), "llo world");
    EXPECT_EQ(buffer.ReadableBytes(), 0);
}

TEST(BufferTest, Find) {
    Buffer buffer;
    buffer.Append("GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n");
    
    const char* crlf = buffer.Find("\r\n");
    ASSERT_TRUE(crlf != nullptr);
    EXPECT_EQ(std::string(buffer.Peek(), crlf), "GET /index.html HTTP/1.1");
    
    buffer.RetrieveUntil(crlf + 2);
    EXPECT_EQ(std::string(buffer.Peek(), 6), "Host: ");
}

TEST(BufferTest, Resize) {
    Buffer buffer;
    std::string large_str(2000, 'y');
    buffer.Append(large_str);
    EXPECT_EQ(buffer.ReadableBytes(), 2000);
    EXPECT_GE(buffer.Cap(), 2000);
    EXPECT_EQ(buffer.RetrieveAllToString(), large_str);
}

TEST(BufferTest, InternalMove) {
    Buffer buffer;
    buffer.Append(std::string(800, 'a'));
    buffer.Retrieve(500);
    EXPECT_EQ(buffer.ReadableBytes(), 300);
    
    // Now read_index is 500, write_index is 800.
    // If we append something that fits in total capacity but not in current writable space,
    // it should move the data to the front.
    buffer.Append(std::string(600, 'b')); 
    EXPECT_EQ(buffer.ReadableBytes(), 900);
    std::string result = buffer.RetrieveAllToString();
    EXPECT_EQ(result.substr(0, 300), std::string(300, 'a'));
    EXPECT_EQ(result.substr(300), std::string(600, 'b'));
}
