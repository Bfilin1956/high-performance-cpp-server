# C++ Async Server (Boost.Asio)

High-performance asynchronous TCP server written in modern C++20 using Boost.Asio.

## Features

- Asynchronous I/O (Boost.Asio)
- Thread pool based execution
- Non-blocking TCP handling
- Lightweight architecture (no heavy frameworks)
- Cross-platform (Linux / Windows)
- Simple client-server protocol
- Custom lock-free logger (freelist-based allocator)
- High-performance memory allocation via custom bump allocator

---

## Architecture

The server is built around:
- `io_context` – core event loop
- `acceptor` – incoming connections
- `session` – per-client connection handler
- thread pool – parallel execution of async handlers

---

## Logging System

The project includes a custom high-performance logging subsystem designed for low latency and minimal allocation overhead.

### Key properties

- Lock-free design (freelist-based)
- Custom bump allocator for fast linear allocations
- Minimal contention in multi-threaded environment
- Designed for high-frequency log bursts (network/server load)
- Reduced heap fragmentation compared to standard allocators

### Memory Model

The logger uses a two-layer allocation strategy:

1. **Bump allocator**
   - Linear allocation in pre-allocated memory block
   - Extremely fast O(1) allocation
   - No per-allocation system calls

2. **Free list fallback**
   - Reuses released log buffers
   - Prevents unnecessary memory growth under load

### Design goal

The logger is optimized for throughput rather than feature richness.
It is intended for server-side diagnostic logging where allocation cost must not affect request latency.

---

## Build

### Requirements

- C++20 compiler (g++ / clang / MSVC)
- CMake 3.16+
- Boost (Asio / system)

### Linux / macOS

```bash
git clone git@github.com:Bfilin1956/high-performance-cpp-server.git
cd high-performance-cpp-server

mkdir build
cd build
cmake ..
make -j$(nproc)
