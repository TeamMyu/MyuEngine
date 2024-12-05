#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(this->id, name.c_str()), value);
    }
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(this->id, name.c_str()), (int)value);
    }
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(this->id, name.c_str()), value);
    }

    void setMatrix4f(const std::string& name, const glm::mat4& value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(this->id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

    void setVector4f(const std::string& name, const glm::vec4& value) const
    {
        glUniform4f(glGetUniformLocation(this->id, name.c_str()),
                    value.x, value.y, value.z, value.w);
    }

    void use()
    {
        glUseProgram(this->id);
    }

private:
    GLuint id;

};
