cmake_minimum_required(VERSION 3.16)

project(igcl-rest-handler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

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
    src/main.cpp
    igcl/igcl_api.h             igcl/cApiWrapper.cpp
    src/http_server.hpp         src/http/http_server.cpp        src/http/result_enum_cast.cpp
    src/device/device.hpp       src/device/device_handle.cpp
    src/device/temp/temp.hpp    src/device/temp/temp_handle.cpp
    src/device/fan/fan.hpp      src/device/fan/fan_handle.cpp
    resources/resources.qrc
)

target_link_libraries(igcl-rest-handler
    PRIVATE
        Qt6::Widgets
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
