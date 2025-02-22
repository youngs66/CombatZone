#pragma once
#include <string>

#ifdef USE_GLES2
    #include <GLES2/gl2.h>
#else
    #include <GL/glew.h>
#endif

#include "../external/glm/glm/glm.hpp"

class Shader {
public:
    Shader(const char* vertexPath, const char* fragmentPath);
    Shader(GLuint programId) : ID(programId) {}
    void use();
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setInt(const std::string &name, int value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setFloat(const std::string &name, float value) const;
    
    GLuint ID;
private:
    void checkCompileErrors(GLuint shader, std::string type);
}; 