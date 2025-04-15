#pragma once

#include <memory>
#include <vector>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Middleware.h"

class MiddlewareChain {
 public:
  // 添加中间件到链中
  void addMiddleware(std::unique_ptr<Middleware> middleware);
  // 开始处理请求：依次执行所有中间件的 handleRequest，
  // 并在请求结束后，逆序调用 handleResponse
  void handle(HttpRequest &httpRequest, HttpResponse &httpResponse);

 private:
  // 执行handleRequest
  void execute(size_t index, HttpRequest &httpRequest,
               HttpResponse &httpResponse) const;

 private:
  std::vector<std::unique_ptr<Middleware>> _middlewares;
};