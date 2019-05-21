#ifndef _NET_HTTP_REQUEST_H_
#define _NET_HTTP_REQUEST_H_

#include <map>
#include <string>
#include <vector>

class HttpRequest
{
public:
    enum ParseState
    {
        PARSE_REQUEST_LINE,
        PARSE_HEADERS,
        PARSE_BODY,
        PARSE_DONE,
    };

    typedef std::map<std::string, std::string> Headers;

    HttpRequest();
    ~HttpRequest();

    // getters
    const std::string &Url() const { return url_; }
    const std::string &Method() const { return method_; }
    const std::string &Proto() const { return proto_; }
    const std::string &Header(const std::string &key) const;
    const std::string &Body() const { return body_; }
    const std::string &ContentType() const { return Header("Content-Type"); }
    size_t ContentLength() const;
    bool KeepAlive() const;

    // setters
    void SetUrl(const std::string &url);
    void SetMethod(const std::string &method) { method_ = method; }
    void SetProto(const std::string &proto) { proto_ = proto; }
    void SetHeader(const std::string &key, const std::string &value) { headers_[key] = value; }
    void SetContentType(const std::string &value) { SetHeader("Content-Type", value); }
    void SetContentLength(size_t len) { SetHeader("Content-Length", std::to_string((unsigned long long)len)); }
    void SetKeepAlive(bool on);

    void Write(const std::string &str);
    void Write(const char *data, size_t len);

    std::string ToString() const;

    ParseState State() const { return parse_state_; }
    void SetState(ParseState state) { parse_state_ = state; }

    void Reset();

private:
    std::string url_;
    std::string method_;
    std::string proto_;

    Headers headers_;
    std::string body_;

    ParseState state_;
};

#endif
