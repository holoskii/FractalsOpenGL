#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Shader
{
public:
    unsigned int programID;
    
    Shader() = default;

    // Constructor loads shader code from file, and launches the GL Program
    Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
    {
        load(vertexShaderPath, fragmentShaderPath);
    }

    void load(const char* vertexShaderPath, const char* fragmentShaderPath)
    {
        // try to open files
        const char* vertexShaderCode;
        const char* fragmentShaderCode;
        std::ifstream vertexShaderFile(vertexShaderPath);
        std::ifstream fragmentShaderFile(fragmentShaderPath);
        if (!vertexShaderFile.is_open()) {
            throw std::exception((std::string("Could not load file: ") + std::string(vertexShaderPath)).c_str());
        }
        if (!fragmentShaderFile.is_open()) {
            throw std::exception((std::string("Could not load file: ") + std::string(fragmentShaderPath)).c_str());
        }

        // load files into strings
        std::stringstream vertexShaderStream, fragmentShaderStream;
        vertexShaderStream << vertexShaderFile.rdbuf();
        fragmentShaderStream << fragmentShaderFile.rdbuf();
        std::string vertexShaderCodeStr = vertexShaderStream.str();
        std::string fragmentShaderCodeStr = fragmentShaderStream.str();
        vertexShaderCode = vertexShaderCodeStr.c_str();
        fragmentShaderCode = fragmentShaderCodeStr.c_str();

        // create shaders
        unsigned int vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShaderID, 1, &vertexShaderCode, NULL);
        glCompileShader(vertexShaderID);
        checkShaderErrors(vertexShaderID);
        unsigned int fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShaderID, 1, &fragmentShaderCode, NULL);
        glCompileShader(fragmentShaderID);
        checkShaderErrors(fragmentShaderID);

        // create and compile program
        programID = glCreateProgram();
        glAttachShader(programID, vertexShaderID);
        glAttachShader(programID, fragmentShaderID);
        glLinkProgram(programID);
        checkShaderErrors(programID);

        // don't need those anymore
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
    }

    ~Shader()
    {
        glDeleteProgram(programID);
    }

    void useProgram()
    {
        glUseProgram(programID);
    }

    void checkShaderErrors(const unsigned int shaderID)
    {
        int success;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[1024];
            glGetShaderInfoLog(shaderID, 1024, NULL, infoLog);
            std::cout << "Shader error: " << infoLog << '\n';
        }
    }
};
