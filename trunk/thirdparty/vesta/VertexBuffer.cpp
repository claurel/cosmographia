/*
 * $Revision: 451 $ $Date: 2010-08-23 09:33:46 -0700 (Mon, 23 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "VertexBuffer.h"
#include "Debug.h"
#include "glhelp/GLVertexBuffer.h"
#include <algorithm>

using namespace vesta;


// Create a vertex buffer with data stored in heap memory.
// This constructor is private; use VertexBuffer::Create() instead.
VertexBuffer::VertexBuffer(unsigned int size, void* data) :
    m_size(size),
    m_data(reinterpret_cast<char*>(data))
{
}


// Create a vertex buffer with data stored in an OpenGL vertex
// buffer object.
// This constructor is private; use VertexBuffer::Create() instead.
VertexBuffer::VertexBuffer(unsigned int size, GLVertexBuffer* vbo) :
    m_size(size),
    m_vbo(vbo),
    m_data(NULL)
{
}


VertexBuffer::~VertexBuffer()
{
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
VertexBuffer::mapWriteOnly(bool discardContents)
{
    if (m_vbo.isValid())
    {
        return m_vbo->mapWriteOnly(discardContents);
    }
    else
    {
        return m_data;
    }
}


/** Map a vertex buffer for read-only access. Returns a pointer to the
  *  buffer contents mapped into memory or null if there was an error.
  *  map() returns null if the buffer is already mapped.
  */
const void*
VertexBuffer::mapReadOnly()
{
    if (m_vbo.isValid())
    {
        return m_vbo->mapReadOnly();
    }
    else
    {
        return m_data;
    }
}


/** Map a vertex buffer for read-write access. Returns a pointer to the
  *  buffer contents mapped into memory or null if there was an error.
  *  map() returns null if the buffer is already mapped.
  */
void*
VertexBuffer::mapReadWrite()
{
    if (m_vbo.isValid())
    {
        return m_vbo->mapReadWrite();
    }
    else
    {
        return m_data;
    }
}


/** Unmap the buffer. Returns false if the buffer data was lost while the
  * buffer was mapped (which means that the buffer shouldn't be drawn, since
  * it contains undefined data.)
  */
bool
VertexBuffer::unmap()
{
    if (m_vbo.isValid())
    {
        return m_vbo->unmap();
    }
    else
    {
        return true;
    }
}


/** Create a new vertex buffer object. This function should only be called after
  * an OpenGL context has been created and made current.
  *
  * The specified data is copied into the vertex buffer. If the data pointer is
  * NULL, the contents of the vertex buffer are left uninitialized.
  */
VertexBuffer*
VertexBuffer::Create(unsigned int size, UsagePattern usage, const void* data)
{
    GLenum glUsage = GL_STATIC_DRAW;
#ifdef VESTA_OGLES2
    switch (usage)
    {
    case StaticDraw:
        glUsage = GL_STATIC_DRAW;
        break;
    case StreamDraw:
        glUsage = GL_STREAM_DRAW;
        break;
    case DynamicDraw:
        glUsage = GL_DYNAMIC_DRAW;
        break;
    case StaticRead:
    case StreamRead:
    case DynamicRead:
        VESTA_WARNING("'Read' usage pattern not supported for buffers in OpenGL ES");
        return NULL;
    case StaticCopy:
    case StreamCopy:
    case DynamicCopy:
        VESTA_WARNING("'Copy' usage pattern not supported for buffers in OpenGL ES");
        return NULL;
    }
#else
    switch (usage)
    {
    case StaticDraw:
        glUsage = GL_STATIC_DRAW;
        break;
    case StaticRead:
        glUsage = GL_STATIC_READ;
        break;
    case StaticCopy:
        glUsage = GL_STATIC_COPY;
        break;
    case StreamDraw:
        glUsage = GL_STREAM_DRAW;
        break;
    case StreamRead:
        glUsage = GL_STREAM_READ;
        break;
    case StreamCopy:
        glUsage = GL_STREAM_COPY;
        break;
    case DynamicDraw:
        glUsage = GL_DYNAMIC_DRAW;
        break;
    case DynamicRead:
        glUsage = GL_DYNAMIC_READ;
        break;
    case DynamicCopy:
        glUsage = GL_DYNAMIC_COPY;
        break;
    }
#endif

    VertexBuffer* vb = NULL;

    if (GLBufferObject::supported())
    {
        GLVertexBuffer* vbo = new GLVertexBuffer(size, glUsage, data);
        if (vbo)
        {
            vb = new VertexBuffer(size, vbo);
        }
    }
    else
    {
        char* vertexData = new char[size];
        if (vertexData)
        {
            if (data)
            {
                std::copy(reinterpret_cast<const char*>(data), reinterpret_cast<const char*>(data) + size, vertexData);
            }
            vb = new VertexBuffer(size, vertexData);
        }
    }

    if (!vb)
    {
        VESTA_WARNING("Error creating vertex buffer");
    }

    return vb;
}
