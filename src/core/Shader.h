#pragma once
#include <GL/glew.h>
#include <string>

class Shader {
public:
    Shader(const char* vertPath, const char* fragPath);
    ~Shader();

    void use() const;
    GLuint id() const { return _program; }

    void setInt(const char* name, int val) const;
    void setFloat(const char* name, float val) const;
    void setVec3(const char* name, float x, float y, float z) const;
    void setMat4(const char* name, const float* mat) const;

private:
    GLuint _program = 0;
    static std::string readFile(const char* path);
    static GLuint compileShader(GLenum type, const char* source);
};
