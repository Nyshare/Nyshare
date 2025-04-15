#include "LogMiddleware.h"

#include "/home/orchid/Server/util/include/Logger.h"

void LogMiddleware::handleRequest(HttpRequest& httpRequest,
                                  HttpResponse& httpResponse,
                                  std::function<void()> next) {
  info("[LogMiddleware] Incoming Request:%s %s",
       httpRequest.methodToString().c_str(), httpRequest.getPath().c_str());
  next();
}

void LogMiddleware::handleResponse(HttpRequest& httpRequest,
                                   HttpResponse& httpResponse) {
  info("[LogMiddleware] Outgoing Response:%d %s", httpResponse.getStatusCode(),
       httpResponse.statusToString().c_str());
}
