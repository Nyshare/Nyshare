#include "RouterMiddleware.h"

#include "Logger.h"

RouterMiddleware::RouterMiddleware(std::shared_ptr<Router> router)
    : router_(router) {}

void RouterMiddleware::handleRequest(HttpRequest& httpRequest,
                                     HttpResponse& httpResponse,
                                     std::function<void()> next) {
  if (!router_->route(httpRequest, httpResponse)) {
    // 路由匹配失败，交由下一个中间件处理
    next();
  }
}

void RouterMiddleware::handleResponse(HttpRequest& httpRequest,
                                      HttpResponse& httpResponse) {}
