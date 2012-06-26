// GLShader.h
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

#ifndef _VESTA_GL_SHADER_H_
#define _VESTA_GL_SHADER_H_

#include "../Object.h"
#include "../OGLHeaders.h"
#include <string>

// GLhandle type not present in OpenGL ES
#if defined(VESTA_OGLES2)
#define GLhandleARB GLuint
#endif

namespace vesta
{

/** GLShader is a C++ wrapper for OpenGL shader objects.
 */
class GLShader : public Object
{
public:
    enum ShaderStage
    {
        VertexStage,
        FragmentStage,
    };

    GLShader(ShaderStage stage);
    ~GLShader();

    ShaderStage stage() const
    {
        return m_stage;
    }

    GLhandleARB glHandle() const
    {
        return m_handle;
    }

    bool compile(const std::string& source);

    /** Get the message log from the OpenGL shader compiler. This
      * will return an empty string if the shader hasn't been compiled
      * yet.
      */
    std::string compileLog() const
    {
        return m_compileLog;
    }

    bool isCompiled() const
    {
        return m_isCompiled;
    }

private:
    ShaderStage m_stage;
    GLhandleARB m_handle;
    bool m_isCompiled;
    std::string m_compileLog;
};

}

#endif // _VESTA_GL_SHADER_H_
