#ifndef _NET_REDIS_COMMAND_H_
#define _NET_REDIS_COMMAND_H_

#include <string>
#include <vector>
class RedisConnection;

class RedisCommand {
public:
    RedisCommand &operator<<(const std::string &str) {
        args_.push_back(str);
        return *this;
    }
    RedisCommand &operator<<(const char *str) {
        args_.push_back(str);
        return *this;
    }
    RedisCommand &operator<<(int val) {
        args_.push_back(std::to_string(static_cast<long long>(val)));
        return *this;
    }
    RedisCommand &operator<<(unsigned val) {
        args_.push_back(std::to_string(static_cast<unsigned long long>(val)));
        return *this;
    }
    RedisCommand &operator<<(long long val) {
        args_.push_back(std::to_string(static_cast<long long>(val)));
        return *this;
    }
    RedisCommand &operator<<(unsigned long long val) {
        args_.push_back(std::to_string(static_cast<unsigned long long>(val)));
        return *this;
    }
    RedisCommand &operator<<(double val) {
        args_.push_back(std::to_string(static_cast<long double>(val)));
        return *this;
    }

    friend class RedisClient;

private:
    std::vector<std::string> args_;
};

class RedisCommands {
public:
    RedisCommands &Add(const RedisCommand &cmd) {
        cmds_.push_back(cmd);
        return *this;
    }

    friend class RedisClient;

private:
    std::vector<RedisCommand> cmds_;
};

#endif
