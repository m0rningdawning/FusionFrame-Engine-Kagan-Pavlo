#include "Shader.h"
#include <glew.h>
#include <glfw3.h>
#include <string>
#include <fstream>

std::string FUSIONOPENGL::ReadTextFile(const char* filepath)
{
    std::ifstream inp(filepath, std::ios::binary);

    if (inp)
    {
        std::string texts;
        inp.seekg(0, std::ios::end);
        texts.resize(inp.tellg());
        inp.seekg(0, std::ios::beg);
        inp.read(&texts[0], texts.size());
        inp.close();
        return texts;
    }
    else
    {
        std::cerr << "Error reading the text file :: " << filepath << "\n";
    }
}

GLuint FUSIONOPENGL::CompileVertShader(const char* vertexsource)
{
    GLuint vertexshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexshader, 1, &vertexsource, nullptr);
    glCompileShader(vertexshader);

    GLint status;
    glGetShaderiv(vertexshader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        char buffer[512];
        glGetShaderInfoLog(vertexshader, 512, nullptr, buffer);
        std::cerr << "Failed to compile vertex shader :: " << buffer << "\n";

    }

    return vertexshader;

}

GLuint FUSIONOPENGL::CompileGeoShader(const char* Geosource)
{
    GLuint Geoshader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(Geoshader, 1, &Geosource, nullptr);
    glCompileShader(Geoshader);

    GLint status;
    glGetShaderiv(Geoshader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        char buffer[512];
        glGetShaderInfoLog(Geoshader, 512, nullptr, buffer);
        std::cerr << "Failed to compile geometry shader :: " << buffer << "\n";

    }

    return Geoshader;
}

GLuint FUSIONOPENGL::CompileShaderProgram(GLuint vertexshader, GLuint geoshader, GLuint fragmentshader)
{
    GLuint m_program;
    m_program = glCreateProgram();
    glAttachShader(m_program, vertexshader);
    glAttachShader(m_program, fragmentshader);
    glAttachShader(m_program, geoshader);
    glLinkProgram(m_program);

    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);
    glDeleteShader(geoshader);

    GLint status;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetProgramInfoLog(m_program, 512, nullptr, buffer);
        std::cerr << "Failed to link program :: " << buffer << "\n";
    }

    return m_program;
}

GLuint FUSIONOPENGL::CompileFragShader(const char* fragmentsource)
{
    GLuint fragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentshader, 1, &fragmentsource, nullptr);
    glCompileShader(fragmentshader);

    GLint status;
    glGetShaderiv(fragmentshader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        char buffer[512];
        glGetShaderInfoLog(fragmentshader, 512, nullptr, buffer);
        std::cerr<< "Failed to compile fragment shader :: " << buffer << "\n";

    }

    return fragmentshader;
}

GLuint FUSIONOPENGL::CompileShaderProgram(GLuint vertexshader , GLuint fragmentshader)
{
    GLuint m_program;
    m_program = glCreateProgram();
    glAttachShader(m_program, vertexshader);
    glAttachShader(m_program, fragmentshader);
    glLinkProgram(m_program);

    glDeleteShader(vertexshader);
    glDeleteShader(fragmentshader);

    GLint status;
    glGetProgramiv(m_program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetProgramInfoLog(m_program, 512, nullptr, buffer);
        std::cerr << "Failed to link program :: " << buffer << "\n";
    }
    return m_program;
}

void FUSIONOPENGL::UseShaderProgram(GLuint program)
{
    glUseProgram(program);
}

void FUSIONOPENGL::DeleteShaderProgram(GLuint program)
{
    glDeleteProgram(program);
}

FUSIONOPENGL::Shader::Shader(const char* vertsourcepath, const char* fragsourcepath)
{
    std::string vertsource = ReadTextFile(vertsourcepath);
    std::string fragsource = ReadTextFile(fragsourcepath);

    GLuint vertexshader = CompileVertShader(vertsource.c_str());
    GLuint fragmentshader = CompileFragShader(fragsource.c_str());
    shaderID = CompileShaderProgram(vertexshader, fragmentshader);
}

FUSIONOPENGL::Shader::Shader(const char* vertsourcepath, const char* geosourcepath, const char* fragsourcepath)
{
    std::string vertsource = ReadTextFile(vertsourcepath);
    std::string geosource = ReadTextFile(geosourcepath);
    std::string fragsource = ReadTextFile(fragsourcepath);

    GLuint vertexshader = CompileVertShader(vertsource.c_str());
    GLuint fragmentshader = CompileFragShader(fragsource.c_str());
    GLuint geoshader = CompileGeoShader(geosource.c_str());
    shaderID = CompileShaderProgram(vertexshader,geoshader, fragmentshader);
}

GLuint FUSIONOPENGL::Shader::GetID()
{
    return shaderID;
}