cmake_minimum_required(VERSION 3.0)
project(Calendar VERSION 0.1.3)

set(CALENDAR_VERSION ${CMAKE_PROJECT_VERSION})
configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_SOURCE_DIR}/inc/config.h @ONLY) 

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
# Debug(-g) | Release(-O3 -DNDEBUG) | RelWithDebInfo(-O2 -g -DNDEBUG) | MinSizeRel(-Os -DNDEBUG)
set(CMAKE_BUILD_TYPE "Release")

include_directories(${PROJECT_SOURCE_DIR}/inc)
aux_source_directory(src SRC_LIST)
add_executable(calendar ${SRC_LIST})
install(
    TARGETS calendar
)
