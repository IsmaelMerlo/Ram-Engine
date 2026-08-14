#include <glad/glad.h>
#include "Engine.h"
#include <iostream>

bool Engine::init() {
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(1024, 768, "Ram Engine v1.0.0 - Atractor Caótico de Aizawa - OpenGL 4.6", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return false;

    // Desactivar Depth Test para que el Alpha Blending Additivo brille al máximo
    glDisable(GL_DEPTH_TEST);

    if (!m_renderer.init()) return false;

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

        m_renderer.updateAndRender(deltaTime, currentTime, width, height);
        m_editor.update(deltaTime, 262144);

        glfwPollEvents();
        glfwSwapBuffers(m_window);
    }
}

void Engine::cleanup() {
    m_renderer.cleanup();
    if (m_window) glfwDestroyWindow(m_window);
    glfwTerminate();
}