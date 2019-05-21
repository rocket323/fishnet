#include "buffer.h"
#include <assert.h>
#include <sys/uio.h>
#include <algorithm>

Buffer::Buffer() : buffer_(INITIAL_SIZE), read_index_(0), write_index_(0)
{
}

void Buffer::Append(const std::string &str)
{
    Append(str.data(), str.length());
}

void Buffer::Append(const char *data, size_t len)
{
    if (WritableBytes() < len)
        MakeSpace(len);
    std::copy(data, data + len, BeginWrite());
    write_index_ += len;
}

const char *Buffer::Peek()
{
    return BeginRead();
}

const char *Buffer::Find(const char *begin, const char *end)
{
    return FindFrom(BeginRead(), begin, end);
}

const char *Buffer::Find(const std::string &delimiter)
{
    assert(!delimiter.empty());
    return Find(delimiter.data(), delimiter.data() + delimiter.length());
}

const char *Buffer::FindFrom(const char *from, const char *begin, const char *end)
{
    assert(from >= BeginRead());
    assert(from <= BeginWrite());
    const char *found = std::search(from, (const char *)BeginWrite(), begin, end);
    if (found == BeginWrite())
        return NULL;
    return found;
}

void Buffer::Retrieve(size_t n)
{
    assert(ReadableBytes() >= n);
    read_index_ += n;
}

void Buffer::RetrieveUntil(const char *end)
{
    assert(end >= BeginRead());
    assert(end <= BeginWrite());
    Retrieve(end - BeginRead());
}

std::string Buffer::RetrieveAllToString()
{
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

void Buffer::RetrieveAll()
{
    read_index_ = 0;
    write_index_ = 0;
}

ssize_t Buffer::ReadFromFd(int fd)
{
    struct iovec iov[2];
    char buf[32 * 1024];
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = WritableBytes();
    iov[1].iov_base = buf;
    iov[1].iov_len = sizeof(buf);
    ssize_t nread = readv(fd, iov, 2);
    if (nread >= 0)
    {
        if ((size_t)nread <= WritableBytes())
        {
            write_index_ += nread;
        }
        else
        {
            const size_t len = nread - WritableBytes();
            write_index_ += WritableBytes();
            Append(buf, len);
        }
    }
    else
    {
        // error
    }
    return nread;
}

void Buffer::Swap(Buffer &rhs)
{
    std::swap(buffer_, rhs.buffer_);
    std::swap(read_index_, rhs.read_index_);
    std::swap(write_index_, rhs.write_index_);
}

void Buffer::MakeSpace(size_t n)
{
    if (WritableBytes() >= n)
        return;

    if (buffer_.size() - ReadableBytes() >= n)
    {
        size_t readable = ReadableBytes();
        std::copy(BeginRead(), BeginWrite(), Begin());
        read_index_ = 0;
        write_index_ = read_index_ + readable;
    }
    else
    {
        size_t readable = ReadableBytes();
        std::vector<char> b(readable + n);
        std::copy(BeginRead(), BeginWrite(), b.data());
        std::swap(buffer_, b);
        read_index_ = 0;
        write_index_ = read_index_ + readable;
    }
}
