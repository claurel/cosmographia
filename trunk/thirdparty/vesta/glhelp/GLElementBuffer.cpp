// GLElementBuffer.cpp
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

#include "GLElementBuffer.h"

using namespace vesta;


/** Create a new vertex buffer with the specified size and usage. If
  *  data is not null, the memory pointed to by data will be used to
  *  initialize the buffer. Otherwise, the initial contents of the buffer
  *  are undefined.
  */
GLElementBuffer::GLElementBuffer(unsigned int size, GLenum usage, const void* data) :
    GLBufferObject(GL_ELEMENT_ARRAY_BUFFER, size, usage, data)
{
}


GLElementBuffer::~GLElementBuffer()
{
}
