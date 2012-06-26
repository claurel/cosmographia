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

#ifndef _VESTA_GL_VERTEX_BUFFER_H_
#define _VESTA_GL_VERTEX_BUFFER_H_

#include "../OGLHeaders.h"
#include "GLBufferObject.h"


namespace vesta
{

/** GLVertexBuffer is a C++ wrapper for OpenGL vertex buffer objects.
 */
class GLVertexBuffer : public GLBufferObject
{
public:
    GLVertexBuffer(unsigned int size, GLenum usage, const void* data = 0);
    ~GLVertexBuffer();
};

}

#endif // _VESTA_GL_VERTEX_BUFFER_H_
