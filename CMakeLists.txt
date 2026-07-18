cmake_minimum_required(VERSION 3.16)

project(igcl-rest-handler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

include(cmake/CPM.cmake)

CPMAddPackage(
    NAME igcl
    GITHUB_REPOSITORY intel/drivers.gpu.control-library
    GIT_TAG master
)

CPMAddPackage(
    NAME magic_enum
    GITHUB_REPOSITORY Neargye/magic_enum
    GIT_TAG v0.9.8
)

qt_standard_project_setup()

qt_add_executable(igcl-rest-handler
    main.cpp
)

target_link_libraries(igcl-rest-handler
    PRIVATE
        magic_enum::magic_enum
)

install(TARGETS igcl-rest-handler
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
