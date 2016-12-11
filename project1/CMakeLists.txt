cmake_minimum_required(VERSION 3.6)
project(cs118-proj1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES HttpRequest.cpp HttpRequest.h HttpResponse.cpp HttpResponse.h)
add_executable(cs118-proj1 ${SOURCE_FILES})