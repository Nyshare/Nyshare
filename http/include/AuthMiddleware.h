#pragma once

#include "Middleware.h"

// token验证中间件
class AuthMiddleware : public Middleware {
 public:
  void handleRequest(HttpRequest& httpRequest, HttpResponse& httpResponse,
                     std::function<void()> next) override;
  void handleResponse(HttpRequest& httpRequest,
                      HttpResponse& httpResponse) override;
};