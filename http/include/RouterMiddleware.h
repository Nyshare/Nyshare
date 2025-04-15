#pragma once

#include "Middleware.h"
#include "Router.h"

class RouterMiddleware : public Middleware {
 public:
  explicit RouterMiddleware(const Router& router);

  void handleRequest(HttpRequest& httpRequest, HttpResponse& httpResponse,
                     std::function<void()> next) override;

  void handleResponse(HttpRequest& httpRequest,
                      HttpResponse& httpResponse) override;

 private:
  const Router& _router;
};