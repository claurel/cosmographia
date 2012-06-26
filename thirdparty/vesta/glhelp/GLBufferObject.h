// GLVertexBuffer.h
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

#ifndef _VESTA_GL_BUFFER_OBJECT_H_
#define _VESTA_GL_BUFFER_OBJECT_H_

#include "../OGLHeaders.h"
#include "../Object.h"


namespace vesta
{

/** GLBufferObject is a C++ wrapper for OpenGL buffer objects handled by the
  *  vertex_buffer_object extension.
 */
class GLBufferObject : public Object
{
protected:
    GLBufferObject(GLenum target, unsigned int size, GLenum usage, const void* data = 0);

public:
    virtual ~GLBufferObject();

    GLuint handle() const
    {
        return m_handle;
    }

    bool isValid() const
    {
        return m_valid;
    }

    const void* mapReadOnly();
    void* mapWriteOnly(bool discardContents = true);
    void* mapReadWrite();

    bool unmap();
    bool isMapped() const
    {
        return m_isMapped;
    }

    void bind() const;
    void unbind() const;

    static bool supported();

private:
    void* map(GLenum access);

private:
    GLenum m_target;
    GLuint m_handle;
    unsigned int m_size;
    GLenum m_usage;
    bool m_valid;
    bool m_isMapped;
};

}

#endif // _VESTA_GL_BUFFER_OBJECT_H_
