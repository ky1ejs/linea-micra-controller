#ifndef HTTP_CLIENT_WRAPPER_H
#define HTTP_CLIENT_WRAPPER_H

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include <map>

enum class HTTPMethod { GET, POST, PUT, DELETE };

struct HTTPResponse {
  int statusCode;
  String body;
  bool isSuccess() const {
    return statusCode >= 200 && statusCode < 300;
  }
};

class HTTPClientWrapper {
private:
  HTTPClient client;
  std::map<String, String> defaultHeaders;
  int timeout;

public:
  HTTPClientWrapper(int timeoutMs = 10000);
  ~HTTPClientWrapper();

  void setDefaultHeader(const String& name, const String& value);
  void removeDefaultHeader(const String& name);
  void setTimeout(int timeoutMs);

  HTTPResponse request(HTTPMethod method, const String& url, const String& body = "");
  HTTPResponse request(HTTPMethod method, const String& url, const JsonDocument& jsonBody);

  // Convenience methods
  HTTPResponse get(const String& url);
  HTTPResponse post(const String& url, const String& body = "");
  HTTPResponse post(const String& url, const JsonDocument& jsonBody);

private:
  void applyHeaders();
  String methodToString(HTTPMethod method);
};

#endif  // HTTP_CLIENT_WRAPPER_H