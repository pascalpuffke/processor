set(CPM_DOWNLOAD_VERSION 0.39.0)
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
if (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}")
    file(DOWNLOAD https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()
include(${CPM_DOWNLOAD_LOCATION})

CPMAddPackage(NAME fmt VERSION 10.0.0 GITHUB_REPOSITORY fmtlib/fmt GIT_TAG 10.0.0)
CPMAddPackage(NAME cxxopts VERSION 3.2.0 GITHUB_REPOSITORY jarro2783/cxxopts GIT_TAG v3.2.0)
CPMAddPackage(NAME googletest VERSION 1.14.0 GITHUB_REPOSITORY google/googletest GIT_TAG v1.14.0)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/_deps/cxxopts-src/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/_deps/fmt-src/include)