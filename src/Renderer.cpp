#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>

// GLSL 460 - Compute Shader: Integración de Ecuaciones del Atractor de Aizawa
const char* computeShaderSource = R"(
#version 460 core
layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

struct Particle {
    vec4 pos; // x, y, z, life
    vec4 vel; // vx, vy, vz, speed
};

layout(std430, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};

uniform float u_DeltaTime;
uniform float u_Time;

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= particles.length()) return;

    vec3 p = particles[idx].pos.xyz;

    // Constantes del Atractor de Aizawa
    float a = 0.95;
    float b = 0.7;
    float c = 0.6;
    float d = 3.5;
    float e = 0.25;
    float f = 0.1;

    // Ecuaciones diferenciales del Caos
    float dx = (p.z - b) * p.x - d * p.y;
    float dy = d * p.x + (p.z - b) * p.y;
    float dz = c + a * p.z - (p.z * p.z * p.z) / 3.0 - (p.x * p.x + p.y * p.y) * (1.0 + e * p.z) + f * p.z * (p.x * p.x * p.x);

    vec3 v = vec3(dx, dy, dz);

    // Integración numérica
    float dt = u_DeltaTime * 0.4;
    p += v * dt;

    float speed = length(v);

    // Reiniciar partícula si escapa del campo caótico
    if (length(p) > 5.0 || isnan(p.x)) {
        float angle = float(idx) * 0.01 + u_Time;
        p = vec3(cos(angle) * 0.1, sin(angle) * 0.1, 0.1 + (fract(sin(float(idx)) * 43758.54) - 0.5) * 0.1);
    }

    particles[idx].pos.xyz = p;
    particles[idx].vel = vec4(v, speed);
}
)";

// GLSL 460 - Vertex Shader
const char* vertexShaderSource = R"(
#version 460 core
layout(location = 0) in vec4 a_Pos;
layout(location = 1) in vec4 a_Vel;

out float v_Speed;

uniform mat4 u_MVP;

void main() {
    v_Speed = a_Vel.w; // Transmitimos la velocidad al Fragment Shader
    gl_Position = u_MVP * vec4(a_Pos.xyz, 1.0);
    gl_PointSize = clamp(2.5 / gl_Position.w, 1.0, 8.0);
}
)";

// GLSL 460 - Fragment Shader: Renderizado de Neón Ultravioleta / Fuego Cuántico
const char* fragmentShaderSource = R"(
#version 460 core
in float v_Speed;
out vec4 FragColor;

void main() {
    // Generar forma de punto esférico brillante
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    if (dist > 0.5) discard;

    float alpha = smoothstep(0.5, 0.0, dist);

    // Gradiente térmico caótico: Magenta -> Cian -> Dorado Radiante
    vec3 colMagenta = vec3(0.8, 0.05, 0.9);
    vec3 colCyan    = vec3(0.0, 0.8, 1.0);
    vec3 colGold    = vec3(1.0, 0.8, 0.2);

    float t = clamp(v_Speed * 0.15, 0.0, 1.0);
    vec3 color = mix(colMagenta, colCyan, t);
    color = mix(color, colGold, pow(t, 2.0));

    FragColor = vec4(color, alpha * 0.8);
}
)";

bool Renderer::init() {
    GLuint cs = compileShader(GL_COMPUTE_SHADER, computeShaderSource);
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (!cs || !vs || !fs) return false;

    m_computeProgram = createComputeProgram(cs);
    m_renderProgram = createProgram(vs, fs);

    glDeleteShader(cs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Sembrar partículas en una esfera inicial
    std::vector<Particle> particles(NUM_PARTICLES);
    for (size_t i = 0; i < NUM_PARTICLES; ++i) {
        float r = 0.2f * ((float)rand() / RAND_MAX);
        float theta = ((float)rand() / RAND_MAX) * 3.14159f * 2.0f;
        float phi = ((float)rand() / RAND_MAX) * 3.14159f;

        particles[i].position = glm::vec4(
            r * sin(phi) * cos(theta),
            r * sin(phi) * sin(theta),
            r * cos(phi) + 0.5f,
            1.0f
        );
        particles[i].velocity = glm::vec4(0.0f);
    }

    // --- OpenGL 4.6 DSA (Direct State Access) ---
    glCreateBuffers(1, &m_ssbo);
    glNamedBufferData(m_ssbo, sizeof(Particle) * NUM_PARTICLES, particles.data(), GL_DYNAMIC_DRAW);

    glCreateVertexArrays(1, &m_vao);
    glVertexArrayVertexBuffer(m_vao, 0, m_ssbo, 0, sizeof(Particle));

    glEnableVertexArrayAttrib(m_vao, 0); // Posición
    glVertexArrayAttribFormat(m_vao, 0, 4, GL_FLOAT, GL_FALSE, offsetof(Particle, position));
    glVertexArrayAttribBinding(m_vao, 0, 0);

    glEnableVertexArrayAttrib(m_vao, 1); // Velocidad
    glVertexArrayAttribFormat(m_vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(Particle, velocity));
    glVertexArrayAttribBinding(m_vao, 0, 0);

    // Configuración para mezcla aditiva de luz
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    return true;
}

void Renderer::updateAndRender(float deltaTime, float totalTime, int width, int height) {
    // COMPUTO EN GPU
    glUseProgram(m_computeProgram);
    glUniform1f(glGetUniformLocation(m_computeProgram, "u_DeltaTime"), deltaTime);
    glUniform1f(glGetUniformLocation(m_computeProgram, "u_Time"), totalTime);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);
    glDispatchCompute((NUM_PARTICLES + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // RENDERIZADO
    glViewport(0, 0, width, height);
    glClearColor(0.01f, 0.01f, 0.03f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_renderProgram);

    // Órbita alrededor del atractor
    float camRadius = 4.5f;
    float camX = sin(totalTime * 0.15f) * camRadius;
    float camZ = cos(totalTime * 0.15f) * camRadius;
    float camY = sin(totalTime * 0.1f) * 1.5f + 0.5f;

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 mvp = proj * view;

    glUniformMatrix4fv(glGetUniformLocation(m_renderProgram, "u_MVP"), 1, GL_FALSE, glm::value_ptr(mvp));

    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
}

void Renderer::cleanup() {
    if (m_ssbo) glDeleteBuffers(1, &m_ssbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_renderProgram) glDeleteProgram(m_renderProgram);
    if (m_computeProgram) glDeleteProgram(m_computeProgram);
}

GLuint Renderer::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}

GLuint Renderer::createProgram(GLuint vs, GLuint fs) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    return program;
}

GLuint Renderer::createComputeProgram(GLuint cs) {
    GLuint program = glCreateProgram();
    glAttachShader(program, cs);
    glLinkProgram(program);
    return program;
}