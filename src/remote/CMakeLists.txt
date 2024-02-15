project(TalcsRemote VERSION ${TALCS_VERSION} LANGUAGES CXX)

file(GLOB_RECURSE _src *.h *.cpp)

find_package(Boost REQUIRED)
find_package(rpclib REQUIRED)

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Device rpclib::rpc
    LINKS_PRIVATE Boost::boost
    QT_INCLUDE_PRIVATE Core
)
