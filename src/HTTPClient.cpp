#include "HTTPClient.h"

HTTPClientWrapper::HTTPClientWrapper(int timeoutMs) : timeout(timeoutMs) {
    setDefaultHeader("Content-Type", "application/json");
    setDefaultHeader("User-Agent", "LineaMicraController/1.0");
}

HTTPClientWrapper::~HTTPClientWrapper() {
    client.end();
}

void HTTPClientWrapper::setDefaultHeader(const String& name, const String& value) {
    defaultHeaders[name] = value;
}

void HTTPClientWrapper::removeDefaultHeader(const String& name) {
    defaultHeaders.erase(name);
}

void HTTPClientWrapper::setTimeout(int timeoutMs) {
    timeout = timeoutMs;
}

HTTPResponse HTTPClientWrapper::request(HTTPMethod method, const String& url, const String& body) {
    HTTPResponse response;
    
    if (!client.begin(url)) {
        Serial.println("Failed to initialize HTTP client for URL: " + url);
        response.statusCode = -1;
        response.body = "Failed to initialize HTTP client";
        return response;
    }
    
    client.setTimeout(timeout);
    applyHeaders();
    
    int httpResponseCode;
    switch (method) {
        case HTTPMethod::GET:
            httpResponseCode = client.GET();
            break;
        case HTTPMethod::POST:
            httpResponseCode = client.POST(body);
            break;
        case HTTPMethod::PUT:
            httpResponseCode = client.PUT(body);
            break;
        case HTTPMethod::DELETE:
            httpResponseCode = client.sendRequest("DELETE", body);
            break;
        default:
            httpResponseCode = -1;
            break;
    }
    
    response.statusCode = httpResponseCode;
    
    if (httpResponseCode > 0) {
        response.body = client.getString();
        Serial.println("HTTP " + methodToString(method) + " " + url + " -> " + String(httpResponseCode));
        // Log response body only in debug builds
        #ifdef DEBUG
        Serial.println("Response: " + response.body);
        #endif
    } else {
        response.body = "HTTP request failed with error: " + String(httpResponseCode);
        Serial.println("HTTP request failed: " + response.body);
    }
    
    client.end();
    return response;
}

HTTPResponse HTTPClientWrapper::request(HTTPMethod method, const String& url, const JsonDocument& jsonBody) {
    String body;
    serializeJson(jsonBody, body);
    return request(method, url, body);
}

HTTPResponse HTTPClientWrapper::get(const String& url) {
    return request(HTTPMethod::GET, url);
}

HTTPResponse HTTPClientWrapper::post(const String& url, const String& body) {
    return request(HTTPMethod::POST, url, body);
}

HTTPResponse HTTPClientWrapper::post(const String& url, const JsonDocument& jsonBody) {
    return request(HTTPMethod::POST, url, jsonBody);
}

void HTTPClientWrapper::applyHeaders() {
    for (const auto& header : defaultHeaders) {
        client.addHeader(header.first, header.second);
    }
}

String HTTPClientWrapper::methodToString(HTTPMethod method) {
    switch (method) {
        case HTTPMethod::GET: return "GET";
        case HTTPMethod::POST: return "POST";
        case HTTPMethod::PUT: return "PUT";
        case HTTPMethod::DELETE: return "DELETE";
        default: return "UNKNOWN";
    }
}