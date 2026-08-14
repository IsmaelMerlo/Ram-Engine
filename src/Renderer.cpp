#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cstdlib>
#include <cmath>

// GLSL 460 - Compute Shader para la Física en GPU
const char* computeShaderSource = R"(
#version 460 core
layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

struct Particle {
    vec4 pos;
    vec4 vel;
};

layout(std430, binding = 0) buffer ParticleBuffer {
    Particle particles[];
};

uniform float u_DeltaTime;
uniform float u_Time;

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= particles.length()) return;

    vec3 pos = particles[idx].pos.xyz;
    vec3 vel = particles[idx].vel.xyz;

    // Centro de atracción gravitatoria
    float dist = length(pos);
    vec3 dir = normalize(-pos);
    vec3 axis = vec3(0.0, 1.0, 0.0);
    vec3 tangent = cross(dir, axis);

    // Fuerza de atracción + rotación orbital
    vec3 accel = dir * (8.0 / (dist * dist + 0.2)) + tangent * 3.5;

    vel += accel * u_DeltaTime;
    pos += vel * u_DeltaTime;

    // Reiniciar partículas que caen al centro o se alejan demasiado
    if (dist > 6.0 || dist < 0.1) {
        float angle = float(idx) * 0.0001 + u_Time;
        float r = 1.5 + sin(angle * 5.0) * 0.8;
        pos = vec3(cos(angle) * r, (fract(sin(float(idx)) * 43758.54) - 0.5) * 0.3, sin(angle) * r);
        vel = vec3(-sin(angle), 0.0, cos(angle)) * 1.2;
    }

    particles[idx].pos.xyz = pos;
    particles[idx].vel.xyz = vel;
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
    v_Speed = length(a_Vel.xyz);
    gl_Position = u_MVP * vec4(a_Pos.xyz, 1.0);
    gl_PointSize = clamp(3.0 / gl_Position.w, 1.0, 10.0);
}
)";

// GLSL 460 - Fragment Shader
const char* fragmentShaderSource = R"(
#version 460 core
in float v_Speed;
out vec4 FragColor;

void main() {
    // Forma circular suavizada
    vec2 coord = gl_PointCoord - vec2(0.5);
    if (length(coord) > 0.5) discard;

    // Gradiente de color según velocidad (Azul -> Cían -> Dorado brillante)
    vec3 coreColor = mix(vec3(0.1, 0.4, 1.0), vec3(1.0, 0.6, 0.1), clamp(v_Speed * 0.2, 0.0, 1.0));
    FragColor = vec4(coreColor, 0.7);
}
)";

bool Renderer::init() {
    // 1. Compilación de Shaders
    GLuint cs = compileShader(GL_COMPUTE_SHADER, computeShaderSource);
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (!cs || !vs || !fs) return false;

    m_computeProgram = createComputeProgram(cs);
    m_renderProgram = createProgram(vs, fs);

    glDeleteShader(cs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // 2. Inicializar partículas en disco galáctico
    std::vector<Particle> particles(NUM_PARTICLES);
    for (size_t i = 0; i < NUM_PARTICLES; ++i) {
        float angle = ((float)rand() / RAND_MAX) * 6.283185f;
        float radius = 0.5f + ((float)rand() / RAND_MAX) * 2.5f;
        float height = (((float)rand() / RAND_MAX) - 0.5f) * 0.3f;

        particles[i].position = glm::vec4(cos(angle) * radius, height, sin(angle) * radius, 1.0f);
        particles[i].velocity = glm::vec4(-sin(angle) * 0.8f, 0.0f, cos(angle) * 0.8f, 0.0f);
    }

    // 3. Uso de OPENGL 4.6 DSA (Direct State Access)
    // Crear SSBO sin necesidad de glBindBuffer previo
    glCreateBuffers(1, &m_ssbo);
    glNamedBufferData(m_ssbo, sizeof(Particle) * NUM_PARTICLES, particles.data(), GL_DYNAMIC_DRAW);

    // Crear VAO mediante DSA
    glCreateVertexArrays(1, &m_vao);
    glVertexArrayVertexBuffer(m_vao, 0, m_ssbo, 0, sizeof(Particle));

    // Atributo 0: Posición
    glEnableVertexArrayAttrib(m_vao, 0);
    glVertexArrayAttribFormat(m_vao, 0, 4, GL_FLOAT, GL_FALSE, offsetof(Particle, position));
    glVertexArrayAttribBinding(m_vao, 0, 0);

    // Atributo 1: Velocidad
    glEnableVertexArrayAttrib(m_vao, 1);
    glVertexArrayAttribFormat(m_vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(Particle, velocity));
    glVertexArrayAttribBinding(m_vao, 1, 0);

    // Configuración de renderizado
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Mezcla aditiva para efecto de brillo

    return true;
}

void Renderer::updateAndRender(float deltaTime, float totalTime, int width, int height) {
    // --- PASO 1: SIMULACIÓN EN COMPUTO (GPU) ---
    glUseProgram(m_computeProgram);
    glUniform1f(glGetUniformLocation(m_computeProgram, "u_DeltaTime"), deltaTime);
    glUniform1f(glGetUniformLocation(m_computeProgram, "u_Time"), totalTime);

    // Conectar el SSBO al puerto 0
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_ssbo);

    // Ejecutar hilos de cálculo en paralelo
    glDispatchCompute((NUM_PARTICLES + 255) / 256, 1, 1);

    // Barrera de memoria para asegurar que el Compute Shader terminó antes de dibujar
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // --- PASO 2: RENDERIZADO GRÁFICO ---
    glViewport(0, 0, width, height);
    glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_renderProgram);

    // Matrices de Cámara 3D (Cámara rotando lentamente alrededor del centro)
    float camX = sin(totalTime * 0.2f) * 6.0f;
    float camZ = cos(totalTime * 0.2f) * 6.0f;
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(camX, 3.5f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
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

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "Error al compilar Shader: " << infoLog << "\n";
        return 0;
    }
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