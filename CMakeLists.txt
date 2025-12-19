cmake_minimum_required(VERSION 3.16)

# ==============================
# 项目基本信息
# ==============================
project(
        nwpu_gamedev_2025_server
        VERSION 0.1.0
        LANGUAGES C CXX
)

# ==============================
# C++ 标准
# ==============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================
# 编译选项（可选但推荐）
# ==============================
if (MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ==============================
# 第三方库（submodules）
# ==============================

# EnTT（header-only）
add_subdirectory(vendors/entt)

# ENet（需要编译）
set(ENET_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ENET_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(vendors/enet)

# glm（header-only）
add_subdirectory(vendors/glm)

# nlohmann/json（header-only）
add_subdirectory(vendors/nlohmann)

# spdlog（header-only 或 static）
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(vendors/spdlog)

# ==============================
# 服务器可执行程序
# ==============================
add_executable(server)

# ==============================
# 源文件
# ==============================
file(GLOB_RECURSE SERVER_SOURCES
        src/*.cpp
)

target_sources(server PRIVATE ${SERVER_SOURCES})

# ==============================
# 头文件路径
# ==============================
target_include_directories(server
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# ==============================
# 链接第三方库
# ==============================
target_link_libraries(server
        PRIVATE
        enet
        EnTT::EnTT
        glm::glm
        nlohmann_json::nlohmann_json
        spdlog::spdlog_header_only
)

# ==============================
# Windows 平台特殊处理
# ==============================
if (WIN32)
    target_link_libraries(server PRIVATE ws2_32 winmm)
endif()

# ==============================
# 输出目录（可选）
# ==============================
set_target_properties(server PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin
)
