#include "DatabaseUtil.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <iomanip>
#include <sstream>

#include "Logger.h"

std::string DatabaseUtil::encryptPassword(const std::string& password) {
  const int saltLen = 16;
  const int hashLen = 32;
  const int iterations = 100000;

  unsigned char salt[saltLen];
  if (!RAND_bytes(salt, saltLen))
    throw std::runtime_error("Failed to generate salt");

  unsigned char hash[hashLen];
  PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt, saltLen,
                    iterations, EVP_sha256(), hashLen, hash);

  return toHex(salt, saltLen) + ":" + toHex(hash, hashLen);
}

bool DatabaseUtil::checkPassword(const std::string& password,
                                 const std::string& stored) {
  const int saltLen = 16;
  const int hashLen = 32;
  const int iterations = 100000;

  size_t sep = stored.find(':');
  if (sep == std::string::npos) return false;

  std::vector<unsigned char> salt = fromHex(stored.substr(0, sep));
  std::vector<unsigned char> hashExpected = fromHex(stored.substr(sep + 1));

  if (salt.size() != saltLen || hashExpected.size() != hashLen) return false;

  unsigned char hashActual[hashLen];
  PKCS5_PBKDF2_HMAC(password.c_str(), password.size(), salt.data(), salt.size(),
                    iterations, EVP_sha256(), hashLen, hashActual);

  return CRYPTO_memcmp(hashActual, hashExpected.data(), hashLen) == 0;
}

std::string DatabaseUtil::toHex(const unsigned char* data, size_t len) {
  std::ostringstream oss;
  for (size_t i = 0; i < len; ++i)
    oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
  return oss.str();
}

std::vector<unsigned char> DatabaseUtil::fromHex(const std::string& hex) {
  std::vector<unsigned char> out;
  for (size_t i = 0; i < hex.length(); i += 2) {
    std::string byteString = hex.substr(i, 2);
    unsigned char byte = (unsigned char)strtol(byteString.c_str(), nullptr, 16);
    out.push_back(byte);
  }
  return out;
}
