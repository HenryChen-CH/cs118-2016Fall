#include <string>
#include <stdlib.h>
#include <iostream>
#include <sys/select.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <chrono>
#include <ctime>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "HttpRequest.h"
#include "HttpResponse.h"

#define SERVER_DEBUG 1

using namespace std;

int fileSize(string file);

int main(int argc, char* argv[]) {
    string host = "localhost";
    int port = 4000;
    string dir = ".";

    if (argc != 4) {
        if (argc != 1) {
            cerr << "Parameter Error \n";
            return 1;
        }
    }
    if (argc == 4) {
        host = argv[1];
        port = atoi(argv[2]);
        dir = argv[3];
        if (dir.back() == '/') dir.pop_back();
    }

    int maxFd = 0;
    fd_set watchFds, readFds, errFds;
    FD_ZERO(&watchFds);
    FD_ZERO(&readFds);
    FD_ZERO(&errFds);

    int err = 0;
    char buf[100];
    // ssize_t  len = 0;
    addrinfo hints, *res0;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    err = getaddrinfo(host.data(), to_string(port).data(), &hints, &res0);
    if (err < 0) {
        cerr << "getaddrinfo error \n";
        return 2;
    }

    int listenFd = socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol);
    maxFd = listenFd;
    FD_SET(listenFd, &watchFds);

    int yes = 1;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        cerr << "setsockopt \n";
        return 3;
    }
    err = ::bind(listenFd, res0->ai_addr, res0->ai_addrlen);
    if (err < 0) {
        cerr << "bind error \n";
        return 4;
    }

    err = listen(listenFd, 10);
    if (err < 0) {
        cerr << "listen err \n";
        return 5;
    }

    struct timeval tv;
    while (1) {
        readFds = watchFds;
        errFds = watchFds;
        int nReady = 0;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        nReady = select(maxFd+1, &readFds, NULL, &errFds, &tv);
        if (nReady < 0) {
            cerr << "select error \n";
            return 6;
        }
        if (nReady > 0) {
            for (int fd = 0; fd <= maxFd; fd++) {
                if (FD_ISSET(fd, &readFds)) {
                    if (fd == listenFd) {
                        // listen socket
                        struct sockaddr_in clientAddr;
                        socklen_t clientSockLen = sizeof(clientAddr);
                        int clientFd = accept(fd, (struct sockaddr*)&clientAddr, &clientSockLen);
                        if (clientFd < 0) {
                            cerr << "accept error\n";
                            return 7;
                        }
                        if (maxFd < clientFd) maxFd = clientFd;
                        FD_SET(clientFd, &watchFds);
                    } else {
                        // normal packets
                        int recLen = 0;
                        memset(buf, 0, sizeof(buf));
                        stringstream ss;
                        while (1) {
                            recLen = recv(fd, buf, 100, 0);
                            if (SERVER_DEBUG) cout << buf;
                            if (recLen < 0) {
                                cerr << "recv error\n";
                                return 8;
                            }
                            if (recLen == 0) {
                                close(fd);
                                FD_CLR(fd, &watchFds);
                                break;
                            }
                            for (int i = 0; i < recLen; i++) {
                                ss << buf[i];
                            }
                            size_t index = ss.str().find("\r\n\r\n");
                            if (index != string::npos) break;
                        }
                        HttpRequest request;
                        HttpResponse response;
                        time_t cur_time= chrono::system_clock::to_time_t(chrono::system_clock::now());
                        string date = ctime(&cur_time);
                        response.setResponseTime(date.substr(0,date.length()-1));
                        int filesize = 0;
                        string path;
                        if(request.parse(ss.str())) {
                            path = request.getPath();
                            if (path[path.size()-1] == '/') path += "index.html";
                            path = dir+path;
                            if (access(path.data(), R_OK) == 0) {
                                response.setResponseCode(200);
                                filesize = fileSize(path);
                                response.setContentLength(filesize);
                            } else {
                                response.setResponseCode(404);
                                response.setContentLength(0);
                            }

                        } else {
                            if (SERVER_DEBUG) cout << "Bad Request\n";
                            response.setResponseCode(400);
                            response.setContentLength(0);
                        }
                        string data = response.encodeHead();
                        err = send(fd, data.data(), data.size(), 0);
                        if (err < 0) {
                            cerr << "send error\n";
                            return 9;
                        }
                        if (filesize > 0) {
                            cout << path << "\n";
                            int fileFd = open(path.data(), O_RDONLY, 0);
                            if (fileFd < 0) {
                                cerr << "open error\n";
                                return 9;
                            }
                            while (1) {
                                int len1 = read(fileFd, buf, 100);
                                if (len1 < 0) {
                                    cerr << "read error\n";
                                    return 10;
                                }
                                if (len1 == 0) break;
                                int len2 = send(fd, buf, len1, 0644);
                                if (len2 < 0) {
                                    cerr << "send error\n";
                                    return 11;
                                }
                                if (len2 == 0) break;
                            }
                            close(fileFd);
                        }
                        close(fd);
                        FD_CLR(fd, &watchFds);
                    }
                }
            }
        }
    }

}

int fileSize(string file) {
    struct stat st;
    stat(file.data(), &st);
    return st.st_size;
}
