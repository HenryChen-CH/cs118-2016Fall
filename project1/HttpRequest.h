//
// Created by 陈昊 on 17/10/2016.
//

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>
#define HTTPREQUEST_DEBUG 0

/*
 * Function of this class is encoding the http given the header information
 */

class HttpRequest {
public:
    std::string path;
    std::string method;
    std::string host;
    std::string httpVersion;
    std::string connectionState;
    std::unordered_map<std::string, std::string> headerMap;

    void setUrl(std::string url);
    void setMethod(std::string method);
    void setHttpVersion(std::string version);
    void setHost(std::string host);
    void setConnectionState(std::string state);
    std::string encode();

    bool parse(std::string str);
    std::string getPath();
    std::string getVersion();
    std::string getConnectionState();

private:
    bool isValid;
    std::vector<std::string> split(const std::string& str, const std::string& delim);
};


