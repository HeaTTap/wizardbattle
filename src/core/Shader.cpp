#include "core/Shader.h"
#include <cstdio>
#include <fstream>
#include <sstream>

Shader::Shader(const char* vertPath, const char* fragPath) {
    std::string vertSrc = readFile(vertPath);
    std::string fragSrc = readFile(fragPath);
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertSrc.c_str());
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragSrc.c_str());
    if (!vs || !fs) { _program = 0; return; }

    _program = glCreateProgram();
    glAttachShader(_program, vs);
    glAttachShader(_program, fs);
    glLinkProgram(_program);

    GLint success;
    glGetProgramiv(_program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(_program, 512, nullptr, log);
        fprintf(stderr, "Shader link error: %s\n", log);
        glDeleteProgram(_program);
        _program = 0;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() {
    if (_program) glDeleteProgram(_program);
}

void Shader::use() const { glUseProgram(_program); }

void Shader::setInt(const char* name, int val) const {
    glUniform1i(glGetUniformLocation(_program, name), val);
}

void Shader::setFloat(const char* name, float val) const {
    glUniform1f(glGetUniformLocation(_program, name), val);
}

void Shader::setVec3(const char* name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(_program, name), x, y, z);
}

void Shader::setMat4(const char* name, const float* mat) const {
    glUniformMatrix4fv(glGetUniformLocation(_program, name), 1, GL_FALSE, mat);
}

std::string Shader::readFile(const char* path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        fprintf(stderr, "Cannot open shader: %s\n", path);
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

GLuint Shader::compileShader(GLenum type, const char* source) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &source, nullptr);
    glCompileShader(s);
    GLint success;
    glGetShaderiv(s, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        fprintf(stderr, "Shader compile error (%d): %s\n", type, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}
