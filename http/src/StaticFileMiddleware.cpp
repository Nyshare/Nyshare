#include "StaticFileMiddleware.h"

#include "Filer.h"
#include "Logger.h"

StaticFileMiddleware::StaticFileMiddleware(const std::string& staticDir)
    : _staticDir(staticDir) {}

void StaticFileMiddleware::handleRequest(HttpRequest& httpRequest,
                                         HttpResponse& httpResponse,
                                         std::function<void()> next) {
  if (httpRequest.getMethod() != HttpRequest::GET) {
    next();
    return;
  }
  std::string path = httpRequest.getPath();
  if (path == "/") {
    path = "/index.html";
  }
  std::string fullPath = _staticDir + path;

  if (!Filer::fileExists(fullPath)) {
    warn("[StaticFileMiddleware] File not found: %s",
         Filer::getAbsolutePath(fullPath).c_str());
    // 查找 404.html 文件作为 fallback 内容
    std::string fallbackPath = _staticDir + "/404.html";
    if (Filer::fileExists(fallbackPath)) {
      std::string fallbackContent = Filer::readFile(fallbackPath);
      httpResponse.setBody(fallbackContent);
      httpResponse.setStatusCode(HttpResponse::NotFound);
      httpResponse.addHeader("Content-Length",
                             std::to_string(fallbackContent.size()));
      httpResponse.addHeader("Content-Type",
                             Filer::getMimeTypeFromFilePath(fallbackPath));
    } else {
      httpResponse.setStatusCode(HttpResponse::NotFound);
    }
    return;
  }
  info("[StaticFileMiddleware] Serving file: %s", fullPath.c_str());
  std::string buffer = Filer::readFile(fullPath);
  if (buffer.empty()) {
    warn("[StaticFileMiddleware] File is empty:%s", fullPath.c_str());
    httpResponse.setStatusCode(HttpResponse::InternalServerError);
    return;
  }
  httpResponse.setBody(buffer);
  httpResponse.setStatusCode(HttpResponse::OK);
  httpResponse.addHeader("Content-Length", std::to_string(buffer.size()));
  httpResponse.addHeader("Content-Type",
                         Filer::getMimeTypeFromFilePath(fullPath));
  httpResponse.addHeader("Connection", "keep-alive");
}

void StaticFileMiddleware::handleResponse(HttpRequest& httpRequest,
                                          HttpResponse& httpResponse) {}
