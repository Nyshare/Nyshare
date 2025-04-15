#pragma once

#include <functional>

#include "HttpRequest.h"
#include "HttpResponse.h"

class Middleware {
 public:
  // 声明所有中间件类必须实现的操作方法
  virtual void handleRequest(HttpRequest& httpRequest,
                             HttpResponse& httpResponse,
                             std::function<void()> next) = 0;
  virtual void handleResponse(HttpRequest& httpRequest,
                              HttpResponse& httpResponse) = 0;
};