/*
 * $Revision: 407 $ $Date: 2010-08-03 14:35:35 -0700 (Tue, 03 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "VertexArray.h"
#include <algorithm>
#include <cmath>
#include <cassert>

using namespace vesta;
using namespace Eigen;


static bool is4ByteAligned(int x)
{
    return (x & 0x3) == 0;
}

/** Create a new VertexArray wrapping the given data buffer. The
 *  @param data a pointer to the vertex data (which will be owned by the vertex buffer)
 *  @param vertexSpec a valid vertex specification
 *  @param stride the spacing between consecutive vertices (in bytes); if zero, the stride will be automatically computed from the vertex spec.
 */
VertexArray::VertexArray(void* data,
                         unsigned int count,
                         const VertexSpec& vertexSpec,
                         unsigned int stride) :
    m_data(data),
    m_count(count),
    m_vertexSpec(vertexSpec),
    m_stride(stride)
{
    assert(stride == 0 || stride >= vertexSpec.size());
    assert(is4ByteAligned(stride));

    if (stride == 0)
    {
        m_stride = vertexSpec.size();
    }
}

    
VertexArray::~VertexArray()
{
    delete[] static_cast<char*>(m_data);
}


BoundingBox
VertexArray::computeBoundingBox() const
{
    BoundingBox bbox;

    int positionIndex = m_vertexSpec.attributeIndex(VertexAttribute::Position);
    assert(positionIndex >= 0);
    if (positionIndex >= 0)
    {
        const float* floatData = reinterpret_cast<float*>(m_data) + m_vertexSpec.attributeOffset(positionIndex) / 4;
        unsigned int step = m_stride / 4;

        if (m_count > 0)
        {
            // Initialize bounding box with first point
            bbox = BoundingBox(Map<Vector3f>(floatData), Map<Vector3f>(floatData));
            floatData += step;
        }

        for (unsigned int i = 1; i < m_count; ++i)
        {
            bbox.include(Map<Vector3f>(floatData));
            floatData += step;
        }
    }

    return bbox;
}


float
VertexArray::computeBoundingSphereRadius() const
{
    float maxDistSquared = 0.0f;

    unsigned int positionIndex = m_vertexSpec.attributeIndex(VertexAttribute::Position);
    assert(positionIndex != VertexSpec::InvalidAttribute);
    if (positionIndex != VertexSpec::InvalidAttribute)
    {
        const float* floatData = reinterpret_cast<float*>(m_data) + m_vertexSpec.attributeOffset(positionIndex) / 4;
        unsigned int step = m_stride / 4;

        for (unsigned int i = 0; i < m_count; ++i)
        {
            maxDistSquared = std::max(maxDistSquared, Map<Vector3f>(floatData).squaredNorm());
            floatData += step;
        }
    }

    return std::sqrt(maxDistSquared);
}


/** Return the position of the vertex at the specified index. Index
  * must be less than the vertex count.
  */
Vector3f
VertexArray::position(unsigned int index) const
{
    int positionIndex = m_vertexSpec.attributeIndex(VertexAttribute::Position);
    assert(positionIndex >= 0);
    assert(index < m_count);

    const float* floatData = reinterpret_cast<float*>(m_data) + m_vertexSpec.attributeOffset(positionIndex) / 4;
    unsigned int stride = m_stride / 4;

    return Map<Vector3f>(floatData + stride * index);
}
