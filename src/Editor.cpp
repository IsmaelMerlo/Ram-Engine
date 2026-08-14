#include "Editor.h"
#include <iostream>

void Editor::init() {
    std::cout << "\n========================================";
    std::cout << "\n RAM ENGINE EDITOR HUD MONITOR";
    std::cout << "\n========================================\n";
}

void Editor::update(float deltaTime, int particleCount) {
    m_timer += deltaTime;
    m_frameCount++;

    if (m_timer >= 1.0f) {
        std::cout << "[Ram Engine Editor] FPS: " << m_frameCount
                  << " | Partículas activas (GPU Compute): " << particleCount << "\n";
        m_frameCount = 0;
        m_timer = 0.0f;
    }
}