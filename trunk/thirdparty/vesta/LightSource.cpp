/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "LightSource.h"
#include "TextureMap.h"

using namespace vesta;


LightSource::LightSource() :
    m_type(PointLight),
    m_luminosity(0.0f),
    m_spectrum(Spectrum::White()),
    m_range(1.0f),
    m_shadowCaster(false)
{
}


LightSource::~LightSource()
{
}


/** Set the texture map used for displaying a glare effect when the light
  * source is directly visible. Glare is disabled when the glare texture is
  * set to NULL.
  */
void
LightSource::setGlareTexture(TextureMap* texture)
{
    m_glareTexture = texture;
}
