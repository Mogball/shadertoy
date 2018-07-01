#include "glsl-strings.h"

#include <GLES2/gl2.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>

#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

static GLsizei s_viewport_width = -1;
static GLsizei s_viewport_height = -1;
static GLuint s_shader_program = 0;
static GLint attribute_position = 0;
static GLint uniform_channel[4];

static GLint uniform_channel_res;
static GLint uniform_channel_time;
static GLint uniform_date;
static GLint uniform_time;
static GLint uniform_mouse;
static GLint uniform_resolution;
static GLint uniform_sample_rate;

static double timespec_diff(
    const struct timespec *start,
    const struct timespec *stop) {

    time_t sec = stop->tv_sec - start->tv_sec;
    long nsec = stop->tv_nsec - start->tv_nsec;
    if (nsec < 0) {
        sec -= (time_t) 1;
        nsec += (long) 1000000000;
    }
    return (double) sec + (double) nsec / 1000000000.0;
}

static bool monotonic_time(struct timespec *tp) {
    if (clock_gettime(CLOCK_MONOTONIC, tp) == -1)
    { return false; }
    return true;
}

static bool compile_shader(
    GLenum type,
    GLsizei num_sources,
    const char **sources,
    GLuint *ret) {

    GLuint shader = 0;
    GLint success = 0;
    GLsizei i = 0;
    GLsizei len_sources[num_sources];

    char *log_msg = NULL;
    GLint log_len = 0;

    for (i = 0; i < num_sources; ++i)
    { len_sources[i] = (GLsizei) strlen(sources[i]); }

    shader = glCreateShader(type);
    glShaderSource(shader, num_sources, sources, len_sources);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 1) {
            log_msg = (char *) malloc(log_len);
            glGetShaderInfoLog(shader, log_len, NULL, log_msg);
            fprintf(stderr, "%s\n", log_msg);
            free(log_msg);
        }
        fprintf(stderr, "Error compiling shader program\n");
        for (i = 0; i < num_sources; ++i) {
            fprintf(stderr, "%s", sources[i]);
        }
        return false;
    }

    *ret = shader;
    return true;
}

static void resize_viewport(GLsizei w, GLsizei h) {
    if (s_viewport_width != w || s_viewport_height != h) {
        glUniform3f(uniform_resolution, (float) w, (float) h, 0.0f);
        glViewport(0, 0, w, h);
        s_viewport_width = w;
        s_viewport_height = h;
        fprintf(stdout, "Setting window size to (%i, %i)\n", w, h);
    }
}

static void render(float abstime) {
    static const GLfloat vertices[] = {
        -1.0f, -1.0f,
        +1.0f, -1.0f,
        -1.0f, +1.0f,
        +1.0f, +1.0f
    };
    if (uniform_time >= 0)
    { glUniform1f(uniform_time, abstime); }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnableVertexAttribArray(attribute_position);
    glVertexAttribPointer(attribute_position, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static bool setup(const char *fragment_body) {
    const char *sources[4] = {0};
    GLuint vertex_shader = 0;
    GLuint fragment_shader = 0;
    GLuint shader_program = 0;

    GLint success = 0;
    GLint log_len = 0;
    char *log_msg = NULL;

    sources[0] = GLSL_STRING(common_header);
    sources[1] = GLSL_STRING(vertex_shader);
    if (!compile_shader(GL_VERTEX_SHADER, 2, sources, &vertex_shader))
    { return false; }

    sources[0] = GLSL_STRING(common_header);
    sources[1] = GLSL_STRING(fragment_header);
    sources[2] = fragment_body;
    sources[3] = GLSL_STRING(fragment_footer);
    if (!compile_shader(GL_FRAGMENT_SHADER, 4, sources, &fragment_shader))
    { return false; }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 1) {
            log_msg = (char *) malloc(log_len);
            glGetProgramInfoLog(shader_program, log_len, NULL, log_msg);
            fprintf(stderr, "%s\n", log_msg);
            free(log_msg);
        }
        fprintf(stderr, "Error linking shader program\n");
        return false;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glReleaseShaderCompiler();
    glUseProgram(shader_program);
    glValidateProgram(shader_program);
    s_shader_program = shader_program;

    attribute_position = glGetAttribLocation(shader_program, "iPosition");
    uniform_channel[0] = glGetUniformLocation(shader_program, "iChannel0");
    uniform_channel[1] = glGetUniformLocation(shader_program, "iChannel1");
    uniform_channel[2] = glGetUniformLocation(shader_program, "iChannel2");
    uniform_channel[3] = glGetUniformLocation(shader_program, "iChannel3");
    uniform_channel_res = glGetUniformLocation(shader_program, "iChannelResolution");
    uniform_channel_time = glGetUniformLocation(shader_program, "iChannelTime");
    uniform_date = glGetUniformLocation(shader_program, "iDate");
    uniform_time = glGetUniformLocation(shader_program, "iTime");
    uniform_mouse = glGetUniformLocation(shader_program, "iMouse");
    uniform_resolution = glGetUniformLocation(shader_program, "iResolution");
    uniform_sample_rate = glGetUniformLocation(shader_program, "iSampleRate");

    return true;
}

static bool shutdown(void) {
    glDeleteProgram(s_shader_program);
    return true;
}

static char *read_file(const char *path) {
    long length = 0;
    char *result = NULL;
    FILE *file = fopen(path, "r");
    if (file) {
        int status = fseek(file, 0, SEEK_END);
        if (status != 0) {
            fclose(file);
            return NULL;
        }
        length = ftell(file);
        status = fseek(file, 0, SEEK_SET);
        if (status != 0) {
            fclose(file);
            return NULL;
        }
        result = (char *) malloc((length + 1) * sizeof(char));
        if (result != NULL) {
            size_t actual_length = fread(result, sizeof(char), length, file);
            result[actual_length] = '\0';
        }
        fclose(file);
        return result;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    const int width = 1600;
    const int height = 900;
    const char *path = NULL;
    const char *shader = GLSL_STRING(fragment_shader);
    char *value = NULL;

    if (argc >= 2) {
        path = argv[1];
        fprintf(stdout, "Reading shader file: %s\n", path);
        value = read_file(path);
        if (value == NULL) {
            return -1;
        }
        shader = value;
    }

    fprintf(stdout, "Booting shader toy emulator ... ");
    sf::Window app(sf::VideoMode(width, height), "Shader Toy");
    struct timespec start;
    struct timespec current;

    if (!setup(shader)) {
        fprintf(stderr, "Failed to setup shader\n");
        return -1;
    }
    fprintf(stdout, "Done!\n");
    resize_viewport(width, height);
    monotonic_time(&start);

    while (app.isOpen()) {
        sf::Event event;
        while (app.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
            { app.close(); }
        }
        monotonic_time(&current);
        render((float) timespec_diff(&start, &current));
        app.display();
    }

    shutdown();
    if (value) { free(value); }
}
