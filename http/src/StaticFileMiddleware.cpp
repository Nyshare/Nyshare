#include "StaticFileMiddleware.h"

#include "Filer.h"
#include "Logger.h"

StaticFileMiddleware::StaticFileMiddleware(const std::string& staticDir)
    : _staticDir(staticDir) {}

void StaticFileMiddleware::handleRequest(HttpRequest& httpRequest,
                                         HttpResponse& httpResponse,
                                         std::function<void()> next) {
  if (httpRequest.method() != HttpRequest::GET) {
    next();
    return;
  }
  std::string path = httpRequest.url();
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
      httpResponse.body(fallbackContent)
          .status(HttpResponse::NotFound)
          .content_length(fallbackContent.size())
          .content_type(Filer::getMimeTypeFromFilePath(fallbackPath));
    } else {
      httpResponse.status(HttpResponse::NotFound);
    }
    return;
  }
  info("[StaticFileMiddleware] Serving file: %s", fullPath.c_str());
  std::string buffer = Filer::readFile(fullPath);
  if (buffer.empty()) {
    warn("[StaticFileMiddleware] File is empty:%s", fullPath.c_str());
    httpResponse.status(HttpResponse::InternalServerError);
    return;
  }
  httpResponse.body(buffer)
      .status(HttpResponse::OK)
      .content_length(buffer.size())
      .content_type(Filer::getMimeTypeFromFilePath(fullPath));
}

void StaticFileMiddleware::handleResponse(HttpRequest& httpRequest,
                                          HttpResponse& httpResponse) {}
