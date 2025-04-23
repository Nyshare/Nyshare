#pragma once

#include <memory>

#include "Middleware.h"
#include "Router.h"

class RouterMiddleware : public Middleware {
 public:
  explicit RouterMiddleware(std::shared_ptr<Router> router);

  void handleRequest(HttpRequest& httpRequest, HttpResponse& httpResponse,
                     std::function<void()> next) override;

  void handleResponse(HttpRequest& httpRequest,
                      HttpResponse& httpResponse) override;

 private:
  std::shared_ptr<Router> router_;
};