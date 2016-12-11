#include <string>
#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>

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

string file_name_from_path(string url);
void sendHttpRequest(string url, int sockfd);
string read_line(int socket, int len);

int main(int argc, char* argv[]) {
    unordered_map<string, int> httpSocks;
    for (int i = 1; i < argc; i++) {
        string URL(argv[i]);
        http::url parsed = http::ParseHttpUrl(URL);
        string server = parsed.host+":"+to_string(parsed.port);
        if (httpSocks.find(server) == httpSocks.end()) {
            int error = 0;
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
            httpSocks.insert({{server, sockfd}});
        }
        sendHttpRequest(string(argv[i]), httpSocks.find(server)->second);
    }

    for (auto const&entry: httpSocks) {
        string key = entry.first;
        http::url parsed = http::ParseHttpUrl(key);
        HttpRequest request;
        request.setHost(parsed.host);
        request.setHttpVersion("1.1");
        request.setUrl("/");
        request.setConnectionState("close");
        string data = request.encode();
        if (send(entry.second, data.data(), data.size(), 0) == -1) {
            cerr << "send error\n";
            exit(4);
        }
        close(entry.second);
        if (CLIENT_DEBUG) cout << "close socket: " << entry.second << "\n";
    }
}

void sendHttpRequest(string url, int sockfd) {
    http::url parsed = http::ParseHttpUrl(url);
    int len = 0;
    HttpRequest request;
    request.setHost(parsed.host);
    request.setMethod("GET");
    request.setHttpVersion("1.1");
    request.setUrl(parsed.path);
    request.setConnectionState("keep-alive");
    string data = request.encode();
    if (send(sockfd, data.data(), data.size(), 0) == -1) {
        cerr << "send error\n";
        exit(4);
    }

    HttpResponse httpResponse;
    stringstream head;
    string line;
    bool receiveHeader = false;
    stringstream body;
    while (true) {
        if (!receiveHeader) {
            line = read_line(sockfd, -1);
            head << line;
            // cout << line;
            if (CLIENT_DEBUG) cout << line;
            if (line == "\r\n") {
                httpResponse.parseHeader(head.str());
                receiveHeader = true;
                if (httpResponse.getResponseCode() != 200) break;
            }
        }else {
            if (httpResponse.getContentLength() != 0) {
                //cout << "response length: " << httpResponse.getContentLength() << "\n";
                line = read_line(sockfd, httpResponse.getContentLength());
                body << line;
                // cout << "\n---------------\n";
                // cout << line;
                break;
            } else {
                char *end;
                line = read_line(sockfd, -1);
                if (CLIENT_DEBUG) cout << line;
                len = strtol(line.substr(0, line.length()-2).data(), &end, 16);
                line  = read_line(sockfd, len+2);
                if (CLIENT_DEBUG) cout << line;
                body << line.substr(0, line.length()-2);
                if (len == 0) break;
            }
        }
    }
    cout << "Response Code: " << httpResponse.getResponseCode() << "\n";
    if (httpResponse.getResponseCode() == 200) {
        ofstream file;
        string filename = file_name_from_path(parsed.path);
        if (filename.size() == 0) filename = "index.html";
        file.open(filename);
        file << body.str();
        if (CLIENT_DEBUG) cout << "Response Body:" << body.str() << "\n";
        file.close();
    }
}

string file_name_from_path(string url) {
    size_t i = url.rfind('/', url.size()-1);
    if (i != string::npos) {
        string file = url.substr(i+1, url.size()-1);
        return file;
    }
    return "";
}

string read_line(int socket, int len) {
    int error = 0;
    stringstream line;
    if (len < 0) {
        char buf;
        while (true) {
            buf = 0x00;
            error = recv(socket, &buf, 1, 0);
            if (error != 1) return "";
            line << buf;
            if (buf == '\n') break;
        }
    } else {
        char buf[1024];
        while (len > 1024) {
            error = recv(socket, buf, 1024, 0);
            if (error < 0) {
                cerr << "recv error\n";
                exit(1);
            }
            for (int i = 0; i < error; i++) line << buf[i];
            len -= error;
        }
        char tmp;
        while (true) {
            recv(socket, &tmp, 1, 0);
            line << tmp;
            len --;
            if (len == 0) break;
        }
    }

    return line.str();
}