#pragma once
#include <GL/glew.h>

class Texture {
public:
    Texture() = default;
    Texture(const char* path);
    ~Texture();

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(int unit = 0) const;
    bool valid() const { return _id != 0; }
    static Texture createSolid(unsigned char r, unsigned char g, unsigned char b);

private:
    GLuint _id = 0;
};
