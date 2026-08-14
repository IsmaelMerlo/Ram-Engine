#pragma once

class Editor {
public:
    void init();
    void update(float deltaTime, int particleCount);

private:
    float m_timer = 0.0f;
    int m_frameCount = 0;
};