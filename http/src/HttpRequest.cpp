#include "HttpRequest.h"

#include <sstream>

#include "Logger.h"

HttpRequest::HttpRequest()
    : method_(GET),
      url_("/"),
      params_(),
      version_("1.1"),
      headers_(),
      body_() {}

HttpRequest::HttpRequest(const std::string& rawRequest)
    : method_(GET), url_("/"), params_(), version_("1.1"), headers_(), body_() {
  if (!parse(rawRequest)) {
    warn("[HttpRequest] parse request failed:%s", rawRequest.c_str());
    method_ = GET;
    url_ = "/";
    params_.clear();
    version_ = "1.1";
    headers_.clear();
    body_.clear();
  }
}

HttpRequest::METHOD HttpRequest::method() const { return method_; }

const std::string& HttpRequest::methodStr() const {
  static const std::unordered_map<METHOD, std::string> methodMap = {
      {METHOD::GET, "GET"},         {METHOD::HEAD, "HEAD"},
      {METHOD::POST, "POST"},       {METHOD::PUT, "PUT"},
      {METHOD::DELETE, "DELETE"},   {METHOD::CONNECT, "CONNECT"},
      {METHOD::OPTIONS, "OPTIONS"}, {METHOD::TRACE, "TRACE"},
      {METHOD::UNKNOWN, "UNKNOWN"}};
  return methodMap.at(method_);
}

const std::string& HttpRequest::url() const { return url_; }

const std::string& HttpRequest::param(const std::string& key) const {
  static const std::string empty;
  auto it = params_.find(key);
  if (it != params_.end()) {
    return it->second;
  }
  return empty;
}

const std::string& HttpRequest::version() const { return version_; }

bool HttpRequest::has_header(HEADER key) const {
  return headers_.find(header_map_.at(key)) != headers_.end();
}

bool HttpRequest::has_header(const std::string& key) const {
  return headers_.find(key) != headers_.end();
}

const std::string& HttpRequest::operator[](HEADER header) const {
  static const std::string empty;
  auto it = headers_.find(header_map_.at(header));
  if (it != headers_.end()) {
    return it->second;
  }
  return empty;
}

const std::string& HttpRequest::operator[](const std::string& key) const {
  static const std::string empty;
  auto it = headers_.find(key);
  if (it != headers_.end()) {
    return it->second;
  }
  return empty;
}

const std::string& HttpRequest::body() const { return body_; }

bool HttpRequest::parse(const std::string& rawRequest) {
  std::istringstream requestStream(rawRequest);

  // 解析请求行
  std::string requestLine;
  std::getline(requestStream, requestLine);  // 读取请求行
  if (requestLine.empty()) {
    return false;
  }

  if (!parseRequestLine(requestLine)) {
    return false;  // 请求行解析失败
  }

  // 解析请求头
  std::string headersLine;
  std::vector<std::string> headersLines;
  while (std::getline(requestStream, headersLine)) {
    if (headersLine == "\r") {
      break;  // 空行代表请求头读取结束
    }
    if (headersLine.back() == '\r') {
      headersLine.pop_back();  // 去除每行请求头末尾的'\r'
    }
    headersLines.push_back(headersLine);
  }
  this->parseHeaders(headersLines);
  // 保存请求体
  std::ostringstream bodyStream;
  bodyStream << requestStream.rdbuf();
  this->body_ = bodyStream.str();
  return true;
}

HttpRequest::METHOD HttpRequest::stringToMethod(const std::string& methodStr) {
  static const std::unordered_map<std::string, HttpRequest::METHOD> methodMap =
      {{"GET", METHOD::GET},         {"HEAD", METHOD::HEAD},
       {"POST", METHOD::POST},       {"PUT", METHOD::PUT},
       {"DELETE", METHOD::DELETE},   {"CONNECT", METHOD::CONNECT},
       {"OPTIONS", METHOD::OPTIONS}, {"TRACE", METHOD::TRACE}};
  auto it = methodMap.find(methodStr);
  if (it != methodMap.end()) {
    return it->second;
  }
  return METHOD::UNKNOWN;  // 未知请求方法
}

