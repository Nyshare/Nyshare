#include "LogMiddleware.h"

#include "Logger.h"

void LogMiddleware::handleRequest(HttpRequest& httpRequest,
                                  HttpResponse& httpResponse,
                                  std::function<void()> next) {
  info("[LogMiddleware] Incoming Request:%s %s",
       httpRequest.methodStr().c_str(), httpRequest.url().c_str());
  next();
}

void LogMiddleware::handleResponse(HttpRequest& httpRequest,
                                   HttpResponse& httpResponse) {
  info("[LogMiddleware] Outgoing Response:%d %s", httpResponse.status(),
       httpResponse.text().c_str());
}
