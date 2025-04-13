#include "Filer.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "Logger.h"

bool Filer::fileExists(const std::string& filePath) {
  if (!std::filesystem::exists(filePath) ||
      !std::filesystem::is_regular_file(filePath)) {
    warn("[Filer] No such file: %s", getAbsolutePath(filePath).c_str());
    return false;
  }
  return true;
}

const std::string Filer::readFile(const std::string& filePath) {
  static const std::string emptyStream;
  std::ifstream filein(filePath, std::ios::in | std::ios::binary);
  if (filein.fail()) {
    warn("[Filer] No such file: %s", getAbsolutePath(filePath).c_str());
    return emptyStream;
  }
  std::ostringstream fileStream;
  fileStream << filein.rdbuf();
  return fileStream.str();
}

bool Filer::writeFile(const std::string& filePath, const std::string& buffer) {
  std::ofstream fileout(filePath, std::ios::app);
  if (fileout.fail()) {
    warn("[Filer] No such file: %s", getAbsolutePath(filePath).c_str());
    return false;
  }
  fileout << buffer << "\n";
  return true;
}

std::string Filer::getExtension(const std::string& filePath) {
  return std::filesystem::path(filePath).extension().string();
}

std::string Filer::getAbsolutePath(const std::string& relativePath) {
  static const std::string emptyString;
  try {
    return std::filesystem::absolute(relativePath).lexically_normal().string();
  } catch (const std::exception& e) {
    error("[Filer] getAbsolutePath exception: %s", e.what());
    return emptyString;
  }
}

const std::string& Filer::getMimeType(const std::string& extension) {
  static const std::unordered_map<std::string, std::string> mimeTypeMap = {
      {".html", "text/html"},        {".htm", "text/html"},
      {".css", "text/css"},          {".js", "text/javascript"},
      {".json", "application/json"}, {".xml", "application/xml"},
      {".txt", "text/plain"},        {".png", "image/png"},
      {".jpg", "image/jpeg"},        {".jpeg", "image/jpeg"},
      {".gif", "image/gif"},         {".pdf", "application/pdf"},
      {".zip", "application/zip"},   {".tar", "application/x-tar"},
      {".gz", "application/gzip"},   {".mp3", "audio/mpeg"},
      {".wav", "audio/wav"},         {".mp4", "video/mp4"},
      {".avi", "video/x-msvideo"},   {".bmp", "image/bmp"},
      {".svg", "image/svg+xml"},     {".ico", "image/vnd.microsoft.icon"},
      {".webp", "image/webp"}};
  static const std::string otherType = "application/octet-stream";
  auto mime = mimeTypeMap.find(extension);
  if (mime == mimeTypeMap.end()) {
    return otherType;
  }
  return mime->second;
}

const std::string& Filer::getMimeTypeFromFilePath(const std::string& filePath) {
  return getMimeType(getExtension(filePath));
}
