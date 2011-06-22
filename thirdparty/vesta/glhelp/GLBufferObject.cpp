// GLBufferObject.cpp
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

#include "GLBufferObject.h"
#include "../Debug.h"
#include <cstddef>

using namespace vesta;


/** Create a new vertex buffer with the specified size and usage. If
  * data is not null, the memory pointed to by data will be used to
  * initialize the buffer. Otherwise, the initial contents of the buffer
  * are undefined.
  *
  * Constructing a GLBufferObject has the side effect of unbinding any
  * currently bound buffer object of the same type.
  */
GLBufferObject::GLBufferObject(GLuint target, unsigned int size, GLenum usage, const void* data) :
    m_target(target),
    m_handle(0),
    m_size(size),
    m_usage(usage),
    m_valid(false),
    m_isMapped(false)
{
    if (supported())
    {
        glGenBuffers(1, &m_handle);
        if (m_handle != 0)
        {
            // Clear the error state
            (void) glGetError();

            // Initialize the buffer
            glBindBuffer(m_target, m_handle);
            glBufferData(m_target, size, data, usage);

            // Check for errors
            GLenum err = glGetError();

            glBindBuffer(m_target, 0);

            if (err != GL_NO_ERROR)
            {
                switch (err)
                {
                case GL_INVALID_ENUM:
                    VESTA_WARNING("Bad usage for GL buffer object.");
                    break;
                case GL_OUT_OF_MEMORY:
                    VESTA_WARNING("Out of memory error when allocating GL buffer object.");
                    break;
                default:
                    VESTA_WARNING("Unknown error initializing GL buffer object.");
                    break;
                }

                glDeleteBuffers(1, &m_handle);
                m_handle = 0;
            }
            else
            {
                m_valid = true;
            }
        }
        else
        {
            VESTA_WARNING("GLBufferObject creation failed.");
        }
    }
    else
    {
        VESTA_WARNING("Attempted to create GLBufferObject , but vertex_buffer_object extension isn't supported.");
    }
}


GLBufferObject::~GLBufferObject()
{
    if (m_handle != 0)
    {
        glDeleteBuffers(1, &m_handle);
    }
}


void
GLBufferObject::bind() const
{
    if (m_valid)
    {
        glBindBuffer(m_target, m_handle);
    }
}


void
GLBufferObject::unbind() const
{
    glBindBuffer(m_target, 0);
}


// Internal method. Called by mapReadOnly, mapWriteOnly, and mapReadWrite.
void*
GLBufferObject::map(GLenum access)
{
    void* data = NULL;

    if (m_isMapped)
    {
        VESTA_WARNING("Attempted to map an already mapped buffer object.");
    }
    else
    {
        bind();
        data = glMapBuffer(m_target, access);
        m_isMapped = true;
    }

    return data;
}


/** Unmap the buffer. Returns false if the buffer data was lost while the
  * buffer was mapped (which means that the buffer shouldn't be drawn, since
  * it contains undefined data.)
  */
bool
GLBufferObject::unmap()
{
    if (!m_isMapped)
    {
        VESTA_WARNING("Attempted to unmap a buffer that isn't mapped.");
        return true;
    }
    else
    {
        bind();
        bool lost = glUnmapBuffer(m_target) == GL_FALSE;
        m_isMapped = false;

        return !lost;
    }
}


/** Map a vertex buffer for read-only access. Returns a pointer to the
  *  buffer contents mapped into memory or null if there was an error.
  *  map() returns null if the buffer is already mapped.
  */
const void*
GLBufferObject::mapReadOnly()
{
    return map(GL_READ_ONLY);
}


/** Map a buffer for write-only access. Returns a pointer to the
  *  buffer contents mapped into memory or null if there was an error.
  *  map() returns null if the buffer is already mapped.
  *
  * The discardContents flag specifies whether the contents should be
  * preserved (discardContents = false) or thrown away. Better performance
  * is possible when contents are discarded, as this allows the driver
  * to optimize GPU/CPU parallelism through buffer renaming.
  */
void*
GLBufferObject::mapWriteOnly(bool discardContents)
{
    if (discardContents)
    {
        bind();

        // The ARB_vertex_buffer_object spec says specifying NULL data is a
        // hint to the GL driver that the buffer data doesn't need to be
        // preserved.
        glBufferData(m_target, m_size, NULL, m_usage);
    }
    return map(GL_WRITE_ONLY);
}


/** Map a vertex buffer for read-write access. Returns a pointer to the
  *  buffer contents mapped into memory or null if there was an error.
  *  map() returns null if the buffer is already mapped.
  */
void*
GLBufferObject::mapReadWrite()
{
    return map(GL_READ_WRITE);
}


/** Return true if vertex buffer objects are supported by the current
  *  OpenGL context.
  */
bool
GLBufferObject::supported()
{
    // Require OpenGL version 1.5 for vertex buffer support. There are some
    // some pre-1.5 drivers could support vertex buffers with the GLEW_ARB_vertex_buffer_object
    // extension, but these are very now.

    // The following test detects the ARB extension; however, all of the buffer object
    // functions would need to be changed to their ARB equivalents if we relied this test.
    //return GLEW_ARB_vertex_buffer_object == GL_TRUE;

    return GLEW_VERSION_1_5 == GL_TRUE;
}
