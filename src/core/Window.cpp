#include "core/Window.h"
#include <GL/glew.h>
#include <cstdio>

Window::Window(const char* title, int width, int height)
    : _width(width), _height(height) {

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        _shouldClose = true;
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    _window = SDL_CreateWindow(title, width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!_window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        _shouldClose = true;
        return;
    }

    _glContext = SDL_GL_CreateContext(_window);
    if (!_glContext) {
        fprintf(stderr, "GL context failed: %s\n", SDL_GetError());
        _shouldClose = true;
        return;
    }

    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        fprintf(stderr, "GLEW init failed: %s\n", glewGetErrorString(glewErr));
        _shouldClose = true;
        return;
    }

    printf("Window created: %dx%d, OpenGL %s\n", width, height,
        glGetString(GL_VERSION));
}

Window::~Window() {
    if (_glContext) SDL_GL_DestroyContext(_glContext);
    if (_window) SDL_DestroyWindow(_window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
}

void Window::swap() {
    SDL_GL_SwapWindow(_window);
}
