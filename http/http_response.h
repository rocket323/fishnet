#ifndef _NET_HTTP_RESPONSE_H_
#define _NET_HTTP_RESPONSE_H_

#include <memory>
#include <string>
#include <vector>
#include "http_util.h"

class HttpResponse
{
public:
    enum ParseState
    {
        PARSE_STATUS_LINE,
        PARSE_HEADERS,
        PARSE_BODY,
        PARSE_DONE,
    };

    typedef std::map<std::string, std::string> Headers;

    HttpResponse();
    ~HttpResponse();

    bool ParseDone() const { return parse_state_ == PARSE_DONE; }

    // getters
    const std::string Proto() const { return proto_; }
    int StatusCode() const { return status_code_; }
    const std::string &StatusMessage() const { return status_message_; }
    const std::string &Header(const std::string &key) const;
    const std::string &ContentType() const { return Header("Content-Length"); }
    size_t ContentLength() const;
    const std::string &Body() const { return body_; }
    bool KeepAlive() const;

    // setters
    void SetProto(std::string &proto) { proto_ = proto; }
    void SetStatusCode(int status_code);
    void SetStatusMessage(const std::string &status_message) { status_message_ = status_message; }
    void SetHeader(const std::string &key, const std::string &value) { headers_[key] = value; }
    void SetConentType(const std::string &value) { SetHeader("Content-Type", value); }
    void SetConentLength(size_t len) { SetHeader("Content-Length", std::to_string((unsigned long long)len)); }
    void SetKeepAlive(bool on);

    void Write(const std::string &str);
    void Write(const char *data, size_t len);

    std::string ToString() const;

    ParseState State() const { return parse_state_; }
    void SetState(ParseState state) { parse_state_ = state; }

    void Reset();

private:
    std::string proto_;
    int statue_code_;
    std::string status_message_;

    Headers headers_;
    std::string body_;
    ParseState parse_state_;
};

#endif
