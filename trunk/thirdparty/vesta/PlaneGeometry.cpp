/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "PlaneGeometry.h"
#include "RenderContext.h"
#include "IntegerTypes.h"
#include <Eigen/Geometry>
#include <algorithm>
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;


PlaneGeometry::PlaneGeometry() :
    m_scale(1.0),
    m_color(0.5f, 0.5f, 0.5f),
    m_opacity(0.2f),
    m_gridLineSpacing(0.0),
    m_grid(0),
    m_solidPlane(0),
    m_vertices(0)
{
    buildGeometry();
}


PlaneGeometry::~PlaneGeometry()
{
    delete m_grid;
    delete m_solidPlane;
    delete m_vertices;
}


void
PlaneGeometry::render(RenderContext& rc,
                      double /* clock */) const
{
    Material material;
    material.setEmission(m_color);

    rc.bindVertexArray(m_vertices);

    rc.pushModelView();
    rc.scaleModelView(Vector3f::Constant((float) m_scale));
    // rc.rotateModelView(m_orientation);

    if (rc.pass() == RenderContext::OpaquePass)
    {
        rc.bindMaterial(&material);
        rc.drawPrimitives(*m_grid);
    }
    else
    {
        material.setOpacity(m_opacity);
        rc.bindMaterial(&material);
        rc.drawPrimitives(*m_solidPlane);
    }

    rc.unbindVertexArray();

    rc.popModelView();
}


float
PlaneGeometry::boundingSphereRadius() const
{
    return float(std::sqrt(2.0) * m_scale);
}


bool
PlaneGeometry::isOpaque() const
{
    return m_opacity >= 1.0f;
}


/** Set the spacing between grid lines in units of kilometers. The spacing
  * for vertical and horizontal grid lines is identical. If the spacing is
  * set to zero, no grid lines will be shown.
  */
void
PlaneGeometry::setGridLineSpacing(double gridLineSpacing)
{
    m_gridLineSpacing = gridLineSpacing;
    delete m_grid;
    delete m_solidPlane;
    delete m_vertices;
    m_grid = 0;
    m_solidPlane = 0;
    m_vertices = 0;
    buildGeometry();
}


void
PlaneGeometry::buildGeometry()
{
    unsigned int gridSubdivision = 0;
    double unscaledSpacing = 0.0;
    double bottomLeft = 0.0;

    if (m_gridLineSpacing > 0.0)
    {
        const unsigned int subdivisionPerQuadrant = (unsigned int) floor(m_scale / m_gridLineSpacing) + 1;
        unscaledSpacing = m_gridLineSpacing / m_scale;
        gridSubdivision = 1 + 2 * subdivisionPerQuadrant;
        bottomLeft = subdivisionPerQuadrant * -unscaledSpacing;
    }
    else
    {
        unscaledSpacing = 2.0;
        gridSubdivision = 2;
        bottomLeft = -1.0;
    }

    const unsigned int vertexCount = (gridSubdivision + 1) * 4;

    // Vertex data must be allocated as char, not Vector3; Eigen uses a custom allocator for
    // objects.
    unsigned char* byteData = new unsigned char[vertexCount * sizeof(Vector3f)];
    Vector3f* data = reinterpret_cast<Vector3f*>(byteData);
    int vertexIndex = 0;

    // Top and bottom
    for (unsigned int i = 0; i <= gridSubdivision; ++i)
    {
        float x = float(i * unscaledSpacing + bottomLeft);
        x = max(-1.0f, min(1.0f, x));
        data[vertexIndex++] = Vector3f(x, -1.0f, 0.0f);
        data[vertexIndex++] = Vector3f(x,  1.0f, 0.0f);
    }

    // Left and right
    for (unsigned int i = 0; i <= gridSubdivision; ++i)
    {
        float y = float(i * unscaledSpacing + bottomLeft);
        y = max(-1.0f, min(1.0f, y));
        data[vertexIndex++] = Vector3f(-1.0f, y, 0.0f);
        data[vertexIndex++] = Vector3f( 1.0f, y, 0.0f);
    }

    m_vertices = new VertexArray(data, vertexCount, VertexSpec::Position, sizeof(data[0]));

    m_grid = new PrimitiveBatch(PrimitiveBatch::Lines, (gridSubdivision + 1) * 2, 0);

    // 4 triangles because we want to draw both the front and back of the
    // plane.
    v_uint16* indices = new v_uint16[12];
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = gridSubdivision * 2 + 1;

    indices[3] = 0;
    indices[4] = gridSubdivision * 2 + 1;
    indices[5] = gridSubdivision * 2;

    indices[6] = gridSubdivision * 2 + 1;
    indices[7] = 1;
    indices[8] = 0;

    indices[9] = gridSubdivision * 2;
    indices[10] = gridSubdivision * 2 + 1;
    indices[11] = 0;

    m_solidPlane = new PrimitiveBatch(PrimitiveBatch::Triangles, indices, 4);
    delete[] indices;
}
