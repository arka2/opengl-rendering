#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <GL/glew.h>
#include <glm/gtx/transform.hpp>


class Shader
{
public:
    // the program ID
    unsigned int ID;

    // constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    // use/activate the shader
    void use();
    // utility uniform functions
    void setBoolValue(const std::string& name, bool value) const;
    void setIntValue(const std::string& name, int value) const;
    void setFloatValue(const std::string& name, float value) const;
    void setVec2Value(const std::string& name, glm::vec2& value) const;
    void setMat4(const std::string& name, glm::mat4 &mat) const;
    void setSampler2D(const std::string& name, unsigned int& value) const;
};

#endif