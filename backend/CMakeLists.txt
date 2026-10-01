cmake_minimum_required(VERSION 3.16)
project(MediPriority LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# ----------------------------------------------------------------------------
# C algorithm library (Phase 2): heap, queue, hash table, linked list, graph
# ----------------------------------------------------------------------------
add_library(medialgo STATIC
    c/src/heap.c
    c/src/queue.c
    c/src/hashtable.c
    c/src/linkedlist.c
    c/src/graph.c
    c/src/dijkstra.c
)
target_include_directories(medialgo PUBLIC c/include)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(medialgo PRIVATE -Wall -Wextra)
endif()

# ----------------------------------------------------------------------------
# C++ backend (Phases 3-5): models, utils, repositories, services, controllers
# ----------------------------------------------------------------------------
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# MySQL Connector/C++ (classic JDBC-style API) -- not always shipped with a
# CMake config, so we locate it manually. On Windows with the MySQL
# Installer, set MYSQLCPPCONN_INCLUDE_DIR / MYSQLCPPCONN_LIB_DIR if this
# auto-detection does not find it (see README.md Troubleshooting section).
find_path(MYSQLCPPCONN_INCLUDE_DIR mysql_connection.h
    PATHS /usr/include /usr/local/include
          "C:/Program Files/MySQL/MySQL Connector C++ 8.0/include"
)
find_library(MYSQLCPPCONN_LIB
    NAMES mysqlcppconn
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
          "C:/Program Files/MySQL/MySQL Connector C++ 8.0/lib64"
          "C:/Program Files/MySQL/MySQL Connector C++ 8.0/lib"
)

if(NOT MYSQLCPPCONN_INCLUDE_DIR OR NOT MYSQLCPPCONN_LIB)
    message(FATAL_ERROR
        "MySQL Connector/C++ not found. Install it (see README.md) or set "
        "MYSQLCPPCONN_INCLUDE_DIR and MYSQLCPPCONN_LIB manually with -D.")
endif()

file(GLOB_RECURSE MEDIPRIORITY_CPP_SOURCES CONFIGURE_DEPENDS
    cpp/utils/*.cpp
    cpp/repositories/*.cpp
    cpp/services/*.cpp
)

add_executable(medipriority_server
    cpp/main.cpp
    ${MEDIPRIORITY_CPP_SOURCES}
)

target_include_directories(medipriority_server PRIVATE
    cpp
    third_party
    ${MYSQLCPPCONN_INCLUDE_DIR}
)

target_link_libraries(medipriority_server PRIVATE
    medialgo
    ${MYSQLCPPCONN_LIB}
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(medipriority_server PRIVATE -Wall -Wextra)
endif()

if(WIN32)
    target_link_libraries(medipriority_server PRIVATE ws2_32 wsock32)
endif()

# ----------------------------------------------------------------------------
# C module tests (Phase 2) -- optional, built alongside the server
# ----------------------------------------------------------------------------
enable_testing()
foreach(test_name heap queue hashtable linkedlist graph)
    add_executable(test_${test_name} c/tests/test_${test_name}.c)
    target_link_libraries(test_${test_name} PRIVATE medialgo m)
    add_test(NAME test_${test_name} COMMAND test_${test_name})
endforeach()
