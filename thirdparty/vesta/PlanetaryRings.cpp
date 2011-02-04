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
#include "Debug.h"
#include <cmath>

using namespace vesta;
using namespace std;


PlanetaryRings::PlanetaryRings(float innerRadius, float outerRadius) :
    m_innerRadius(innerRadius),
    m_outerRadius(outerRadius)
{

}


PlanetaryRings::~PlanetaryRings()
{
}


void
PlanetaryRings::render(RenderContext& rc,
                       double /* clock */) const
{
    Material material;
    material.setBrdf(Material::ParticulateVolume);
    material.setDiffuse(Spectrum::Flat(1.0f));
    material.setOpacity(0.99f);
    material.setBlendMode(Material::AlphaBlend);
    material.setBaseTexture(m_texture.ptr());

    rc.bindMaterial(&material);
    rc.setVertexInfo(VertexSpec::PositionTex);

    unsigned int ringSections = 128;

    glDisable(GL_CULL_FACE);
    glBegin(GL_TRIANGLE_STRIP);
    for (unsigned int i = 0; i <= ringSections; ++i)
    {
        float t = float(i) / float(ringSections);
        float theta = t * float(PI) * 2.0f;
        float s = sin(theta);
        float c = cos(theta);

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(c * m_innerRadius, s * m_innerRadius, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(c * m_outerRadius, s * m_outerRadius, 0.0f);
    }
    glEnd();
    glEnable(GL_CULL_FACE);
}


float
PlanetaryRings::boundingSphereRadius() const
{
    return m_outerRadius;
}

