#include "Router.h"

void Router::addRoute(HttpRequest::METHOD method, const std::string& path,
                      HandleFunc handler) {
  this->_routes[method][path] = handler;
}

bool Router::route(HttpRequest& httpRequest, HttpResponse& httpResponse) const {
  auto methodRoute = _routes.find(httpRequest.method());
  if (methodRoute == _routes.end()) {
    return false;
  }
  const auto& pathToHandlerMap = methodRoute->second;
  auto pathRoute = pathToHandlerMap.find(httpRequest.url());
  if (pathRoute == pathToHandlerMap.end()) {
    return false;
  }
  pathRoute->second(httpRequest, httpResponse);
  return true;
}
