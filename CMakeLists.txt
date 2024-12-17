#
#
# Tencent is pleased to support the open source community by making tRPC available.
#
# Copyright (C) 2024 THL A29 Limited, a Tencent company.
# All rights reserved.
#
# If you have downloaded a copy of the tRPC source code from Tencent,
# please note that tRPC source code is licensed under the GNU General Public License Version 2.0 (GPLv2),
# A copy of the GPLv2 License is included in this file.
#
#

cmake_minimum_required(VERSION 3.14)

project(trpc-cpp-database)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("Cloning trpc-cpp, please waiting...")
include(com_github_trpc_cpp)

message("Fetching mysql libs, please waiting...")
include(mysqlclient)

file(GLOB_RECURSE SRC_FILES ./trpc/client/*.cc)

file(GLOB_RECURSE TEST_FILES ./trpc/client/*test.cc)

list(REMOVE_ITEM SRC_FILES ${TEST_FILES})

add_library(trpc_cpp_database_mysql
    ${SRC_FILES}
)

target_link_libraries(
    trpc_cpp_database_mysql PRIVATE
    ${TARGET_LINK_LIBS}
)

target_include_directories(
    trpc_cpp_database_mysql PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TARGET_INCLUDE_PATHS}
)