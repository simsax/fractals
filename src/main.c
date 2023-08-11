#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Config.h"

#define WIDTH 512
#define HEIGHT 512

GLFWwindow* window;
uint32_t vbo;
uint32_t vertex_shader;
uint32_t fragment_shader;
uint32_t shader_program;
uint32_t vao;

float positions[12]
    = { -1.0f, -1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f };

// float positions[12]
//     = { -0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f };

char* read_file(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Couldn't open file \"%s\".\n", path);
        exit(1);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char* buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(1);
    }
    size_t bytes_read = fread(buffer, sizeof(char), file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Couldn't read file \"%s\".\n", path);
        exit(1);
    }
    buffer[bytes_read] = '\0';
    fclose(file);
    return buffer;
}

static void error_callback(int error, const char* msg)
{
    fprintf(stderr, "Error %d: %s", error, msg);
}

void GLAPIENTRY message_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* userParam)
{
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void init_window()
{
    glfwSetErrorCallback(error_callback);
    // initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // set opengl to be core
#ifndef NDEBUG
    // During init, enable debug output
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    window = glfwCreateWindow(WIDTH, HEIGHT, "Fractals", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window");
        exit(1);
    }
    glfwMakeContextCurrent(window);

    glfwSwapInterval(0); // enables v-sync

    // initialize glew
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize glew");
        exit(1);
    }

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    // set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
#ifndef NDEBUG
    // debug stuff
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(message_callback, 0);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
#endif
}

void init_vao()
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
}

void init_vbo()
{
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void create_shader(GLenum type, uint32_t* shader, const char* shader_source)
{
    int success;
    char info_log[512];

    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &shader_source, NULL);
    glCompileShader(*shader);
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(*shader, 512, NULL, info_log);
        fprintf(stderr, "Vertex shader compilation failed.\n");
        exit(1);
    }
}

void init_shader()
{
    int success;
    char info_log[512];
    size_t path_size = strlen(SOURCE_DIR) + strlen("/src/shader.vert");
    char* shader_path = malloc(path_size + 1);
    snprintf(shader_path, path_size + 1, "%s%s", SOURCE_DIR, "/src/shader.vert");
    char* vertex_shader_source = read_file(shader_path);
    snprintf(shader_path, path_size + 1, "%s%s", SOURCE_DIR, "/src/shader.frag");
    char* fragment_shader_source = read_file(shader_path);
    free(shader_path);

    create_shader(GL_VERTEX_SHADER, &vertex_shader, vertex_shader_source);
    create_shader(GL_FRAGMENT_SHADER, &fragment_shader, fragment_shader_source);
    free(vertex_shader_source);
    free(fragment_shader_source);
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        fprintf(stderr, "Shader linking failed.\n");
        exit(1);
    }
    glUseProgram(shader_program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void render_loop()
{
    while (!glfwWindowShouldClose(window)) {
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main(void)
{
    init_window();
    init_vao();
    init_vbo();
    init_shader();
    render_loop();
    return 0;
}