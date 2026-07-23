cmake_minimum_required(VERSION 3.16)

project(igcl-rest-handler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME magic_enum
    GITHUB_REPOSITORY Neargye/magic_enum
    GIT_TAG v0.9.8
)

CPMAddPackage(
    NAME boost
    URL "https://github.com/boostorg/boost/releases/download/boost-1.91.0-1/boost-1.91.0-1-cmake.tar.gz"
)

qt_standard_project_setup()

qt_add_executable(igcl-rest-handler
    main.cpp
    igcl/igcl_api.h igcl/cApiWrapper.cpp
    http_server.hpp http_server.cpp
    device/device.hpp device/device_handle.cpp
    device/temp/temp.hpp device/temp/temp_handle.cpp
)

target_link_libraries(igcl-rest-handler
    PRIVATE
        magic_enum::magic_enum
        Boost::beast
        Boost::json
    ws2_32
    mswsock
)

install(TARGETS igcl-rest-handler
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
