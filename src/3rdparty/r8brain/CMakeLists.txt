project(r8brain VERSION 6.4 LANGUAGES CXX C)

file(GLOB_RECURSE _src *.h *.cpp *.c)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_library(r8brain STATIC ${_src})

target_include_directories(r8brain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
