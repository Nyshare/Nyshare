#include "MiddlewareChain.h"

void MiddlewareChain::addMiddleware(std::unique_ptr<Middleware> middleware) {
  this->_middlewares.emplace_back(std::move(middleware));
}

void MiddlewareChain::handle(HttpRequest& httpRequest,
                             HttpResponse& httpResponse) {
  // 执行httpRequest
  this->execute(0, httpRequest, httpResponse);
  // 执行httpResponse
  for (const auto& middleware : _middlewares) {
    middleware->handleResponse(httpRequest, httpResponse);
  }
}

void MiddlewareChain::execute(size_t index, HttpRequest& httpRequest,
                              HttpResponse& httpResponse) const {
  if (index >= _middlewares.size()) {
    return;
  }
  auto next = [this, index, &httpRequest, &httpResponse]() {
    this->execute(index + 1, httpRequest, httpResponse);
  };
  _middlewares[index]->handleRequest(httpRequest, httpResponse,
                                     std::move(next));
}
