#pragma once
#include <GL/glew.h>
#include <SDL3/SDL.h>

class Window {
public:
    Window(const char* title, int width, int height);
    ~Window();

    void swap();
    bool shouldClose() const { return _shouldClose; }
    void setShouldClose(bool v) { _shouldClose = v; }
    int width() const { return _width; }
    int height() const { return _height; }
    SDL_Window* sdlWindow() const { return _window; }

private:
    SDL_Window* _window = nullptr;
    SDL_GLContext _glContext = nullptr;
    int _width, _height;
    bool _shouldClose = false;
};
