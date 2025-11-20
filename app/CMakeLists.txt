set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(RESOURCES "${SRC_DIR}/resource/scrDbg.rc")

file(GLOB_RECURSE SRC_FILES
    "${SRC_DIR}/**.hpp"
    "${SRC_DIR}/**.cpp"
)

add_executable(scrDbgApp WIN32 ${SRC_FILES} ${RESOURCES})

set_target_properties(scrDbgApp PROPERTIES OUTPUT_NAME "scrDbg")

target_precompile_headers(scrDbgApp PUBLIC "${SRC_DIR}/Common.hpp")

target_compile_definitions(scrDbgApp PRIVATE
    _CRT_SECURE_NO_WARNINGS
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_include_directories(scrDbgApp PRIVATE
    "${SRC_DIR}"
)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Widgets Gui)

target_link_libraries(scrDbgApp PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)