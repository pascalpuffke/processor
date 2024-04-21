cmake_minimum_required(VERSION 3.20)
project(processor LANGUAGES CXX)

set(WARN_FLAGS -Wall -Wextra -Wshadow -Wold-style-cast -Woverloaded-virtual -Wconversion -Wsign-conversion -Wimplicit-fallthrough)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/options.cmake)
include(cmake/packages.cmake)

set(SOURCES
    src/main.cpp
    src/assembler.cpp
    src/disassembler.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} fmt::fmt)
target_compile_options(
    ${PROJECT_NAME}
    PRIVATE
    ${WARN_FLAGS}
    ${ADDITIONAL_OPTIONS}
    -fdiagnostics-color=always
)

# For some reason, building on Mac (with default AppleClang compiler) causes a seemingly infinite loop of
# the compiler/linker doing nothing useful other than keeping all cores loaded and eating up every byte of memory
# at the "linking gmock" phase
if (NOT APPLE)
    add_subdirectory(test)
endif ()
