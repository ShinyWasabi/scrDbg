include(cmake/minhook.cmake)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SHARED_DIR "${CMAKE_SOURCE_DIR}/src/shared")

set(RESOURCES "${SHARED_DIR}/scrDbgLib.rc")

file(GLOB_RECURSE SRC_FILES
    "${SRC_DIR}/**.hpp"
    "${SRC_DIR}/**.cpp"
)

file(GLOB_RECURSE SHARED_FILES
    "${SHARED_DIR}/**.hpp"
    "${SHARED_DIR}/**.cpp"
)

add_library(scrDbgLib MODULE ${SRC_FILES} ${SHARED_FILES} ${RESOURCES})

set_target_properties(scrDbgLib PROPERTIES OUTPUT_NAME "scrDbg")

target_precompile_headers(scrDbgLib PUBLIC "${SRC_DIR}/Common.hpp")

target_compile_definitions(scrDbgLib PRIVATE
    _CRT_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_include_directories(scrDbgLib PRIVATE
    "${SRC_DIR}"
    "${SHARED_DIR}"
    "${minhook_SOURCE_DIR}/include"
    "${minhook_SOURCE_DIR}/src/hde"
)

target_link_libraries(scrDbgLib PRIVATE
    minhook
)