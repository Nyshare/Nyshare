#pragma once

#include "Middleware.h"

class LogMiddleware : public Middleware {
 public:
  void handleRequest(HttpRequest& httpRequest, HttpResponse& httpResponse,
                     std::function<void()> next) override;
  void handleResponse(HttpRequest& httpRequest,
                      HttpResponse& httpResponse) override;
};