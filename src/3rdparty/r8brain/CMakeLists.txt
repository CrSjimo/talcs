project(r8brain VERSION 6.4 LANGUAGES CXX C)

file(GLOB_RECURSE _src *.h *.cpp *.c)

add_library(r8brain STATIC ${_src})

target_include_directories(r8brain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
