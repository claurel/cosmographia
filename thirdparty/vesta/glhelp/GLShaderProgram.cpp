// GLShaderProgram.cpp
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// VESTA is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// VESTA is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// VESTA. If not, see <http://www.gnu.org/licenses/>.

#include "GLShaderProgram.h"
#include "../Debug.h"
#include "../Object.h"

using namespace vesta;
using namespace std;

#ifdef VESTA_OGLES2
#define glGetUniformLocationARB glGetUniformLocation
#define glBindAttribLocationARB glBindAttribLocation
#endif


GLShaderProgram::GLShaderProgram() :
    m_handle(0),
    m_isLinked(false)
{
#ifdef VESTA_OGLES2
    m_handle = glCreateProgram();
#else
    m_handle = glCreateProgramObjectARB();
#endif
}


GLShaderProgram::~GLShaderProgram()
{
    if (m_handle != 0)
    {
#ifdef VESTA_OGLES2
        glDeleteProgram(m_handle);
#else
        glDeleteObjectARB(m_handle);
#endif
    }
}


bool
GLShaderProgram::addShader(GLShader* shader)
{
    switch (shader->stage())
    {
    case GLShader::VertexStage:
        m_vertexShader = shader;
        break;
    case GLShader::FragmentStage:
        m_fragmentShader = shader;
        break;
    default:
        return false;
    }

    return true;
}


bool
GLShaderProgram::link()
{
    if (m_isLinked)
    {
        // Oops--already linked
        //return false;
    }

    if (m_vertexShader.isNull() || m_fragmentShader.isNull())
    {
        return false;
    }

    if (!m_vertexShader->isCompiled() || !m_fragmentShader->isCompiled())
    {
        return false;
    }

    if (m_handle == 0)
    {
        return false;
    }

    GLint status = 0;
    
#ifdef VESTA_OGLES2
    glAttachShader(m_handle, m_vertexShader->glHandle());
    glAttachShader(m_handle, m_fragmentShader->glHandle());
    glLinkProgram(m_handle);
    glGetProgramiv(m_handle, GL_LINK_STATUS, &status);
#else
    glAttachObjectARB(m_handle, m_vertexShader->glHandle());
    glAttachObjectARB(m_handle, m_fragmentShader->glHandle());
    glLinkProgramARB(m_handle);
    glGetObjectParameterivARB(m_handle, GL_OBJECT_LINK_STATUS_ARB, &status);
#endif

    if (status == GL_TRUE)
    {
        m_isLinked = true;
    }

    // Get the log of error and warning messages and store it with
    // this shader objects.
    GLint length = 0;
#ifdef VESTA_OGLES2
    glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &length);
#else
    glGetObjectParameterivARB(m_handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
#endif
    if (length > 0)
    {
        GLsizei charCount = 0;
        char* linkLogChars = new char[length];
#ifdef VESTA_OGLES2
        glGetProgramInfoLog(m_handle, length, &charCount, linkLogChars);
#else
        glGetInfoLogARB(m_handle, length, &charCount, linkLogChars);
#endif

        m_log = string(linkLogChars, charCount);
        delete[] linkLogChars;
    }

    return m_isLinked;
}


void
GLShaderProgram::bindAttribute(const char* name, int location)
{
    glBindAttribLocationARB(m_handle, location, name);
}


void
GLShaderProgram::setSampler(const char* name, unsigned int samplerIndex)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniform1i(location, samplerIndex);
    }
}


/** Set the value of a GLSL shader program uniform with a scalar float type
  */
void
GLShaderProgram::setConstant(const char* name, float value)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniform1f(location, value);
    }
}


/** Set the value of a GLSL shader program uniform with a vec2 type
  */
void
GLShaderProgram::setConstant(const char* name, const Eigen::Vector2f& value)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniform2fv(location, 1, value.data());
    }
}


/** Set the value of a GLSL shader program uniform with a vec3 type
  */
void
GLShaderProgram::setConstant(const char* name, const Eigen::Vector3f& value)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniform3fv(location, 1, value.data());
    }
}


/** Set the value of a GLSL shader program uniform with a vec4 type
  */
void
GLShaderProgram::setConstant(const char* name, const Eigen::Vector4f& value)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniform4fv(location, 1, value.data());
    }
}


/** Set the value of a GLSL shader program uniform with a 2x2 matrix (mat2) type
  */
