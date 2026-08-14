#include <glad/glad.h>
#include "Engine.h"
#include <iostream>

bool Engine::init() {
    if (!glfwInit()) {
        std::cout << "Error al inicializar GLFW \n";
        return false;
    }

    // Solicitar OpenGL 4.6 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(1024, 768, "Ram Engine", nullptr, nullptr);
    if (!m_window) {
        std::cout << "Error al crear la ventana de GLFW \n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Error al inicializar GLAD ❌\n";
        return false;
    }

    std::cout << "GLAD inicializado. OpenGL " << glGetString(GL_VERSION) << "\n";

    if (!m_renderer.init()) {
        std::cout << "Error al inicializar el Renderer ❌\n";
        return false;
    }

    m_editor.init();
    m_lastFrameTime = (float)glfwGetTime();

    return true;
}

void Engine::run() {
    while (!glfwWindowShouldClose(m_window)) {
        float currentTime = (float)glfwGetTime();
        float deltaTime = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;

        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);

        // Actualizar física y renderizar
        m_renderer.updateAndRender(deltaTime, currentTime, width, height);

        // Monitor en consola
        m_editor.update(deltaTime, 131072);

        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }
}

void Engine::cleanup() {
    m_renderer.cleanup();
    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
    std::cout << "Ram Engine apagado \n";
}