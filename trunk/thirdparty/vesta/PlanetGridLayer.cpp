/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "PlanetGridLayer.h"
#include "RenderContext.h"
#include "QuadtreeTile.h"
#include "Units.h"
#include "WorldGeometry.h"
#include "TextureFont.h"
#include <Eigen/LU>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


PlanetGridLayer::PlanetGridLayer() :
    m_gridColor(1.0f, 1.0f, 1.0f),
    m_gridOpacity(0.5f)
{
}


PlanetGridLayer::~PlanetGridLayer()
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


static float
chooseGridSpacing(float idealSpacing)
{
    static const float allowedSpacings[] =
    {
        30.0f,
        15.0f,
        10.0f,
         5.0f,
         3.0f,
         2.0f,
         1.0f,
         1.0f / 2.0f,  // 30 mins
         1.0f / 3.0f,  // 20 mins
         1.0f / 5.0f,  // 12 mins
         1.0f / 10.0f, //  6 mins
         1.0f / 15.0f, //  4 mins
         1.0f / 20.0f, //  3 mins
         1.0f / 30.0f, //  2 mins
         1.0f / 60.0f, //  1 min
    };
    static const unsigned int nSpacings = sizeof(allowedSpacings) / sizeof(allowedSpacings[0]);

    float spacing = allowedSpacings[nSpacings - 1];
    for (unsigned int i = 0; i < nSpacings; ++i)
    {
        if (idealSpacing > allowedSpacings[i])
        {
            spacing = allowedSpacings[i];
            break;
        }
    }

    return spacing;
}


void
PlanetGridLayer::renderTile(RenderContext& rc, const WorldGeometry* world, const QuadtreeTile* tile) const
{
    float radius = world->ellipsoidAxes().minCoeff() * 0.5f;
    Vector3f cameraPosition = (rc.modelview().inverse() * Vector4f::UnitW()).start<3>();
    float distToCenter = rc.modelview().translation().norm();
    float altitude = max(1.0f, distToCenter - radius);
    float apparentSize = radius / altitude;

    // Compute the approximate projected size of the planet in pixels
    float pixelSize = apparentSize / rc.pixelSize();
    float idealLatSpacing = 360.0f / (pixelSize / 30.0f);

    // Fade out grid when grid spacing gets large
    float opacity = m_gridOpacity;
    if (idealLatSpacing > 45.0f)
    {
        opacity *= max(0.0f, (90.0f - idealLatSpacing) / 45.0f);
    }

    if (opacity == 0.0f)
    {
        return;
    }

    rc.setVertexInfo(VertexSpec::Position);

    Material simpleMaterial;
    simpleMaterial.setDiffuse(m_gridColor);
    simpleMaterial.setOpacity(opacity);
    rc.bindMaterial(&simpleMaterial);

    // Reduce meridian spacing when the observer is close to the pole
    float z = abs(cameraPosition.normalized().z());
    float idealLonSpacing = idealLatSpacing / max(0.01f, sqrt(1.0f - z * z));

    // TODO: Reduce spacing further when observer is close to the surface
    // and looking out along the horizon.

    float latSpacing = float(toRadians(chooseGridSpacing(idealLatSpacing)));
    float lonSpacing = float(toRadians(chooseGridSpacing(idealLonSpacing)));

    float tileArc = float(PI) * tile->extent();
    Vector2f southwest = tile->southwest();

    float lonWest = float(PI) * southwest.x();
    float lonEast = lonWest + tileArc;
    float latSouth = float(PI) * southwest.y();
    float latNorth = latSouth + tileArc;

    int firstMeridian = int(ceil(lonWest / lonSpacing));
    int lastMeridian = int(floor(lonEast / lonSpacing));
    int firstParallel = int(ceil(latSouth / latSpacing));
    int lastParallel = int(floor(latNorth / latSpacing));

    for (int i = firstMeridian; i <= lastMeridian; ++i)
    {
        drawMeridian(i * lonSpacing, latSouth, latNorth);
    }

    for (int i = firstParallel; i <= lastParallel; ++i)
    {
        drawParallel(i * latSpacing, lonWest, lonEast);
    }

    // TODO: Draw coordinate labels
    float subLon = atan2(cameraPosition.y(), cameraPosition.z());
    float subLat = acos(cameraPosition.normalized().z());

    float meridian = std::floor(subLon) / idealLonSpacing;
}


/** Set the font used for coordinate labels.
  */
void
PlanetGridLayer::setLabelFont(TextureFont* font)
{
    m_labelFont = font;
}
