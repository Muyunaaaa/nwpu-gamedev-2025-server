cmake_minimum_required(VERSION 3.15)
project(thetaforce_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# ==============================
# 第三方库（submodules）
# ==============================

# EnTT（header-only，但自带 CMake）
add_subdirectory(vendors/entt)

# glm（header-only，不 add_subdirectory）
# concurrentqueue（header-only，不 add_subdirectory）

# nlohmann/json
add_subdirectory(vendors/nlohmann)

# flatbuffers
add_subdirectory(vendors/flatbuffers)

# JoltPhysics
add_subdirectory(vendors/JoltPhysics/Build)

# toml++
add_subdirectory(vendors/tomlplusplus)

# tinygltf
add_subdirectory(vendors/tinygltf)

# spdlog
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(vendors/spdlog)

# ==============================
# 资源复制（服务器运行需要）
# ==============================

file(COPY ${CMAKE_SOURCE_DIR}/config
     DESTINATION ${CMAKE_BINARY_DIR}/bin)

file(COPY ${CMAKE_SOURCE_DIR}/assets
     DESTINATION ${CMAKE_BINARY_DIR}/bin)

file(COPY ${CMAKE_SOURCE_DIR}/settings.toml
     DESTINATION ${CMAKE_BINARY_DIR}/bin)

# ==============================
# 服务器可执行程序
# ==============================

add_executable(server
        "include/stats/ComputeStats.h"
        src/stats/ComputeStats.cpp)

# ==============================
# 源文件
# ==============================

file(GLOB_RECURSE SERVER_SOURCES
    src/*.cpp
    include/*.h
    shared/src/*.cpp
    shared/include/*.h
    config/*.h
    config/*.cpp
    config/TomlImpl.hpp
)

target_sources(server PRIVATE ${SERVER_SOURCES})

# ==============================
# 头文件路径
# ==============================

target_include_directories(server PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/vendors/glm
    ${PROJECT_SOURCE_DIR}/vendors/concurrentqueue
    ${PROJECT_SOURCE_DIR}/vendors/flatbuffers/include
    ${PROJECT_SOURCE_DIR}/vendors/JoltPhysics
    ${PROJECT_SOURCE_DIR}/shared/include
    ${PROJECT_SOURCE_DIR}/config
    ${PROJECT_SOURCE_DIR}/generated
)

# ==============================
# ENet（Linux 系统库）
# ==============================

find_package(PkgConfig REQUIRED)
pkg_check_modules(ENET REQUIRED libenet)

target_include_directories(server PRIVATE ${ENET_INCLUDE_DIRS})
target_link_libraries(server PRIVATE ${ENET_LIBRARIES})

# ==============================
# 链接库
# ==============================

find_package(Threads REQUIRED)

target_link_libraries(server PRIVATE
    Threads::Threads
    EnTT::EnTT
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
    Jolt
    tomlplusplus::tomlplusplus
    tinygltf
    enet
    flatbuffers
)

# ==============================
# Windows 平台特殊处理
# ==============================

if (WIN32)
    target_link_libraries(server PRIVATE ws2_32 winmm)
endif()

target_include_directories(server PRIVATE vendors/flatbuffers/include)

# ==============================
# 输出目录
# ==============================

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)

