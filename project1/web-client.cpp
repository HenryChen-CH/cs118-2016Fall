#include <string.h>
#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>

#include "UriParser.hpp"
#include "HttpRequest.h"
#include "HttpResponse.h"

#define CLIENT_DEBUG 0

using namespace std;

string fileNameFromPath(string url);
void sendHttpRequest(string url);


int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        sendHttpRequest(argv[i]);
    }
}

void sendHttpRequest(string url) {
    http::url parsed = http::ParseHttpUrl(url);
    int error = 0;
    char buf[1024];
    ssize_t len = 0;

    addrinfo hints, *res0;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    string servname = parsed.port == 0 ? "http" : to_string(parsed.port);
    error = getaddrinfo(parsed.host.data(), servname.data(), &hints, &res0);
    if (error) {
        cerr << "getaddrinfo error\n";
        exit(1);
    }

    int sockfd = socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol);
    if (sockfd < 0) {
        cerr << "socket error\n";
        exit(2);
    }

    if (connect(sockfd, res0->ai_addr, res0->ai_addrlen) == -1) {
        cerr << "connect error\n";
        exit(3);
    }

    HttpRequest request;
    request.setHost(parsed.host);
    request.setMethod("GET");
    request.setUrl(parsed.path);
    string data = request.encode();
    if (send(sockfd, data.data(), data.size(), 0) == -1) {
        cerr << "send error\n";
        exit(4);
    }

    HttpResponse httpResponse;
    stringstream ss;
    int fileLen = -1;
    int contentLength = 0;
    while (true) {
        len = recv(sockfd, buf, 1024, 0);
        for (int i = 0; i < len; i++) {
            ss << buf[i];
            if (CLIENT_DEBUG) cout << buf[i];
        }
        if (fileLen >= 0) {
            fileLen += len;
        }
        if (CLIENT_DEBUG) cout << fileLen << ":" << contentLength << "\n";
        if (fileLen == contentLength) break;
        if (fileLen == -1) {
            ssize_t index = ss.str().find("\r\n\r\n");
            fileLen = ss.str().size()-index-4;
            httpResponse.parseHeader(ss.str().substr(0,index));
            contentLength = httpResponse.getContentLength();
        }
        if (len <= 0) break;
    }
    close(sockfd);
    httpResponse.parse(ss.str());
    cout << httpResponse.getResponseText() << "\n";
    if (CLIENT_DEBUG) cout << "Response Code: " << httpResponse.getResponseCode() << "\n";
    if (httpResponse.getResponseCode() == 200) {
        ofstream file;
        string filename = fileNameFromPath(parsed.path);
        if (filename.size() == 0) filename = "index.html";
        file.open(filename);
        file << httpResponse.getBody();
        if (CLIENT_DEBUG) cout << "Response Body:" << httpResponse.getBody() << "\n";
        file.close();
    }
    freeaddrinfo(res0);
}

string fileNameFromPath(string url) {
    size_t i = url.rfind('/', url.size()-1);
    if (i != string::npos) {
        string file = url.substr(i+1, url.size()-1);
        return file;
    }
    return "";
}
