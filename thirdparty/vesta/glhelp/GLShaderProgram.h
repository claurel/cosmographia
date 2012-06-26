// GLShaderProgram.h
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

#ifndef _VESTA_GL_SHADER_PROGRAM_H_
#define _VESTA_GL_SHADER_PROGRAM_H_

#include "GLShader.h"
#include "../Spectrum.h"
#include <string>


namespace vesta
{

/** GLShaderProgram is a C++ wrapper for OpenGL shader program
 *  objects.
 */
class GLShaderProgram : public Object
{
public:
    GLShaderProgram();
    ~GLShaderProgram();

    bool addShader(GLShader* shader);
    bool link();

    GLShader* vertexShader() const
    {
        return m_vertexShader.ptr();
    }

    GLShader* fragmentShader() const
    {
        return m_fragmentShader.ptr();
    }

    /** Get the OpenGL handle for this shader.
      */
    GLhandleARB glHandle() const
    {
        return m_handle;
    }

    /** Get the log of warning and error messages from the
      * GLSL shader linker.
      */
    std::string log() const
    {
        return m_log;
    }

    bool isLinked() const
    {
        return m_isLinked;
    }

    void bind() const
    {
#ifdef VESTA_OGLES2
        glUseProgram(m_handle);
#else
        glUseProgramObjectARB(m_handle);
#endif
    }

    void bindAttribute(const char* name, int location);

    void setSampler(const char* name, unsigned int samplerIndex);
    void setConstant(const char* name, float value);
    void setConstant(const char* name, const Eigen::Vector2f& value);
    void setConstant(const char* name, const Eigen::Vector3f& value);
    void setConstant(const char* name, const Eigen::Vector4f& value);
    void setConstant(const char* name, const Eigen::Matrix2f& value);
    void setConstant(const char* name, const Eigen::Matrix3f& value);
    void setConstant(const char* name, const Eigen::Matrix4f& value);
    void setConstant(const char* name, const Spectrum& color);
    void setConstantArray(const char* name, const float values[], unsigned int count);
    void setConstantArray(const char* name, const Eigen::Vector2f values[], unsigned int count);
    void setConstantArray(const char* name, const Eigen::Vector3f values[], unsigned int count);
    void setConstantArray(const char* name, const Eigen::Vector4f values[], unsigned int count);
    void setConstantArray(const char* name, const Eigen::Matrix4f values[], unsigned int count);

    static GLShaderProgram* CreateShaderProgram(const std::string& vertexShaderSource,
                                                const std::string& fragmentShaderSource);

private:
    GLhandleARB m_handle;
    counted_ptr<GLShader> m_vertexShader;
    counted_ptr<GLShader> m_fragmentShader;
    std::string m_log;
    bool m_isLinked;
};

}

#endif // _VESTA_GL_SHADER_PROGRAM_H_
