/*
 * $Revision: 451 $ $Date: 2010-08-23 09:33:46 -0700 (Mon, 23 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_VERTEX_BUFFER_H_
#define _VESTA_VERTEX_BUFFER_H_

#include "Object.h"


namespace vesta
{
class GLVertexBuffer;

/** A VertexBuffer is a block of memory containing vertex data. The class
  * hides the underlying details: the block may be stored in system memory
  * or in GPU memory if the vertex_buffer_object extension is available.
  */
class VertexBuffer : public Object
{
public:
    enum UsagePattern
    {
        StaticDraw,
        StaticRead,
        StaticCopy,
        StreamDraw,
        StreamRead,
        StreamCopy,
        DynamicDraw,
        DynamicRead,
        DynamicCopy,
    };

private:
    VertexBuffer(unsigned int size, void* data = 0);
    VertexBuffer(unsigned int size, GLVertexBuffer* vbo);

public:
    ~VertexBuffer();

    /** Get the size of the vertex buffer in bytes.
      */
    unsigned int size() const
    {
        return m_size;
    }

    /** Get a pointer to the GLVertexBuffer object holding the vertex data. This will
      * return NULL if the vertex data is stored in heap memory instead of in a
      * vertex buffer object.
      */
    GLVertexBuffer* vbo() const
    {
        return m_vbo.ptr();
    }

    /** Return a pointer to the heap memory for the vertex buffer. This will be NULL
      * if the vertex data is stored in a vertex buffer object.
      */
    void* data() const
    {
        return m_data;
    }

    void* mapWriteOnly(bool discardContents = true);
    const void* mapReadOnly();
    void* mapReadWrite();
    bool unmap();

    static VertexBuffer *Create(unsigned int size, UsagePattern usage, const void* data = 0);

private:
    unsigned int m_size;
    counted_ptr<GLVertexBuffer> m_vbo;
    char* m_data;
};

}

#endif // _VESTA_VERTEX_BUFFER_H_
