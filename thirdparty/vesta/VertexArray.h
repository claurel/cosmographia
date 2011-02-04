/*
 * $Revision: 368 $ $Date: 2010-07-19 17:29:08 -0700 (Mon, 19 Jul 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_VERTEX_ARRAY_H_
#define _VESTA_VERTEX_ARRAY_H_

#include "VertexSpec.h"
#include "BoundingBox.h"

namespace vesta
{

class VertexArray
{
public:
    VertexArray(void* data,
                unsigned int count,
                const VertexSpec& vertexSpec,
                unsigned int stride = 0);
    ~VertexArray();

    void* data() const
    {
        return m_data;
    }

    unsigned int count() const
    {
        return m_count;
    }

    const VertexSpec& vertexSpec() const
    {
        return m_vertexSpec;
    }

    unsigned int stride() const
    {
        return m_stride;
    }

    BoundingBox computeBoundingBox() const;

    float computeBoundingSphereRadius() const;

    Eigen::Vector3f position(unsigned int index) const;

    /** Get a pointer to the data for the specified vertex.
      */
    VertexAttribute::Component* vertex(unsigned int index) const
    {
        return reinterpret_cast<VertexAttribute::Component*>(m_data) + index * (m_stride >> 2);
    }

private:
    void* m_data;
    unsigned int m_count;
    VertexSpec m_vertexSpec;
    unsigned int m_stride;
};

}

#endif // _VESTA_VERTEX_ARRAY_H_
