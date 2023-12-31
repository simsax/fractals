#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <sys/inotify.h>
#include <errno.h>
#include <unistd.h>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Config.h"

#define WIDTH 1024
#define HEIGHT 1024

GLFWwindow* window;

typedef float Value;

float resolution[2] = { WIDTH, HEIGHT };
float mouse_position[2] = { 0.0, 0.0 };
float prev_mouse_position[2] = { -1.0, -1.0 };
Value x_coords[2] = { -2.0, 2.0 };
Value y_coords[2] = { -2.0, 2.0 };
bool dragging = false;
bool right_mouse_pressed = false;

// returns a dynamically allocated buffer of chars which needs to be freed by the caller
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        right_mouse_pressed = true;
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        right_mouse_pressed = false;
}

void scale(float scaling_factor, Value* arr, uint32_t size)
{
    for (int i = 0; i < size; i++) {
        arr[i] *= scaling_factor;
    }
}

void add_element(float point, Value* arr, uint32_t size)
{
    for (int i = 0; i < size; i++) {
        arr[i] += point;
    }
}

void sub_element(float point, Value* arr, uint32_t size)
{
    for (int i = 0; i < size; i++) {
        arr[i] -= point;
    }
}

void update_coords(float yoffset)
{
    float scaling_factor = 0.8;
    float origin[2] = {
        (1 - mouse_position[0]) * x_coords[0] + mouse_position[0] * x_coords[1],
        (1 - mouse_position[1]) * y_coords[0] + mouse_position[1] * y_coords[1],
    };
    sub_element(origin[0], x_coords, 2);
    sub_element(origin[1], y_coords, 2);
    scale(yoffset > 0 ? scaling_factor : 1.0 / scaling_factor, x_coords, 2);
    scale(yoffset > 0 ? scaling_factor : 1.0 / scaling_factor, y_coords, 2);
    add_element(origin[0], x_coords, 2);
    add_element(origin[1], y_coords, 2);
}

void translate(float xoffset, float yoffset)
{
    // transform offset from 0-1 to offset on the complex plane
    float x_len = x_coords[1] - x_coords[0];
    float y_len = y_coords[1] - y_coords[0];
    xoffset *= x_len;
    yoffset *= y_len;
    sub_element(xoffset, x_coords, 2);
    sub_element(yoffset, y_coords, 2);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    mouse_position[0] = (float)x / resolution[0];
    mouse_position[1] = 1 - (float)y / resolution[1]; // flip y coord
    update_coords(yoffset);
}

void drag()
{
    float xoffset = mouse_position[0] - prev_mouse_position[0];
    float yoffset = mouse_position[1] - prev_mouse_position[1];
    translate(xoffset, yoffset);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        mouse_position[0] = xpos / resolution[0];
        mouse_position[1] = 1 - ypos / resolution[1];
        if (dragging) {
            // not first press
            drag();
        }
        dragging = true;
        prev_mouse_position[0] = mouse_position[0];
        prev_mouse_position[1] = mouse_position[1];
    } else {
        dragging = false;
    }
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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
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

    glfwSwapInterval(1); // enables v-sync

    // initialize glew
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize glew");
        exit(1);
    }

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    glViewport(0, 0, WIDTH, HEIGHT);

    // set callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
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
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
}

GLuint compile_shader(GLenum type, const char* shader_source)
{
    int success;
    char info_log[512];

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "Vertex shader compilation failed.\n%s\n", info_log);
        return shader;
    }
    return shader;
}

GLuint load_shaders()
{
    int success;
    char info_log[512];
    char shader_path[512];
    snprintf(shader_path, 512, "%s%s", SOURCE_DIR, "/shaders/shader.vert");
    char* vertex_shader_source = read_file(shader_path);
    snprintf(shader_path, 512, "%s%s", SOURCE_DIR, "/shaders/shader.frag");
    char* fragment_shader_source = read_file(shader_path);

    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    if (!vertex_shader) {
        free(vertex_shader_source);
        free(fragment_shader_source);
        glDeleteShader(vertex_shader);
        return 0;
    }
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    if (!fragment_shader) {
        free(vertex_shader_source);
        free(fragment_shader_source);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return 0;
    }

    free(vertex_shader_source);
    free(fragment_shader_source);
    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, info_log);
        fprintf(stderr, "Shader linking failed.\n%s", info_log);
        glDeleteProgram(shader_program);
        return 0;
    }
    return shader_program;
}

