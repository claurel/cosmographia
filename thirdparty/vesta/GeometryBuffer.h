/*
 * $Revision: 572 $ $Date: 2011-03-16 15:28:27 -0700 (Wed, 16 Mar 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_GEOMETRY_BUFFER_H_
#define _VESTA_GEOMETRY_BUFFER_H_

#include <Eigen/Core>
#include "Object.h"
#include "PrimitiveBatch.h"

namespace vesta
{
class VertexBuffer;
class RenderContext;

/** GeometryBuffer is used to draw using commands similar to the legacy
  * OpenGL immediate mode commands: glBegin, glVertex, glNormal, etc. It should
  * be used instead of immediate mode, since the immediate mode functions are
  * deprecated in OpenGL 3.0+ and are not available at all in OpenGL ES 2.0 and
  * later.
  *
  * Static vertex buffers should be used instead where possible, as they will give
  * better performance especially for large amounts of geometry.
  */
class GeometryBuffer : public Object
{
public:
    GeometryBuffer(RenderContext* rc);
    ~GeometryBuffer();

    void begin(PrimitiveBatch::PrimitiveType primType);
    void beginPoints();
    void beginLines();
    void beginLineStrip();
    void beginTriangles();
    void beginTriangleStrip();
    void end();

    void vertex(const Eigen::Vector3f& v);
    void vertex(const Eigen::Vector3d& v);
    void vertex(float x, float y, float z);

private:
    void flush();

private:
    counted_ptr<VertexBuffer> m_vb;
    RenderContext* m_rc;
    char* m_vertexData;
    PrimitiveBatch::PrimitiveType m_primitiveType;
    unsigned int m_vertexCount;
    unsigned int m_vertexCapacity;
    bool m_inBeginEnd;
};

};

#endif // _VESTA_GEOMETRY_BUFFER_H_
