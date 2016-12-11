CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=cs118proj2

# Add all .cpp files that need to be compiled for your server
SERVER_FILES=server.cpp TCB.cpp TcpPacket.cpp

SERVERfr_FILES=serverfr.cpp TCBfr.cpp TcpPacket.cpp

# Add all .cpp files that need to be compiled for your client
CLIENT_FILES=client.cpp clientHelper.cpp TcpPacket.cpp

all: server client serverfr

*.o: *.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

serverfr: $(SERVERfr_FILES:.cpp=.o)
	$(CXX) -o $@ $(CXXFLAGS) $(SERVERfr_FILES:.cpp=.o)

server: $(SERVER_FILES:.cpp=.o)
	$(CXX) -o $@ $(CXXFLAGS) $(SERVER_FILES:.cpp=.o)

client: $(CLIENT_FILES:.cpp=.o)
	$(CXX) -o $@ $(CXXFLAGS) $(CLIENT_FILES:.cpp=.o)

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM server serverfr client *.tar.gz

tarball: clean
	tar -cvf $(USERID).tar.gz *.h *.cpp Makefile Vagrantfile *.lua