void reload_shaders(GLuint* program)
{
    GLuint shader_program = load_shaders();
    if (shader_program) {
        glDeleteProgram(*program);
        *program = shader_program;
    }
}

// adapted from man inotify
void handle_events(GLuint* shader_program, int fd)
{
    /* Some systems cannot read integer variables if they are not
        properly aligned. On other systems, incorrect alignment may
        decrease performance. Hence, the buffer used for reading from
        the inotify file descriptor should have the same alignment as
        struct inotify_event. */
    char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event* event;
    ssize_t len;

    /* Loop while events can be read from inotify file descriptor. */

    for (;;) {
        /* Read some events. */

        len = read(fd, buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        /* If the nonblocking read() found no events to read, then
            it returns -1 with errno set to EAGAIN. In that case,
            we exit the loop. */

        if (len <= 0)
            break;

        /* Loop over all events in the buffer. */

        for (char* ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len) {

            event = (const struct inotify_event*)ptr;
            if (event->mask & IN_CLOSE_WRITE || event->mask & IN_DELETE || event->mask & IN_CREATE)
                reload_shaders(shader_program);
        }
    }
}

void init_inotify(struct pollfd* fds, int* fd)
{
    char dir_name[512];
    snprintf(dir_name, 512, "%s%s", SOURCE_DIR, "/shaders");

    /* Create the file descriptor for accessing the inotify API. */
    *fd = inotify_init1(IN_NONBLOCK);
    if (*fd == -1) {
        perror("inotify_init1");
        exit(EXIT_FAILURE);
    }

    int wd = inotify_add_watch(*fd, dir_name, IN_CLOSE_WRITE | IN_DELETE | IN_CREATE);
    if (wd == -1) {
        fprintf(stderr, "Cannot watch '%s': %s\n", dir_name, strerror(errno));
        exit(EXIT_FAILURE);
    }

    fds->fd = *fd; /* Inotify input */
    fds->events = POLLIN;
}

void poll_shaders(GLuint* shader_program, struct pollfd* fds, int fd)
{
    int poll_num = poll(fds, 1, 0);
    if (poll_num == -1 && errno != EINTR) {
        perror("poll");
        exit(EXIT_FAILURE);
    }

    if (poll_num > 0) {
        if (fds->revents & POLLIN) {
            handle_events(shader_program, fd);
        }
    }
}

void reset_coords()
{
    x_coords[0] = -2.0;
    y_coords[0] = -2.0;
    x_coords[1] = 2.0;
    y_coords[1] = 2.0;
}

void render_loop(GLuint shader_program, struct pollfd* fds, int fd)
{
    int iTime_location = glGetUniformLocation(shader_program, "iTime");
    int iResolution_location = glGetUniformLocation(shader_program, "iResolution");
    int xcoords = glGetUniformLocation(shader_program, "xCoords");
    int ycoords = glGetUniformLocation(shader_program, "yCoords");
    while (!glfwWindowShouldClose(window)) {
        if (right_mouse_pressed) {
            right_mouse_pressed = false;
            reset_coords();
        }
        poll_shaders(&shader_program, fds, fd);
        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_program);
        double current_time = glfwGetTime();
        glUniform1f(iTime_location, current_time);
        glUniform2fv(iResolution_location, 1, &resolution[0]);
        glUniform2fv(xcoords, 1, &x_coords[0]);
        glUniform2fv(ycoords, 1, &y_coords[0]);
        // glUniform2dv(xcoords, 1, &x_coords[0]);
        // glUniform2dv(ycoords, 1, &y_coords[0]);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main(void)
{
    struct pollfd fds;
    int fd;
    init_inotify(&fds, &fd);

    init_window();
    init_vao();
    GLuint shader_program = load_shaders();
    render_loop(shader_program, &fds, fd);

    // close inotify file descriptor
    close(fd);
    return 0;
}
