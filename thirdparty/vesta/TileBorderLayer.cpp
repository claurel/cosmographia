/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "TileBorderLayer.h"
#include "RenderContext.h"
#include "QuadtreeTile.h"
#include "Units.h"
#include "WorldGeometry.h"
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


TileBorderLayer::TileBorderLayer() :
    m_color(1.0f, 1.0f, 0.0f),
    m_opacity(1.0f)
{
}


TileBorderLayer::~TileBorderLayer()
{
}


static void
drawParallel(float lat, float lon0, float lon1)
{
#ifndef VESTA_NO_IMMEDIATE_MODE_3D
    float cosLat = cos(lat);
    float sinLat = sin(lat);

    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i <= 32; ++i)
    {
        float t = float(i) / 32.0f;
        float lon = (1.0f - t) * lon0 + t * lon1;
        Vector3f v(cos(lon) * cosLat, sin(lon) * cosLat, sinLat);
        glVertex3fv(v.data());
    }
    glEnd();
#endif
}


static void
drawMeridian(float lon, float lat0, float lat1)
{
#ifndef VESTA_NO_IMMEDIATE_MODE_3D
    float cosLon = cos(lon);
    float sinLon = sin(lon);

    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i <= 32; ++i)
    {
        float t = float(i) / 32.0f;
        float lat = (1.0f - t) * lat0 + t * lat1;
        float cosLat = cos(lat);
        Vector3f v(cosLon * cosLat, sinLon * cosLat, sin(lat));
        glVertex3fv(v.data());
    }
    glEnd();
#endif
}


void
TileBorderLayer::renderTile(RenderContext& rc, const WorldGeometry* /* world */, const QuadtreeTile* tile) const
{
    rc.setVertexInfo(VertexSpec::Position);

    Material simpleMaterial;
    simpleMaterial.setDiffuse(m_color);
    simpleMaterial.setOpacity(m_opacity);
    rc.bindMaterial(&simpleMaterial);

    float tileArc = float(PI) * tile->extent();
    Vector2f southwest = tile->southwest();

    float lonWest = float(PI) * southwest.x();
    float lonEast = lonWest + tileArc;
    float latSouth = float(PI) * southwest.y();
    float latNorth = latSouth + tileArc;

    drawParallel(latSouth, lonWest, lonEast);
    drawParallel(latNorth, lonWest, lonEast);
    drawMeridian(lonEast, latSouth, latNorth);
    drawMeridian(lonEast, latSouth, latSouth);
}
