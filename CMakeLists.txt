cmake_minimum_required(VERSION 3.16)

project(igcl-rest-handler LANGUAGES CXX)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING "The IGCL requires 64-bit for Performance & Telemetry API's. This build will probably not work properly.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

# If using Qt6, override DEFAULT_MAJOR_VERSION
set(QT_DEFAULT_MAJOR_VERSION 6 CACHE STRING "Qt version to use, defaults to 6")
set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME singleapplication
    GITHUB_REPOSITORY itay-grudev/SingleApplication
    GIT_TAG v3.5.6
)

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

qt_add_executable(igcl-rest-handler WIN32
    src/main.cpp
    igcl/igcl_api.h                 igcl/cApiWrapper.cpp
    src/http_server.hpp             src/http/http_server.cpp        src/http/result_enum_cast.cpp
    src/device/device.hpp           src/device/device_handle.cpp
    src/device/temp/temp.hpp        src/device/temp/temp_handle.cpp
    src/device/fan/fan.hpp          src/device/fan/fan_handle.cpp
    src/device/freq/freq.hpp        src/device/freq/freq_handle.cpp
    src/device/power/power.hpp      src/device/power/power_handle.cpp
    src/device/memory/memory.hpp    src/device/memory/memory_handle.cpp
    resources/resources.qrc         resources/images/app.rc
)

target_link_libraries(igcl-rest-handler PRIVATE
    Qt6::Widgets
    magic_enum::magic_enum
    Boost::beast
    Boost::json
    SingleApplication::SingleApplication
    ws2_32
    mswsock
)

if(WIN32)
    get_target_property(QT_CORE_DLL Qt6::Core IMPORTED_LOCATION_RELEASE)
    if(NOT QT_CORE_DLL)
        get_target_property(QT_CORE_DLL Qt6::Core IMPORTED_LOCATION)
    endif()

    get_filename_component(QT_BIN_DIR "${QT_CORE_DLL}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE NAMES windeployqt windeployqt6 HINTS "${QT_BIN_DIR}")

    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET igcl-rest-handler POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-translations --compiler-runtime $<TARGET_FILE:igcl-rest-handler>
            COMMENT "Deploying Qt runtime next to igcl-rest-handler"
            VERBATIM
        )
    else()
        message(WARNING "windeployqt was not found, so the executable may require Qt to be on PATH at runtime")
    endif()
else()
    message(FATAL_ERROR "unsupported platform")
endif()

install(TARGETS igcl-rest-handler
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
