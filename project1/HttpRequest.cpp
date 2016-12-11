//
// Created by 陈昊 on 17/10/2016.
//

#include "HttpRequest.h"

void HttpRequest::setHttpVersion(std::string version) {
    this->httpVersion = version;
}

void HttpRequest::setMethod(std::string method) {
    this->method = method;
}

void HttpRequest::setUrl(std::string url) {
    this->path = url;
}

void HttpRequest::setHost(std::string host) {
    this->host = host;
}

void HttpRequest::setConnectionState(std::string state) {
    this->connectionState = state;
}

std::string HttpRequest::encode() {
    std::string http;
    if (method.size() == 0) method = "GET";
    http.append(method);
    http.append(" ");
    if (path.size() == 0) path = "/";
    http.append(path);
    http.append(" ");
    if (httpVersion.size() == 0) httpVersion = "1.0";
    http.append("HTTP/"+httpVersion+"\r\n");

    http.append("Host: "+host+"\r\n");
    if (connectionState.length() > 0) http.append("Connection: "+connectionState+"\r\n");
    http.append("Accept-language: fr\r\n");
    http.append("\r\n");

    if (HTTPREQUEST_DEBUG) std::cout << "Http Request:\n" << http <<"--------------\n";

    return http;
}

bool HttpRequest::parse(std::string str) {
    std::vector<std::string> lines = this->split(str, "\r\n");
    if (lines.size() == 0) return false;
    std::vector<std::string> components = this->split(lines[0], " ");
    if (components.size() != 3) return false;
    if (components[0] != "GET" || !(components[2] == "HTTP/1.1" || components[2] == "HTTP/1.0")) return false;
    headerMap.insert({{"method", components[0]}, {"path", components[1]}, {"version", components[2]}});

    for (size_t i = 1; i < lines.size(); i++) {
        if (lines[i].length() == 0) continue;
        std::vector<std::string> tmp = this->split(lines[i], ": ");
        if (tmp.size() != 2) return false;
        headerMap.insert({tmp[0], tmp[1]});
    }
    if (headerMap.find("Host") == headerMap.end()) return false;
    isValid = true;
    return true;
}

std::string HttpRequest::getPath() {
    if (isValid) return headerMap["path"];
    return "";
}

std::string HttpRequest::getVersion() {
    if (isValid) return headerMap["version"];
    return "";
}

std::string HttpRequest::getConnectionState() {
    if (isValid) return headerMap["Connection"];
    return "";
}

std::vector<std::string> HttpRequest::split(const std::string& str, const std::string& delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}
