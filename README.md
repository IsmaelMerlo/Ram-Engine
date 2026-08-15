# Ram Engine 🐏

A high-performance, GPU-driven 3D graphics engine written in **modern C++17** and **OpenGL 4.6 Core Profile**. 

https://github.com/user-attachments/assets/6ade2bb5-cad2-4564-a132-0c29dc7578a1



**Ram Engine** is designed as a zero-overhead graphics laboratory focused on parallel compute pipelines, modern state management via Direct State Access (DSA), and real-time mathematical field simulations.

---

## OpenGL 4.6 features used

Unlike legacy OpenGL implementations that suffer from state-machine binding overhead and CPU bottlenecks, **Ram Engine** leverages modern GPU-driven architecture features:

* **Direct State Access (DSA):** Eliminates global context binding overhead by creating and configuring buffers (`glCreateBuffers`), vertex array objects (`glCreateVertexArrays`), and memory structures directly on the GPU without requiring `glBind*` state switches.
* **Compute Shaders (`GLSL 460 core`):** Parallel compute pipeline execution (`glDispatchCompute`) capable of integrating differential equations for **262,144 (256K) concurrent particles** strictly within GPU hardware.
* **Shader Storage Buffer Objects (SSBOs):** Zero-copy memory architecture sharing `std430` layout structures between Compute and Rasterization stages without transferring data back to host CPU RAM.
* **Explicit Memory Synchronization:** Precise memory barrier implementation (`glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT)`) guaranteeing data hazard safety between compute dispatches and draw calls.
* **Additive Particle Shading:** Speed-vector mapping within the Fragment Shader generating dynamic thermal spectral gradients (Ultraviolet $\rightarrow$ Cyan $\rightarrow$ Solar Gold).

---

## Maths

The engine calculates the non-linear **Aizawa Chaotic Attractor** in real time across 256K particles through numerical integration executed entirely within GPU Workgroups:

$$
\begin{aligned}
\frac{dx}{dt} &= (z - b)x - dy \\
\frac{dy}{dt} &= dx + (z - b)y \\
\frac{dz}{dt} &= c + az - \frac{z^3}{3} - (x^2 + y^2)(1 + ez) + f z x^3
\end{aligned}
$$

> *Where constants $a=0.95$, $b=0.7$, $c=0.6$, $d=3.5$, $e=0.25$, and $f=0.1$ define the complex toroidal chaotic topology.*

---

## Architecture & Module Breakdown

The codebase strictly enforces the **Single Responsibility Principle (SRP)**, separating system lifecycle, GPU rendering, and real-time diagnostic monitoring:

```text
Ram_Engine/
├── include/
│   ├── Engine.h     # Core Application Controller & Window Manager
│   ├── Renderer.h   # Low-Level OpenGL 4.6 Compute & Graphics Subsystem
│   └── Editor.h     # Performance Profiler & Real-time Telemetry Monitor
└── src/
    ├── Engine.cpp   # GLFW Swapchain, Event Loop & Delta-time Management
    ├── Renderer.cpp # SSBO Allocation, Shader Compilation & Dispatch Pipeline
    ├── Editor.cpp   # FPS Counters & Metric Aggregation
    └── main.cpp     # Engine Entry Point
```

* **`Engine`**: Manages window creation, OpenGL context binding via **GLFW**, swapchain buffers, event polling, and delta-time frame synchronization.
* **`Renderer`**: Encapsulates all graphics hardware state: pipeline execution, shader compilation (`GL_COMPUTE_SHADER`, `GL_VERTEX_SHADER`, `GL_FRAGMENT_SHADER`), SSBO buffer allocation via DSA, and viewport transformations using **GLM**.
* **`Editor`**: Light-weight HUD telemetry monitor tracking frame latency, active thread dispatches, and throughput metrics.

---

## Techs & Dependencies

* **Language:** C++17 (Strict optimization flags: `-Wall -Wextra -O3 -march=native`)
* **Graphics API:** OpenGL 4.6 Core Profile
* **Extension Loader:** GLAD
* **Windowing & Input:** GLFW 3.3
* **Mathematics:** GLM (OpenGL Mathematics)
* **Build System:** CMake 3.20+ (Automated dependency fetch via `FetchContent`)

---

## Building & Running

### Prerequisites
* C++17 compatible compiler (`G++`, `Clang`, or `MSVC`)
* CMake 3.20 or higher
* GPU with OpenGL 4.6 drivers supported

### Build Instructions
```bash
# Clone the repository
git clone [https://github.com/IsmaelMerlo/Ram-Engine.git](https://github.com/IsmaelMerlo/Ram-Engine.git)
cd Ram-Engine

# Generate build files via CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build executable
cmake --build build --config Release

# Run the Engine
./build/Ram-Engine
```

---
*Developed as an exploration into high-performance GPU programming and modern C++ software architecture, just for fun.*
