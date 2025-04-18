#include "ErrorMiddleware.h"

#include <iostream>

#include "Logger.h"

void ErrorMiddleware::handleRequest(HttpRequest& httpRequest,
                                    HttpResponse& httpResponse,
                                    std::function<void()> next) {
  try {
    next();
  } catch (const std::exception& e) {
    // 程序发生异常，设置状态码InternalServerError
    httpResponse.setStatusCode(HttpResponse::InternalServerError);
    error("[ErrorMiddleware] exception caught: %s", e.what());
  }
}

void ErrorMiddleware::handleResponse(HttpRequest& httpRequest,
                                     HttpResponse& httpResponse) {
  return;
}
