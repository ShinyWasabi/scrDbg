cmake_minimum_required(VERSION 3.20)

project(scrDbg CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")

set(RESOURCES "${SRC_DIR}/scrDbg.rc")

file(GLOB_RECURSE SRC_FILES
    "${SRC_DIR}/**.hpp"
    "${SRC_DIR}/**.cpp"
)

add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES} ${RESOURCES})

target_precompile_headers(${PROJECT_NAME} PUBLIC "${SRC_DIR}/Common.hpp")

add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${SRC_DIR}"
)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Widgets Gui)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)