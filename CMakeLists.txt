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

add_subdirectory(test)