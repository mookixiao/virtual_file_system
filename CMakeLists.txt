cmake_minimum_required(VERSION 3.16)
project(virtual_file_system C)

set(CMAKE_C_STANDARD 99)

# 格式化文件系统
add_executable(moo_mkfs moo_mkfs.c)

# 展示文件系统信息
add_executable(moo_fsinfo moo_fsinfo.c)

# 文件系统浏览器
add_executable(moo_exp moo_exp.c utils/moo_fs.h utils/moo_utils.h utils/moo_log.c)

add_subdirectory(utils)

target_link_libraries(moo_exp PRIVATE utils m)
target_include_directories(moo_exp PRIVATE ${PROJECT_BINARY_DIR} utils)