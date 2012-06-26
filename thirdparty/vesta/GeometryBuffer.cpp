/*
 * $Revision: 572 $ $Date: 2011-03-16 15:28:27 -0700 (Wed, 16 Mar 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "GeometryBuffer.h"
#include "VertexBuffer.h"
#include "RenderContext.h"
#include "Debug.h"

using namespace vesta;
using namespace Eigen;


/** Create a new GeometryBuffer attached to the specified render context.
  */
GeometryBuffer::GeometryBuffer(RenderContext* rc) :
    m_vb(rc->vertexStreamBuffer()),
    m_rc(rc),
    m_vertexData(NULL),
    m_primitiveType(PrimitiveBatch::Points),
    m_vertexCount(0),
    m_vertexCapacity(0),
    m_inBeginEnd(0)
{
}


GeometryBuffer::~GeometryBuffer()
{
    if (m_rc)
    {
        m_rc->unbindVertexBuffer();
    }

    if (m_vertexData)
    {
        m_vb->unmap();
        m_vertexData = NULL;
    }
}


// Compute the number of primitives represented by the specified number of vertices
static unsigned int CalcPrimitiveCount(PrimitiveBatch::PrimitiveType type, unsigned int vertexCount)
{
    switch (type)
    {
    case PrimitiveBatch::Triangles:
        return vertexCount / 3;
        break;
    case PrimitiveBatch::Lines:
        return vertexCount / 2;
        break;
    case PrimitiveBatch::TriangleStrip:
    case PrimitiveBatch::TriangleFan:
        return vertexCount < 3 ? 0 : vertexCount - 2;
        break;
    case PrimitiveBatch::LineStrip:
        return vertexCount < 2 ? 0 : vertexCount - 1;
        break;
    default:
        return vertexCount;
        break;
    }
}


/** Start a new group of the specified primitive type, much like
  * glBegin(). This method should not be called within a current
  * begin/end pair.
  */
void
GeometryBuffer::begin(PrimitiveBatch::PrimitiveType primType)
{
    if (m_inBeginEnd)
    {
        VESTA_WARNING("GeometryBuffer: begin before end");
        return;
    }

    m_inBeginEnd = true;

    m_primitiveType = primType;
    m_vertexData = reinterpret_cast<char*>(m_vb->mapWriteOnly());
    m_vertexCount = 0;

    unsigned int stride = 12;
    m_vertexCapacity = m_vb->size() / stride;

    // Round down to the nearest multiple of six. This will ensure that the
    // buffer is always flushed on a primitive boundary, since six is the least
    // common multiple of 2 (vertices per line) and 3 (vertices per triangle).
    m_vertexCapacity = (m_vertexCapacity / 6) * 6;
}


/** Start a new set of points.
 */
void
GeometryBuffer::beginPoints()
{
    begin(PrimitiveBatch::Points);
}


/** Start a new set of lines.
 */
void
GeometryBuffer::beginLines()
{
    begin(PrimitiveBatch::Lines);
}


/** Start a new line strip.
 */
void
GeometryBuffer::beginLineStrip()
{
    begin(PrimitiveBatch::LineStrip);
}


/** Start a new set of triangles.
 */
void
GeometryBuffer::beginTriangles()
{
    begin(PrimitiveBatch::TriangleStrip);
}


/** Start a new triangle strip.
  */
void
GeometryBuffer::beginTriangleStrip()
{
    begin(PrimitiveBatch::TriangleStrip);
}


/** End the current primitive set. It is illegal to call
 *  end without first calling begin.
 */
void
GeometryBuffer::end()
{
    if (!m_inBeginEnd)
    {
        VESTA_WARNING("GeometryBuffer: end without begin");
        return;
    }
    m_inBeginEnd = false;

    flush();
}


// Draw the contents of the vertex buffer and reset it. Flush is called at the
// end of a primitive set and when the vertex buffer is filled.
void
GeometryBuffer::flush()
{
    unsigned int primitiveCount = CalcPrimitiveCount(m_primitiveType, m_vertexCount);

    if (m_vertexCount > 0)
    {
        if (m_vb->unmap())
        {
            if (m_rc)
            {
                m_rc->bindVertexBuffer(VertexSpec::Position, m_vb.ptr(), VertexSpec::Position.size());
                m_rc->drawPrimitives(PrimitiveBatch(m_primitiveType, primitiveCount, 0));
            }
        }

        m_vertexCount = 0;
        m_vertexData = NULL;
    }
}


/** Specify a vertex. Must be called within begin/end.
  */
void
GeometryBuffer::vertex(const Eigen::Vector3f& v)
{
    if (m_vertexCount == m_vertexCapacity)
    {
        flush();
        m_vertexData = reinterpret_cast<char*>(m_vb->mapWriteOnly());
        if (!m_vertexData)
        {
            m_vertexCapacity = 0;
        }

        if (m_vertexCapacity == 0)
        {
            return;
        }
    }

    Map<Vector3f>(reinterpret_cast<float*>(m_vertexData) + 3 * m_vertexCount) = v;
    ++m_vertexCount;
}


/** Specify a double precision vertex. Must be called within begin/end.
  *
  * Note: double precision values are simply cast to single precision.
  */
void
GeometryBuffer::vertex(const Eigen::Vector3d& v)
{
    vertex(Vector3f(v.cast<float>()));
}


/** Specify a vertex. Must be called within begin/end.
  */
void
GeometryBuffer::vertex(float x, float y, float z)
{
    vertex(Vector3f(x, y, z));
}
