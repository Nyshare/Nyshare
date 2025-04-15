#include "HttpRequest.h"

#include <sstream>

HttpRequest::HttpRequest() : _method(UNKNOWN) {}

bool HttpRequest::parse(const std::string& rawRequest) {
  std::istringstream requestStream(rawRequest);

  // 解析请求头
  std::string requestLine;
  std::getline(requestStream, requestLine);  // 读取请求行
  if (requestLine.empty()) {
    return false;
  }
  this->parseRequestLine(requestLine);  // 解析请求行

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
  this->_body = bodyStream.str();
  return true;
}

HttpRequest::Method HttpRequest::getMethod() const { return _method; }

const std::string& HttpRequest::methodToString() const {
  static const std::unordered_map<HttpRequest::Method, std::string>
      methodtoStringMap = {
          {Method::GET, "GET"},         {Method::HEAD, "HEAD"},
          {Method::POST, "POST"},       {Method::PUT, "PUT"},
          {Method::DELETE, "DELETE"},   {Method::CONNECT, "CONNECT"},
          {Method::OPTIONS, "OPTIONS"}, {Method::TRACE, "TRACE"}};
  return methodtoStringMap.at(_method);
}

const std::string& HttpRequest::getPath() const { return _path; }

const std::unordered_map<std::string, std::string>&
HttpRequest::getQueryParams() const {
  return _queryParams;
}

const std::string& HttpRequest::getVersion() const { return _version; }

const std::unordered_map<std::string, std::string>& HttpRequest::getHeaders()
    const {
  return _headers;
}

const std::string& HttpRequest::getBody() const { return _body; }

const std::string& HttpRequest::getQueryParam(const std::string& key) const {
  static const std::string emptyString;
  auto it = _queryParams.find(key);
  return ((it != _queryParams.end()) ? it->second : emptyString);
}

const std::string& HttpRequest::getHeader(const std::string& key) const {
  static const std::string emptyString;
  auto it = _headers.find(key);
  return ((it != _headers.end()) ? it->second : emptyString);
}

bool HttpRequest::hasHeader(const std::string& key) const {
  auto it = _headers.find(key);
  return it != _headers.end();
}

HttpRequest::Method HttpRequest::parseMethod(const std::string& methodStr) {
  // UNKNOWN,GET,HEAD,POST,PUT,DELETE,CONNECT,OPTIONS,TRACE
  static const std::unordered_map<std::string, HttpRequest::Method> methodMap =
      {{"GET", Method::GET},         {"HEAD", Method::HEAD},
       {"POST", Method::POST},       {"PUT", Method::PUT},
       {"DELETE", Method::DELETE},   {"CONNECT", Method::CONNECT},
       {"OPTIONS", Method::OPTIONS}, {"TRACE", Method::TRACE}};
  auto it = methodMap.find(methodStr);
  if (it != methodMap.end()) {
    return it->second;
  }
  return Method::UNKNOWN;
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

void HttpRequest::parseRequestLine(const std::string& line) {
  // HTTP请求行示例：GET /path/to/page?user=tom&id=123 HTTP/1.1
  std::istringstream lineStream(line);

  // 提取请求方法
  std::string methodStr;
  lineStream >> methodStr;
  this->_method = this->parseMethod(methodStr);

  // 提取url(包含查询参数)
  std::string url;
  lineStream >> url;

  auto mark = url.find('?');
  if (mark != std::string::npos) {  // 存在查询参数
    // 分离查询路径
    this->_path = url.substr(0, mark);
    // 分离查询参数
    std::string queryParamsStr = url.substr(mark + 1);
    this->parseParameters(queryParamsStr);
  } else {
    this->_path = url;
  }

  // 提取协议版本
  std::string version;
  lineStream >> version;
  this->_version = version;
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
    this->_queryParams.insert({std::move(key), std::move(value)});
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
    _headers.insert({std::move(key), std::move(value)});
  }
}