#pragma once

#include <string>

class Filer {
 public:
  // 判断文件是否存在
  static bool fileExists(const std::string &filePath);
  // 从文件中读取数据
  static const std::string readFile(const std::string &filePath);
  // 将数据写入文件
  static bool writeFile(const std::string &filePath, const std::string &buffer);
  // 获取文件拓展名
  static std::string getExtension(const std::string &filePath);
  // 获取文件的绝对路径
  static std::string getAbsolutePath(const std::string &relativePath);
  // 根据文件拓展名获取 MOME 类型
  static const std::string &getMimeType(const std::string &extension);
  // 直接从文件路径获取 MIME 类型
  static const std::string &getMimeTypeFromFilePath(
      const std::string &filePath);

 private:
  Filer();
  Filer(const Filer &obj) = delete;
  Filer &operator=(const Filer &obj) = delete;
};