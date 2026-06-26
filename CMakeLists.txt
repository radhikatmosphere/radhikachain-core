cmake_minimum_required(VERSION 3.16)
project(radhikachain-core VERSION 
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "RadhikaChain Core - Proof of Karma L1 Blockchain"
)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build types
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_RADHIKAD "Build radhikad daemon" ON)
option(BUILD_RADHIKA_CLI "Build radhika-cli" ON)
option(BUILD_RADHIKA_TX "Build radhika-tx" ON)
option(BUILD_RADHIKA_WALLET "Build radhika-wallet" ON)
option(BUILD_RADHIKA_UTIL "Build radhika-util" ON)
option(ENABLE_TESTS "Build tests" ON)
option(ENABLE_BENCH "Build benchmarks" OFF)
option(ENABLE_ZMQ "Enable ZMQ notifications" ON)
option(ENABLE_BIP324 "Enable BIP324 P2P encryption" ON)
option(USE_ASAN "Enable AddressSanitizer" OFF)
option(USE_TSAN "Enable ThreadSanitizer" OFF)

# Find dependencies
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread chrono program_options test)
find_package(LibEvent REQUIRED)
find_package(ZMQ QUIET)
find_package(SQLite3 REQUIRED)
find_package(UniValue REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/leveldb/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/secp256k1/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/univalue/include
    ${OPENSSL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
    ${LIBEVENT_INCLUDE_DIRS}
    ${ZMQ_INCLUDE_DIRS}
    ${SQLITE3_INCLUDE_DIRS}
    ${UNIVALUE_INCLUDE_DIRS}
)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter")
if(USE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()
if(USE_TSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fPIE")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread -pie")
endif()

# Subdirectories
add_subdirectory(src/secp256k1)
add_subdirectory(src/univalue)
add_subdirectory(src/leveldb)
add_subdirectory(src)

# Install
install(PROGRAMS
    ${CMAKE_CURRENT_BINARY_DIR}/src/radhikad
    ${CMAKE_CURRENT_BINARY_DIR}/src/radhika-cli
    ${CMAKE_CURRENT_BINARY_DIR}/src/radhika-tx
    DESTINATION bin
)