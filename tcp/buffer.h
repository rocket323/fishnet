#ifndef _NET_BUFFER_H_
#define _NET_BUFFER_H_

#include <string>
#include <vector>

class Buffer
{
public:
    static const size_t INITIAL_SIZE = 1024;

    Buffer();

    void Append(const std::string &str);
    void Append(const char *data, size_t len);

    const char *Peek();
    void Retrieve(size_t n);
    void RetrieveUntil(const char *end);
    std::string RetrieveAllToString();
    void RetrieveAll();

    size_t ReadableBytes() const { return write_index_ - read_index_; }
    size_t WritableBytes() const { return Cap() - write_index_; }
    size_t Cap() const { return buffer_.size(); }

    ssize_t ReadFromFd(int fd);
    void Swap(Buffer &rhs);

private:
    char *Begin() { return &*buffer_.begin(); }
    char *BeginRead() { return Begin() + read_index_; }
    char *BeginWrite() { return Begin() + write_index_; }

    // MakeSpace(n) to guarentee space for another n bytes.
    // After MakeSpace(n), at least n bytes can be written to the
    // buffer without another allocation.
    void MakeSpace(size_t n);

private:
    std::vector<char> buffer_;
    int read_index_;
    int write_index_;
};

#endif
