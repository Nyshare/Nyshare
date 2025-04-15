#include "HttpResponse.h"

HttpResponse::HttpResponse() : _statusCode(OK) {}

void HttpResponse::setStatusCode(StatusCode statusCode) {
  this->_statusCode = statusCode;
}

void HttpResponse::addHeader(const std::string& key, const std::string& value) {
  _headers[key] = value;
}

void HttpResponse::setBody(const std::string body) { this->_body = body; }

std::string HttpResponse::toString() const {
  std::string response;

  // 生成相应行
  response.append("HTTP/1.1 ")
      .append(std::to_string(_statusCode))
      .append(" ")
      .append(statusToString())
      .append("\r\n");

  // 生成响应头
  for (const auto& it : _headers) {
    response.append(it.first).append(": ").append(it.second).append("\r\n");
  }

  // 空行
  response.append("\r\n");

  // 生成响应体
  response.append(_body);

  return response;
}

HttpResponse::StatusCode HttpResponse::getStatusCode() const {
  return _statusCode;
}

const std::string& HttpResponse::statusToString() const {
  return _statusTextMap.at(_statusCode);
}

const std::unordered_map<HttpResponse::StatusCode, std::string>
    HttpResponse::_statusTextMap = {
        {OK, "OK"},
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