std::string HttpRequest::urlDecode(const std::string& str) {
  // user%20name=Tom%20Lee
  std::ostringstream decode;
  for (size_t i = 0; i < str.length(); ++i) {
    char ch = str[i];
    if (ch == '%') {
      // URL中编码必须是两位十六进制数
      if (i + 2 < str.length()) {
        std::string hex = str.substr(i + 1, 2);
        // 判断两位字符是否是合法的十六进制数(0~9A~Fa~f)
        if (isxdigit(hex[0]) && isxdigit(hex[1])) {
          // 将两位字符转换成对应的字符
          int value = std::stoi(hex, nullptr, 16);
          decode << static_cast<char>(value);
          i += 2;
        } else {
          decode << '%' << hex;  // 非法转义，保留原来的'%'
        }
      } else {
        decode << '%';  // 末尾不完整转义。保留原字符
      }
    } else if (ch == '+') {
      decode << ' ';  // URL编码中，'+'表示空格
    } else {
      decode << ch;
    }
  }
  return decode.str();
}

std::string HttpRequest::trim(const std::string& str) {
  // 去掉开头空格
  size_t start = str.find_first_not_of(' ');
  if (start == std::string::npos) {
    return std::string();  // 全是空格
  }

  // 去掉结尾空格
  size_t end = str.find_last_not_of(' ');
  return str.substr(start, end - start + 1);
}

bool HttpRequest::parseRequestLine(const std::string& line) {
  // HTTP请求行示例：GET /path/to/page?user=tom&id=123 HTTP/1.1
  std::istringstream lineStream(line);

  // 提取请求方法
  std::string methodStr;
  lineStream >> methodStr;
  if (methodStr.empty()) {
    return false;  // 请求方法为空
  }
  method_ = this->stringToMethod(methodStr);
  if (method_ == METHOD::UNKNOWN) {
    return false;  // 不支持的请求方法
  }

  // 提取url(包含查询参数)
  std::string url;
  lineStream >> url;
  if (url.empty()) {
    return false;  // 请求路径为空
  }

  auto mark = url.find('?');
  if (mark != std::string::npos) {  // 存在查询参数
    // 分离查询路径
    this->url_ = url.substr(0, mark);
    // 分离查询参数
    std::string queryParamsStr = url.substr(mark + 1);
    this->parseParameters(queryParamsStr);
  } else {
    this->url_ = url;
  }

  // 提取协议版本
  std::string version;
  lineStream >> version;
  this->version_ = version;
  if (version.empty()) {
    return false;  // 协议版本为空
  }
  return true;
}

void HttpRequest::parseParameters(const std::string& paramStr) {
  // 查询参数示例："user=tom&id=123&age=18"

  std::istringstream paramStream(paramStr);
  std::string param;

  while (std::getline(paramStream, param, '&')) {
    if (param.empty()) {  // 读取完毕
      break;
    }
    auto equal = param.find('=');
    if (equal == std::string::npos) {  // 非法参数，"user=tom&id&age=18"
      continue;
    }
    std::string key = this->urlDecode(param.substr(0, equal));
    std::string value = this->urlDecode(param.substr(equal + 1));
    if (value.empty()) {  // 系统设计：不保留 value 为空的查询参数
      continue;
    }
    this->params_.insert({std::move(key), std::move(value)});
  }
}

void HttpRequest::parseHeaders(const std::vector<std::string>& lines) {
  // "Host: www.example.com",
  // "User-Agent: curl/7.64.1",
  // "Accept: */*"
  for (const auto& it : lines) {
    auto colon = it.find(':');
    if (colon == std::string::npos) {
      continue;
    }
    std::string key = this->trim(it.substr(0, colon));
    std::string value = this->trim(it.substr(colon + 1));
    if (key.empty() || value.empty()) {
      continue;
    }
    headers_.insert({std::move(key), std::move(value)});
  }
}

const std::unordered_map<HttpRequest::HEADER, std::string>
    HttpRequest::header_map_ = {{HOST, "Host"},
                                {USER_AGENT, "User-Agent"},
                                {ACCEPT, "Accept"},
                                {ACCEPT_LANGUAGE, "Accept-Language"},
                                {ACCEPT_ENCODING, "Accept-Encoding"},
                                {CONNECTION, "Connection"},
                                {CONTENT_TYPE, "Content-Type"},
                                {CONTENT_LENGTH, "Content-Length"},
                                {COOKIE, "Cookie"},
                                {AUTHORIZATION, "Authorization"}};