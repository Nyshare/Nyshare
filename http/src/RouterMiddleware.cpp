#include "RouterMiddleware.h"

#include "Logger.h"

RouterMiddleware::RouterMiddleware(const Router& router) : _router(router) {}

void RouterMiddleware::handleRequest(HttpRequest& httpRequest,
                                     HttpResponse& httpResponse,
                                     std::function<void()> next) {
  if (!_router.route(httpRequest, httpResponse)) {
    info("[RouterMiddleware] Route not found:%s %s",
         httpRequest.methodToString().c_str(), httpRequest.getPath().c_str());
    httpResponse.setStatusCode(HttpResponse::NotFound);
  }
  next();
}

void RouterMiddleware::handleResponse(HttpRequest& httpRequest,
                                      HttpResponse& httpResponse) {}
