//
// Created by 陈昊 on 18/10/2016.
//
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <unordered_map>

class HttpResponse {
public:
    void parse(std::string str);
    void parseHeader(std::string header);
    int getResponseCode();
    std::string getResponseText();
    std::string getBody();
    int getContentLength();

    void setResponseCode(int code);
    void setHttpVersion(std::string version);
    void setContentLength(int len);
    void setResponseTime(std::string time);
    std::string encodeHead();
private:
    std::string header;
    std::string body;
    std::string responseDate;
    int responseCode = 0;
    std::string responseText;
    std::string version = "1.0";
    int contentLength = 0;
    std::unordered_map<std::string, std::string> headerMap;

    std::vector<std::string> split(const std::string& str, const std::string& delim);
    std::string responseCodeTranslate(int code);
};
