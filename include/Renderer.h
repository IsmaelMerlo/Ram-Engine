#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

struct Particle {
    glm::vec4 position; // x, y, z, vida
    glm::vec4 velocity; // vx, vy, vz, velocidad_escalar
};

class Renderer {
public:
    bool init();
    void updateAndRender(float deltaTime, float totalTime, int width, int height);
    void cleanup();

private:
    GLuint compileShader(GLenum type, const char* source);
    GLuint createProgram(GLuint vs, GLuint fs);
    GLuint createComputeProgram(GLuint cs);

    static const unsigned int NUM_PARTICLES = 262144; // 256k partículas caóticas

    GLuint m_vao = 0;
    GLuint m_ssbo = 0;
    GLuint m_renderProgram = 0;
    GLuint m_computeProgram = 0;
};