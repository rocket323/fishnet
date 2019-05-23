#include "http_response.h"

HttpResponse::HttpResponse() : parse_state_(PARSE_STATUS_LINE)
{
}

HttpResponse::~HttpResponse()
{
}

const std::string &HttpResponse::Header(const std::string &key) const
{
    auto iter = headers_.find(key);
    if (iter != headers_.end())
        return iter->second;
    return HTTP_HEADER_NONE;
}

size_t HttpResponse::ContentLength() const
{
    auto iter = headers_.find("Content-Length");
    size_t content_length = 0;
    if (iter != headers_.end())
    {
        int tmp = std::stoi(iter->second);
        if (tmp > 0)
            content_length = tmp;
    }
    return content_length;
}

bool HttpResponse::KeepAlive() const
{
    const std::string value = Header("Connection");

    // HTTP/1.1 supports keep-alive default
    if (value == HTTP_HEADER_NONE)
        return proto_ == HTTP_VERSION_1_1;

    return (value == "Keep-Alive" || value == "keep-alive");
}

void HttpResponse::SetStatusCode(int status_code)
{
    status_code_ = status_code;
    status_message_ = HttpUtil::GetHttpStatusReason(stauts_code);
}

void HttpResponse::SetKeepAlive(bool on)
{
    if (on)
        SetHeader("Connection", "Keep-Alive");
    else
        SetHeader("Connection", "close");
}

void HttpResponse::Write(const std::string &str)
{
    Write(str.data(), str.length());
}

void HttpResponse::Write(const char *data, size_t len)
{
    body_.append(data, len);
}

std::string HttpResponse::ToString() const
{
    const std::string crlf("\r\n");
    std::string str;
    str.reserve(1024);
    char buf[256];

    // Status line
    str += proto_ + " " + std::to_string((long long)status_code_) + " " + status_message_ + crlf;

    // Set conent length if has body
    if (body_.length() > 0)
    {
        snprintf(buf, sizeof(buf), "Content-Length: %lu\r\n", body_.length());
        str += buf;
    }

    for (auto iter = headers_.begin(); iter != headers_.end(); iter++)
    {
        if (iter->second.empty())
            continue;
        str += iter->first + ": " + iter->second;
        str += crlf;
    }
    str += crlf;

    // Set body if has
    if (body_.length() > 0)
        str += body_;

    return str;
}

void HttpResponse::Reset()
{
    proto_ = "HTTP/1.1";
    SetStatusCode(HttpStatus_OK);
    headers_.clear();
    body_.clear();
    parse_state_ = PARSE_STATUS_LINE;
}
