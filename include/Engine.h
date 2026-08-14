#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Renderer.h"
#include "Editor.h"

class Engine {
public:
    bool init();
    void run();
    void cleanup();

private:
    GLFWwindow* m_window = nullptr;
    Renderer m_renderer;
    Editor m_editor;
    float m_lastFrameTime = 0.0f;
};