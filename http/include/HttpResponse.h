#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class HttpResponse {
 public:
  // 状态码枚举
  enum STATUS {
    OK = 200,
    NoContent = 204,
    PartialContent = 206,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304,
    TemporaryRedirect = 307,
    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    InternalServerError = 500,
    ServiceUnavailable = 503
  };

  // 常用响应头枚举
  enum HEADER {
    ACCEPT_RANGES = 0,
    AGE,
    ALLOW,
    CACHE_CONTROL,
    CONTENT_ENCODING,
    CONTENT_LANGUAGE,
    CONTENT_LENGTH,
    CONTENT_LOCATION,
    CONTENT_MD5,
    CONTENT_RANGE,
    CONTENT_TYPE,
    DATE,
    ETAG,
    EXPIRES,
    LAST_MODIFIED,
    LOCATION,
    PRAGMA,
    PROXY_AUTHENTICATE,
    REFRESH,
    RETRY_AFTER,
    SERVER,
    SET_COOKIE,
    TRAILER,
    TRANSFER_ENCODING,
    VIA,
    WARNING,
    VARY,
    WWW_AUTHENTICATE,
  };

  HttpResponse();
  HttpResponse(STATUS status, const std::string& version = "1.1");

  // 获取/设置状态码
  STATUS status() const;
  HttpResponse& status(STATUS status);

  // 获取/设置版本号
  const std::string& version() const;
  HttpResponse& version(const std::string& version);

  // 获取/设置状态文本
  const std::string& text() const;
  HttpResponse& text(const std::string& text);

  // 获取状态码标准文本
  const std::string& standard_text(STATUS status) const;

  // 检查是否包含指定响应头
  bool has_header(HEADER header) const;
  bool has_header(const std::string& header) const;

  // 获取指定响应头(不存在时返回空字符串)
  const std::string& operator[](HEADER header) const;
  const std::string& operator[](const std::string& header) const;

  // 设置指定响应头(覆盖式)
  HttpResponse& set(HEADER header, const std::string& value);
  HttpResponse& set(const std::string& header, const std::string& value);

  // 设置指定响应头(追加式)
  HttpResponse& append(HEADER header, const std::string& value);
  HttpResponse& append(const std::string& header, const std::string& value);

  // 获取/设置内容类型
  const std::string& content_type() const;
  HttpResponse& content_type(const std::string& type);

  // 获取/设置内容长度
  size_t content_length() const;
  HttpResponse& content_length(size_t length);

  // 获取/设置响应体
  const std::string& body() const;
  HttpResponse& body(const std::string& body);

  // 将 HttpResponse 对象序列化成响应字符串
  std::string to_string() const;

 private:
  STATUS status_;                                         // 响应状态码
  std::string version_;                                   // 响应版本号
  std::string text_;                                      // 响应状态文本
  std::unordered_map<std::string, std::string> headers_;  // 响应头
  std::vector<std::pair<std::string, std::string>>
      append_headers_;  // 追加响应头
  std::string body_;    // 响应体
  static const std::unordered_map<STATUS, std::string>
      text_map_;  // 状态码与状态文本的映射
  static const std::unordered_map<HEADER, std::string>
      header_map_;  // 响应头与字符串的映射
};