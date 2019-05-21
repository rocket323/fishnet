#ifndef _NET_UTIL_H_
#define _NET_UTIL_H_

class Util
{
public:
    static int64_t CurrentSystemTime() { return 0; }
};

#ifdef _WIN32
#define likely(x) (x)
#define unlikely(x) (x)
#else
// expression x likely true.
#define likely(x) __builtin_expect(!!(x), true)

// expression x likely false.
#define unlikely(x) __builtin_expect(!!(x), false)
#endif

#define DEBUG(fmt, args...) printf("[debug]%s::%s():%d|" fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##args);

#define INFO(fmt, args...) printf("[info]%s::%s():%d|" fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##args);

#define ERR(fmt, args...) printf("[error]%s::%s():%d|" fmt "\n", __FILE__, __FUNCTION__, __LINE__, ##args);

#endif