void
GLShaderProgram::setConstant(const char* name, const Eigen::Matrix2f& value)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniformMatrix2fv(location, 1, GL_FALSE, value.data());
    }
}


/** Set the value of a GLSL shader program uniform with a 3x3 matrix (mat3) type
  */
void
GLShaderProgram::setConstant(const char* name, const Eigen::Matrix3f& value)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniformMatrix3fv(location, 1, GL_FALSE, value.data());
    }
}


/** Set the value of a GLSL shader program uniform with a 4x4 matrix (mat4) type
  */
void
GLShaderProgram::setConstant(const char* name, const Eigen::Matrix4f& value)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniformMatrix4fv(location, 1, GL_FALSE, value.data());
    }
}


/** Set the value of a GLSL shader program uniform with a color (vec3) type
  */
void
GLShaderProgram::setConstant(const char* name, const Spectrum& color)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniform3fv(location, 1, color.data());
    }
}


/** Set the value of a GLSL shader program uniform with a float array type.
  */
void
GLShaderProgram::setConstantArray(const char* name, const float values[], unsigned int count)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniform1fv(location, count, values);
    }
}


/** Set the value of a GLSL shader program uniform with a vec2 array type.
  */
void
GLShaderProgram::setConstantArray(const char* name, const Eigen::Vector2f values[], unsigned int count)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniform2fv(location, count, values[0].data());
    }
}


/** Set the value of a GLSL shader program uniform with a vec3 array type.
  */
void
GLShaderProgram::setConstantArray(const char* name, const Eigen::Vector3f values[], unsigned int count)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniform3fv(location, count, values[0].data());
    }
}


/** Set the value of a GLSL shader program uniform with a vec4 array type.
  */
void
GLShaderProgram::setConstantArray(const char* name, const Eigen::Vector4f values[], unsigned int count)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniform4fv(location, count, values[0].data());
    }
}


/** Set the value of a GLSL shader program uniform with a 4x4 matrix array type.
  */
void
GLShaderProgram::setConstantArray(const char* name, const Eigen::Matrix4f values[], unsigned int count)
{
    GLint location = glGetUniformLocationARB(m_handle, name);
    if (location >= 0)
    {
        glUniformMatrix4fv(location, count, GL_FALSE, values[0].data());
    }
}


/** Create a shader program using the specified vertex and fragment
  * shader source strings.
  *
  * \param vertexShaderSource string containing GLSL code for the vertex shader
  * \param fragmentShaderSource string containing GLSL code for the fragment shader
  *
  * \return a pointer to a new shader program if successful, null otherwise.
  */
GLShaderProgram*
GLShaderProgram::CreateShaderProgram(const string& vertexShaderSource,
                                     const string& fragmentShaderSource)
{
    counted_ptr<GLShader> vertexShader(new GLShader(GLShader::VertexStage));
    if (!vertexShader->compile(vertexShaderSource.c_str()))
    {
        VESTA_WARNING("Error creating vertex shader:");
        VESTA_WARNING("Error message(s):\n%s", vertexShader->compileLog().c_str());
        return NULL;
    }
    else if (!vertexShader->compileLog().empty())
    {
        VESTA_LOG("Vertex shader compile messages:\n%s", vertexShader->compileLog().c_str());
    }

    // Compile the fragment shader
    counted_ptr<GLShader> fragmentShader(new GLShader(GLShader::FragmentStage));
    if (!fragmentShader->compile(fragmentShaderSource.c_str()))
    {
        VESTA_WARNING("Error creating fragment shader:");
        VESTA_WARNING("Error message(s):\n%s", fragmentShader->compileLog().c_str());
        return NULL;
    }
    else if (!fragmentShader->compileLog().empty())
    {
        VESTA_LOG("Vertex shader compile messages:\n%s", fragmentShader->compileLog().c_str());
    }

    // Attach the vertex and fragment shaders
    GLShaderProgram* shaderProgram = new GLShaderProgram();
    shaderProgram->addShader(vertexShader.ptr());
    shaderProgram->addShader(fragmentShader.ptr());

    // Link the shader program
    if (!shaderProgram->link())
    {
        VESTA_WARNING("Error linking shader program:");
        VESTA_WARNING("Error message(s):\n%s", shaderProgram->log().c_str());
        delete shaderProgram;
        // vertex and fragment shaders automatically deleted along with program
    }
    else if (!shaderProgram->log().empty())
    {
        VESTA_LOG("Shader program link messages:\n%s", shaderProgram->log().c_str());
    }

    return shaderProgram;
}

