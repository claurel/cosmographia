/*
 * $Revision: 253 $ $Date: 2010-04-30 06:31:38 -0500 (Fri, 30 Apr 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "PlanetaryRings.h"
#include "TextureMap.h"
#include "Material.h"
#include "RenderContext.h"
#include "Units.h"
#include "VertexBuffer.h"
#include <cmath>

using namespace vesta;
using namespace std;


PlanetaryRings::PlanetaryRings(float innerRadius, float outerRadius) :
    m_innerRadius(innerRadius),
    m_outerRadius(outerRadius)
{
    setShadowCaster(true);
}


PlanetaryRings::~PlanetaryRings()
{
}


void
PlanetaryRings::render(RenderContext& rc,
                       double /* clock */) const
{
    if (rc.pass() == RenderContext::TranslucentPass)
    {
        Material material;
        material.setBrdf(Material::RingParticles);
        material.setDiffuse(Spectrum::Flat(1.0f));
        material.setOpacity(0.99f);
        material.setBlendMode(Material::AlphaBlend);
        material.setBaseTexture(m_texture.ptr());

        rc.bindMaterial(&material);
        rc.setVertexInfo(VertexSpec::PositionTex);

        unsigned int ringSections = 128;

        glDisable(GL_CULL_FACE);

        VertexBuffer* vb = rc.vertexStreamBuffer();
        float* vertexData = reinterpret_cast<float*>(vb->mapWriteOnly(true));

        unsigned int pos = 0;
        for (unsigned int i = 0; i <= ringSections; ++i)
        {
            float t = float(i) / float(ringSections);
            float theta = t * float(PI) * 2.0f;
            float s = sin(theta);
            float c = cos(theta);

            vertexData[pos + 0] = c * m_innerRadius;
            vertexData[pos + 1] = s * m_innerRadius;
            vertexData[pos + 2] = 0.0f;
            vertexData[pos + 3] = 0.0f;
            vertexData[pos + 4] = 0.0f;
            pos += 5;

            vertexData[pos + 0] = c * m_outerRadius;
            vertexData[pos + 1] = s * m_outerRadius;
            vertexData[pos + 2] = 0.0f;
            vertexData[pos + 3] = 1.0f;
            vertexData[pos + 4] = 0.0f;
            pos += 5;
        }

        vb->unmap();
        rc.bindVertexBuffer(VertexSpec::PositionTex, vb, 5 * 4);
        rc.drawPrimitives(PrimitiveBatch(PrimitiveBatch::TriangleStrip, ringSections * 2, 0));
        rc.unbindVertexBuffer();

        glEnable(GL_CULL_FACE);
    }
}


float
PlanetaryRings::boundingSphereRadius() const
{
    return m_outerRadius;
}


/** Set the rings texture map. The texture is applied so that the inner edge of the
  * rings is assigned texture coordinate (0, 0) and the outer edge is assigned
  * (1, 0). The second texture coordinate is always zero, thus it is appropriate
  * to use a texture map with a height of 1.
  */
void
PlanetaryRings::setTexture(TextureMap* texture)
{
    m_texture = texture;
}


/** Planetary rings are treated as ellipsoidal even though the geometry
  * is a degenerate ellipsoid.
  */
AlignedEllipsoid
PlanetaryRings::ellipsoid() const
{
    return AlignedEllipsoid(Eigen::Vector3d(m_outerRadius, m_outerRadius, 0.0));
}

