// GLShader.cpp
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

#include "GLShader.h"

using namespace vesta;
using namespace std;

#ifdef VESTA_OGLES2
#define glDeleteObjectARB glDeleteObject
//#define glCreateShaderObjectARB glCreateShaderObject
#define glShaderSourceARB glShaderSource
#define glCompileShaderARB glCompileShader

#define GL_VERTEX_SHADER_ARB GL_VERTEX_SHADER
#define GL_FRAGMENT_SHADER_ARB GL_FRAGMENT_SHADER
#define GL_OBJECT_INFO_LOG_LENGTH_ARB GL_OBJECT_INFO_LOG_LENGTH
#endif



GLShader::GLShader(ShaderStage stage) :
    m_stage(stage),
    m_handle(0),
    m_isCompiled(false)
{
}


GLShader::~GLShader()
{
    if (m_handle != 0)
    {
#ifdef VESTA_OGLES2
        glDeleteShader(m_handle);
#else
        glDeleteObjectARB(m_handle);
#endif
    }
}


/** Compile the GLSL source. Return true if the compilation
  * was successful, false if there was an error. The list of
  * warnings and errors may be retrieved with the compileLog()
  * method. Compile() may only be called a single time;
  * subsequent calls will return false and have no effect.
  */
bool
GLShader::compile(const string& source)
{
    if (isCompiled())
    {
        return false;
    }

#ifdef VESTA_OGLES2
    if (stage() == VertexStage)
    {
        m_handle = glCreateShader(GL_VERTEX_SHADER);
    }
    else if (stage() == FragmentStage)
    {
        m_handle = glCreateShader(GL_FRAGMENT_SHADER);
    }
#else
    if (stage() == VertexStage)
    {
        m_handle = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
    }
    else if (stage() == FragmentStage)
    {
        m_handle = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
    }
#endif
    
    if (m_handle == 0)
    {
        // Unable to generate a shader object handle
        return false;
    }

    // Set the source code and tell OpenGL to compile it.
    const char* sourceString = source.c_str();
    GLint sourceStringLength = source.length();
    glShaderSourceARB(m_handle, 1, &sourceString, &sourceStringLength);
    glCompileShaderARB(m_handle);

    // Get the log of error and warning messages and store it with
    // this shader objects.
    GLint length = 0;
#ifdef VESTA_OGLES2
    glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &length);
#else
    glGetObjectParameterivARB(m_handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
#endif
    if (length > 0)
    {
        GLsizei charCount = 0;
        char* compileLogChars = new char[length];
#ifdef VESTA_OGLES2
        glGetShaderInfoLog(m_handle, length, &charCount, compileLogChars);
#else
        glGetInfoLogARB(m_handle, length, &charCount, compileLogChars);
#endif

        m_compileLog = string(compileLogChars, charCount);
        delete[] compileLogChars;
    }

    // Find out whether the compilation was successful
    GLint status = GL_FALSE;
#ifdef VESTA_OGLES2
    glGetShaderiv(m_handle, GL_COMPILE_STATUS, &status);
#else
    glGetObjectParameterivARB(m_handle, GL_OBJECT_COMPILE_STATUS_ARB, &status);
#endif

    m_isCompiled = status == GL_TRUE;

    return m_isCompiled;
}
