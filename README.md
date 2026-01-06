# NWPU GameDev 2025 Server

*[客户端git仓库](https://github.com/konakona418/nwpu-gamedev-2025)*

## 简介

本项目是 **NWPU GameDev 2025** 游戏项目的服务端，基于 C++20 开发。服务端负责处理核心游戏逻辑、网络通信、物理模拟以及状态同步，支持多人在线对战（CS-like 射击游戏）。

## 核心特性

*   **高性能网络通信**：基于 UDP (ENet) 的低延迟通信，适合 FPS 游戏。
*   **状态机切换**：使用状态机进行服务器状态切换。
*   **物理引擎**：集成 JoltPhysics 进行物理模拟和碰撞检测。
*   **协议序列化**：使用 FlatBuffers 进行高效的数据序列化与反序列化。
*   **多线程架构**：主逻辑线程与网络 I/O 线程分离，保证游戏逻辑的稳定运行。
*   **完整游戏流程**：支持准备、战斗、下包/拆包、结算等完整的 CS 模式游戏流程。

## 技术栈

*   **语言**：C++20
*   **构建工具**：CMake
*   **核心库**：
    *   **网络**：[ENet](http://enet.bespin.org/)
    *   **ECS**：[EnTT](https://github.com/skypjack/entt)
    *   **物理**：[JoltPhysics](https://github.com/jrouwe/JoltPhysics)
    *   **序列化**：[FlatBuffers](https://google.github.io/flatbuffers/)
    *   **数学库**：[GLM](https://github.com/g-truc/glm)
    *   **JSON**：[nlohmann/json](https://github.com/nlohmann/json)
    *   **并发**：[concurrentqueue](https://github.com/cameron314/concurrentqueue)
    *   **日志**：[spdlog](https://github.com/gabime/spdlog)
    *   **配置**：[toml++](https://github.com/marzer/tomlplusplus)

## 目录结构

```
nwpu-gamedev-2025-server/
├── assets/             # 游戏资源（地图、模型等）
├── config/             # 配置文件（武器参数、服务器配置）
├── include/            # 头文件
│   ├── core/           # 核心系统（经济、战斗、移动等）
│   ├── entity/         # 实体组件定义
│   ├── network/        # 网络模块
│   ├── physics/        # 物理模块
│   └── protocol/       # FlatBuffers 生成代码
├── schemas/            # FlatBuffers 协议定义文件 (.fbs)
├── src/                # 源代码
├── vendors/            # 第三方依赖库
├── CMakeLists.txt      # CMake 构建脚本
└── settings.toml       # 服务器运行时配置
```

## 快速开始

### 环境要求

*   Windows / Linux / macOS
*   C++20 兼容编译器 (MSVC, GCC, Clang)
*   CMake 3.16+

### 编译步骤

1.  **克隆仓库**（包含子模块）：
    ```bash
    git clone --recursive https://github.com/your-repo/nwpu-gamedev-2025-server.git
    cd nwpu-gamedev-2025-server
    ```

2.  **构建项目**：
    ```bash
    mkdir build && cd build
    cmake ..
    cmake --build . --config Release
    ```

### 运行服务端

编译完成后，可执行文件通常位于 `build/bin/` 或 `build/Release/` 目录下。

1.  确保 `assets/` 和 `config/` 目录与可执行文件在正确的相对路径下（或手动复制到运行目录）。
2.  运行服务器：
    ```bash
    ./server.exe
    ```

## 游戏设计概览

### 游戏模式
*   **赛制**：可在settings.toml中调整局数，局数是偶数则有一局加时，奇数则没有。
*   **阵营**：CT (反恐精英) vs T (恐怖分子)。
*   **胜利条件**：
    *   **T**：全歼 CT 或 成功引爆 C4。
    *   **CT**：全歼 T 或 成功拆除 C4 或 时间耗尽 T 未下包。

### 核心流程
1.  **准备阶段 (10s)**：购买装备，不可移动。
2.  **作战阶段 (120s)**：自由战斗，T 尝试下包。
3.  **爆破阶段 (45s)**：C4 倒计时，CT 尝试拆包。
4.  **结算阶段**：计算金钱奖励，重置回合。

### 经济系统
*可调整*
*   初始金钱：$800
*   金钱上限：$8000
*   击杀奖励：$200
*   胜/负奖励：$3000 / $2000
*   拆包/下包奖励

## 贡献指南

1.  Fork 本仓库。
2.  创建特性分支 (`git checkout -b feature/AmazingFeature`)。
3.  提交更改 (`git commit -m 'Add some AmazingFeature'`)。
4.  推送到分支 (`git push origin feature/AmazingFeature`)。
5.  提交 Pull Request。

## 许可证

[MIT License](LICENSE)

