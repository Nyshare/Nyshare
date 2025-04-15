#pragma once

#include <string>
#include <unordered_map>

class HttpResponse {
 public:
  // 状态码枚举
  enum StatusCode {
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

  HttpResponse();

  void setStatusCode(StatusCode statusCode);
  void addHeader(const std::string &key, const std::string &value);
  void setBody(const std::string body);

  // 将HttpResponse对象序列化成响应字符串
  std::string toString() const;
  StatusCode getStatusCode() const;
  // 返回字符串格式的状态码描述
  const std::string &statusToString() const;

 private:
  StatusCode _statusCode;                                 // 响应状态码
  std::unordered_map<std::string, std::string> _headers;  // 响应头
  std::string _body;                                      // 响应体
  static const std::unordered_map<HttpResponse::StatusCode, std::string>
      _statusTextMap;
};