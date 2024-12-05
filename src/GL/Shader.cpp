#include "GL/Shader.h"

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
   // read shader files
   // TODO: Consider creating a file I/O utility class
   std::string vertexCode, fragmentCode;
   std::ifstream vsFile, fsFile;
   vsFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
   fsFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
   try
   {
      vsFile.open(vertexPath);
      fsFile.open(fragmentPath);

      std::stringstream vsStream, fsStream;
      vsStream << vsFile.rdbuf();
      fsStream << fsFile.rdbuf();

      vsFile.close();
      fsFile.close();
      vertexCode = vsStream.str();
      fragmentCode = fsStream.str();

      std::cout << "Successfully loaded vertex shader from: " << vertexPath << std::endl;
      std::cout << "Successfully loaded fragment shader from: " << fragmentPath << std::endl;
   }
   catch (std::ifstream::failure e)
   {
      std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
   }

   // compile vertex shader
   GLuint vs = glCreateShader(GL_VERTEX_SHADER);
   const char* vsCode = vertexCode.c_str();
   glShaderSource(vs, 1, &vsCode, NULL);
   glCompileShader(vs);

   // check for vertex shader compile status
   int success;
   char infoLog[512];
   glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
   if(!success)
   {
      glGetShaderInfoLog(vs, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
   };


   // compile fragment shader
   GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
   const char* fsCode = fragmentCode.c_str();
   glShaderSource(fs, 1, &fsCode, NULL);
   glCompileShader(fs);

   // check for fragment shader compile status
   glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
   if(!success)
   {
      glGetShaderInfoLog(fs, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
   };

   // initialize shader program
   this->id = glCreateProgram();
   glAttachShader(this->id, vs);
   glAttachShader(this->id, fs);
   glLinkProgram(this->id);

   // print linking errors if any
   glGetProgramiv(this->id, GL_LINK_STATUS, &success);
   if(!success)
   {
      glGetProgramInfoLog(this->id, 512, NULL, infoLog);
      std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
   }

   // delete the shaders as they're linked into our program now and no longer necessary
   glDeleteShader(vs);
   glDeleteShader(fs);

   std::cout << "Successfully linked shader program" << std::endl;
}

Shader::~Shader()
{
   glDeleteProgram(this->id);
}
