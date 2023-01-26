cmake_minimum_required(VERSION 3.0)
project(cnow VERSION 1.0.0)

set(CNOW_VERSION ${CMAKE_PROJECT_VERSION})
configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_SOURCE_DIR}/include/config.h @ONLY) 

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE "Release")

if(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
    if (MINGW)
        add_compile_options("-finput-charset=utf-8")
        add_compile_options("-fexec-charset=gbk")
    endif()
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)
aux_source_directory(src SRC_LIST)
add_executable(cnow 
    ${SRC_LIST}
)

install(
    TARGETS cnow
)

if(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/source
        DESTINATION "/usr/local/share/cnow"
    )
endif()
