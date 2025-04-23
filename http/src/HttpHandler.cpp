#include "HttpHandler.h"

#include <iostream>

#include "AuthHandler.h"
#include "AuthMiddleware.h"
#include "ErrorMiddleware.h"
#include "LogMiddleware.h"
#include "Logger.h"
#include "PostsHandler.h"
#include "RouterMiddleware.h"
#include "StaticFileMiddleware.h"

HttpHandler::HttpHandler(std::shared_ptr<MiddlewareChain> middlewareChain)
    : _httpRequest(std::make_unique<HttpRequest>()),
      _httpResponse(std::make_unique<HttpResponse>()),
      _middlewareChain(middlewareChain) {}

std::string HttpHandler::handleHttpRequest(const std::string& rawRequest) {
  // 1、将原始 HTTP 请求字符串解析成 HttpRequest 对象
  // _httpRequest = std::make_unique<HttpRequest>(rawRequest);
  auto request = std::make_unique<HttpRequest>(rawRequest);
  auto response = std::make_unique<HttpResponse>();
  // 中间件链处理请求
  _middlewareChain->handle(*request, *response);
  // 返回最终响应
  return response->to_string();
}