# 编译所需的cmake最低版本号
cmake_minimum_required(VERSION 3.10)
# 设置项目名和版本号
project(Calendar VERSION 1.1)

# 配置文件和config.h变量替换
set(CALENDAR_VERSION_MAJOR 1)
set(CALENDAR_VERSION_MINOR 1)
configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_SOURCE_DIR}/inc/config.h @ONLY) 

# 指定语言标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# 调试类型: 
# Debug(-g) | Release(-O3 -DNDEBUG) | RelWithDebInfo(-O2 -g -DNDEBUG) | MinSizeRel(-Os -DNDEBUG)
set(CMAKE_BUILD_TYPE "Release")

# 文件输出路径
# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 头文件包含路径
include_directories(${PROJECT_SOURCE_DIR}/inc)

# 获取某一目录下源文件列表，赋值给变量SRC_LIST
aux_source_directory(src SRC_LIST)

# 生成动态/静态库
# add_library(cale SHARED ${PROJECT_SOURCE_DIR}/src/calendar.c)
# add_library(cale ${PROJECT_SOURCE_DIR}/src/calendar.c)
# 生成可执行文件
add_executable(calendar ${SRC_LIST})

# 链接库
# target_link_libraries(calendar cale)

# 安装
# 安装目录前缀
# set(CMAKE_INSTALL_PREFIX /home/iheng/.ms)
# 可执行文件默认安装到bin目录，库文件默认安装到lib目录
install(
    TARGETS calendar
)
