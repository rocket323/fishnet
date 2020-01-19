#include "http_request.h"

HttpRequest::HttpRequest() {
    Reset();
}

HttpRequest::~HttpRequest() {
}

const std::string &HttpRequest::Header(const std::string &key) const {
    auto iter = headers_.find(key);
    if (iter != headers_.end())
        return iter->second;
    return HTTP_HEADER_NONE;
}

size_t HttpRequest::ContentLength() const {
    auto iter = headers_.find("Content-Length");
    size_t content_length = 0;
    if (iter != headers_.end()) {
        int tmp = stoi(iter->second);
        if (tmp > 0)
            content_length = tmp;
    }
    return content_length;
}

bool HttpRequest::KeepAlive() const {
    const std::string value = Header("Connection");

    // HTTP/1.1 supports keep-alive default
    if (value == HTTP_HEADER_NONE)
        return proto_ == HTTP_VERSION_1_1;

    return (value == "Keep-Alive" || value == "keep-alive");
}

void HttpRequest::SetUrl(const std::string &url) {
    if (url.length() == 0)
        return;

    if (url[0] == '/') {
        url_ = url;
        return;
    }

    // Only supports http now
    const std::string scheme("http://");
    if (!Util::BeginWith(url, scheme))
        return;

    std::string host = url.substr(scheme.length());
    size_t slash = host.find('/');
    if (slash == std::string::npos) {
        SetHeader("Host", host);
        url_ = '/';
    } else {
        SetHeader("Host", host.substr(0, slash));
        url_ = host.substr(slash);
    }
}

void HttpRequest::SetKeepAlive(bool on) {
    if (on)
        SetHeader("Connection", "Keep-Alive");
    else
        SetHeader("Connection", "close");
}

void HttpRequest::Write(const std::string &str) {
    Write(str.data(), str.length());
}

void HttpRequest::Write(const char *data, size_t len) {
    body_.append(data, len);
}

std::string HttpRequest::ToString() const {
    std::string crlf("\r\n");
    std::string str;
    char buf[256];

    // Request line
    str = method_ + " " + url_ + " " + proto_ + crlf;

    // Set content length if has body
    if (body_.length() > 0) {
        sprintf(buf, sizeof(buf), "Content-Length: %lu\r\n", body_.length());
        str += buf;
    }

    for (auto &kv : headers_) {
        if (kv.second.empty())
            continue;
        str += kv.first + ": " + kv.second;
        str += crlf;
    }
    str += crlf;

    // Set body if has body
    if (body_.length() > 0)
        str += body_;

    return str;
}

void HttpRequest::Reset() {
    SetMethod("GET");
    SetUrl("/");
    SetProto(HTTP_VERSION_1_1);
    headers_.clear();
    body_.clear();
    parse_state_ = PARSE_REQUEST_LINE;
}
