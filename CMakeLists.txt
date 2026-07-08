cmake_minimum_required(VERSION 3.20)
project(Ram_Engine CXX C)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE INTERNAL "")

# Configurar el estándar moderno de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Flags de optimización rigurosos para exprimir el rendimiento de la CPU
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -march=native")

# Módulo de CMake para la gestión automatizada de dependencias externas
include(FetchContent)

# 1. Descarga y configuración de GLFW (Gestión de ventanas y contextos)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
)
# Desactivar binarios redundantes de GLFW para acelerar la velocidad de compilación
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# 2. Descarga de GLM (Librería matemática optimizada para gráficos)
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# Localizar los drivers nativos de OpenGL instalados en el sistema operativo
find_package(OpenGL REQUIRED)

# Lista explícita de archivos fuente del motor
set(SOURCES
        src/main.cpp
        src/glad.c
        src/Engine.cpp
        src/Renderer.cpp
)

# Declaración del ejecutable principal del proyecto
add_executable(${PROJECT_NAME} ${SOURCES})

# Configuración de las rutas de búsqueda para las cabeceras (.h / .hpp)
target_include_directories(${PROJECT_NAME} PRIVATE
        include
        ${glfw_SOURCE_DIR}/include
        ${glm_SOURCE_DIR}
)

# Vinculación (Linking) de las librerías estáticas y dinámicas al binario final
target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        glm
        OpenGL::GL
        ${CMAKE_DL_LIBS}
)