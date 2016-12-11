//
// Created by 陈昊 on 18/10/2016.
//

#include "HttpResponse.h"

void HttpResponse::parse(std::string str) {
    size_t i = str.find("\r\n\r\n");
    if (i != std::string::npos) {
        header = str.substr(0, i);
        body = str.substr(i+4, str.size());
        this->parseHeader(header);
    }
}

std::string HttpResponse::getBody() {
    return body;
}

int HttpResponse::getResponseCode() {
    return responseCode;
}

std::string HttpResponse::getResponseText() {
    return responseText;
}

void HttpResponse::setResponseCode(int code) {
    this->responseCode = code;
}

void HttpResponse::setHttpVersion(std::string version) {
    this->version = version;
}

void HttpResponse::setContentLength(int len){
    this->contentLength = len;
}

void HttpResponse::setResponseTime(std::string time) {
    this->responseDate = time;
}

std::string HttpResponse::encodeHead() {
    std::stringstream ss;
    ss << "HTTP/" << version << " " << responseCode << " " << responseCodeTranslate(responseCode) << "\r\n";
    ss << "Connection: " << "keep-alive\r\n";
    ss << "Content-Length: " << contentLength << "\r\n";
    if (responseDate.length() > 0) ss << "Date: " << responseDate << "\r\n";
    ss << "\r\n";
    return ss.str();
}

int HttpResponse::getContentLength() {
    return atoi(headerMap["Content-Length"].data());
}

//

void HttpResponse::parseHeader(std::string header) {
    std::vector<std::string> lines = this->split(header, "\r\n");
    if (lines.size() == 0) return;
    size_t first = lines[0].find(' ', 0);
    if (first == std::string::npos) return;
    size_t second = lines[0].find(' ', first+1);
    responseCode = atoi(lines[0].substr(first+1, second).data());
    responseText = lines[0].substr(first+1, lines[0].size());
    for (size_t i = 1; i < lines.size(); i++) {
        std::vector<std::string> tmp = this->split(lines[i], ": ");
        if (tmp.size() != 2) continue;
        headerMap.insert({tmp[0], tmp[1]});
    }
}

std::vector<std::string> HttpResponse::split(const std::string& str, const std::string& delim)
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

std::string HttpResponse::responseCodeTranslate(int code) {
    switch (code) {
        case 200:
            return "OK";
        case 400:
            return "Bad Request";
        case 404:
            return "Not Found";
        default:
            return "";
    }
}