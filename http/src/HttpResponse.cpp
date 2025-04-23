#include "HttpResponse.h"

#include "Logger.h"

HttpResponse::HttpResponse()
    : status_(OK),
      version_("1.1"),
      text_("OK"),
      headers_(),
      append_headers_(),
      body_() {}

HttpResponse::HttpResponse(STATUS status, const std::string& version)
    : status_(status),
      version_("1.1"),
      text_(text_map_.at(status)),
      headers_(),
      append_headers_(),
      body_() {}

HttpResponse::STATUS HttpResponse::status() const { return status_; }

HttpResponse& HttpResponse::status(STATUS status) {
  this->status_ = status;
  return *this;
}

const std::string& HttpResponse::version() const { return version_; }

HttpResponse& HttpResponse::version(const std::string& version) {
  for (const auto& ch : version) {
    if ((ch < '0' || ch > '9') && ch != '.') {
      warn("[HttpResponse] version is invalid");
      return *this;
    }
  }
  this->version_ = version;
  return *this;
}

const std::string& HttpResponse::text() const { return text_; }

HttpResponse& HttpResponse::text(const std::string& text) {
  this->text_ = text;
  return *this;
}

const std::string& HttpResponse::standard_text(STATUS status) const {
  return text_map_.at(status);
}

bool HttpResponse::has_header(HEADER header) const {
  return headers_.find(header_map_.at(header)) != headers_.end();
}

bool HttpResponse::has_header(const std::string& header) const {
  return headers_.find(header) != headers_.end();
}

const std::string& HttpResponse::operator[](HEADER header) const {
  static const std::string empty;
  auto it = headers_.find(header_map_.at(header));
  if (it != headers_.end()) {
    return it->second;
  }
  return empty;
}

const std::string& HttpResponse::operator[](const std::string& header) const {
  static const std::string empty;
  auto it = headers_.find(header);
  if (it != headers_.end()) {
    return it->second;
  }
  return empty;
}

HttpResponse& HttpResponse::set(HEADER header, const std::string& value) {
  headers_[header_map_.at(header)] = value;
  return *this;
}

HttpResponse& HttpResponse::set(const std::string& header,
                                const std::string& value) {
  headers_[header] = value;
  return *this;
}

HttpResponse& HttpResponse::append(HEADER header, const std::string& value) {
  append_headers_.emplace_back(header_map_.at(header), value);
  return *this;
}

HttpResponse& HttpResponse::append(const std::string& header,
                                   const std::string& value) {
  append_headers_.emplace_back(header, value);
  return *this;
}

const std::string& HttpResponse::content_type() const {
  static const std::string empty;
  auto it = headers_.find(header_map_.at(CONTENT_TYPE));
  if (it != headers_.end()) {
    return it->second;
  }
  return empty;
}

HttpResponse& HttpResponse::content_type(const std::string& type) {
  headers_[header_map_.at(CONTENT_TYPE)] = type;
  return *this;
}

size_t HttpResponse::content_length() const {
  auto it = headers_.find(header_map_.at(CONTENT_LENGTH));
  if (it != headers_.end()) {
    return std::stoul(it->second);
  }
  return 0;
}

HttpResponse& HttpResponse::content_length(size_t length) {
  headers_[header_map_.at(CONTENT_LENGTH)] = std::to_string(length);
  return *this;
}

const std::string& HttpResponse::body() const { return body_; }

HttpResponse& HttpResponse::body(const std::string& body) {
  this->body_ = body;
  return *this;
}

std::string HttpResponse::to_string() const {
  std::string response;

  // 生成响应行
  response.append("HTTP/")
      .append(version_)
      .append(" ")
      .append(std::to_string(status_))
      .append(" ")
      .append(text_)
      .append("\r\n");

  // 生成响应头
  for (const auto& it : headers_) {
    response.append(it.first).append(": ").append(it.second).append("\r\n");
  }

  // 生成追加响应头
  for (const auto& it : append_headers_) {
    response.append(it.first).append(": ").append(it.second).append("\r\n");
  }

  // 生成空行
  response.append("\r\n");

  // 生成响应体
  response.append(body_);

  return std::move(response);
}

const std::unordered_map<HttpResponse::STATUS, std::string>
    HttpResponse::text_map_ = {{OK, "OK"},
                               {NoContent, "NoContent"},
                               {PartialContent, "PartialContent"},
                               {MovedPermanently, "MovedPermanently"},
                               {Found, "Found"},
                               {SeeOther, "SeeOther"},
                               {NotModified, "NotModified"},
                               {TemporaryRedirect, "TemporaryRedirect"},
                               {BadRequest, "BadRequest"},
                               {Unauthorized, "Unauthorized"},
                               {Forbidden, "Forbidden"},
                               {NotFound, "NotFound"},
                               {InternalServerError, "InternalServerError"},
                               {ServiceUnavailable, "ServiceUnavailable"}};

const std::unordered_map<HttpResponse::HEADER, std::string>
    HttpResponse::header_map_ = {
        {ACCEPT_RANGES, "Accept-Ranges"},
        {AGE, "Age"},
        {ALLOW, "Allow"},
        {CACHE_CONTROL, "Cache-Control"},
        {CONTENT_ENCODING, "Content-Encoding"},
        {CONTENT_LANGUAGE, "Content-Language"},
        {CONTENT_LENGTH, "Content-Length"},
        {CONTENT_LOCATION, "Content-Location"},
        {CONTENT_MD5, "Content-MD5"},
        {CONTENT_RANGE, "Content-Range"},
        {CONTENT_TYPE, "Content-Type"},
        {DATE, "Date"},
        {ETAG, "ETag"},
        {EXPIRES, "Expires"},
        {LAST_MODIFIED, "Last-Modified"},
        {LOCATION, "Location"},
        {PRAGMA, "Pragma"},
        {PROXY_AUTHENTICATE, "Proxy-Authenticate"},
        {REFRESH, "Refresh"},
        {RETRY_AFTER, "Retry-After"},
        {SERVER, "Server"},
        {SET_COOKIE, "Set-Cookie"},
        {TRAILER, "Trailer"},
        {TRANSFER_ENCODING, "Transfer-Encoding"},
        {VIA, "Via"},
        {WARNING, "Warning"},
        {VARY, "Vary"},
        {WWW_AUTHENTICATE, "WWW-Authenticate"},
};