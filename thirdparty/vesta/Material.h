/*
 * $Revision: 445 $ $Date: 2010-08-20 12:32:00 -0700 (Fri, 20 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_MATERIAL_H_
#define _VESTA_MATERIAL_H_

#include "Entity.h"
#include "Spectrum.h"
#include "TextureMap.h"


namespace vesta
{

class Material : public Object
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    enum BlendMode
    {
        Opaque                  = 0,
        AlphaBlend              = 1,
        AdditiveBlend           = 2,
        PremultipliedAlphaBlend = 3,
    };

    enum SpecularModifierSource
    {
        SpecularTextureRGB      = 0,
        DiffuseTextureAlpha     = 1,
    };
    
    enum BRDF
    {
        Lambert,
        BlinnPhong,
        BlinnPhongReflective,
        ParticulateVolume,
        RingParticles
    };

    Material() :
        Object(),
        m_brdf(Lambert),
        m_opacity(1.0f),
        m_phongExponent(1.0f),
        m_fresnelReflectance(1.0f),
        m_blendMode(Opaque),
        m_specularModifier(SpecularTextureRGB)
    {
    }

    Material(const Material& m) :
        Object(),
        m_brdf(m.m_brdf),
        m_opacity(m.m_opacity),
        m_diffuse(m.m_diffuse),
        m_specular(m.m_specular),
        m_phongExponent(m.m_phongExponent),
        m_fresnelReflectance(m.m_fresnelReflectance),
        m_emission(m.m_emission),
        m_blendMode(m.m_blendMode),
        m_baseTexture(m.m_baseTexture),
        m_normalTexture(m.m_normalTexture),
        m_specularTexture(m.m_specularTexture),
        m_specularModifier(m.m_specularModifier)
    {
    }

    Material& operator=(const Material& m)
    {
        m_brdf = m.m_brdf;
        m_opacity = m.m_opacity;
        m_diffuse = m.m_diffuse;
        m_specular = m.m_specular;
        m_phongExponent = m.m_phongExponent;
        m_fresnelReflectance = m.m_fresnelReflectance;
        m_emission = m.m_emission;
        m_blendMode = m.m_blendMode;
        m_baseTexture = m.m_baseTexture;
        m_normalTexture = m.m_normalTexture;
        m_specularTexture = m.m_specularTexture;
        m_specularModifier = m.m_specularModifier;

        return *this;
    }

    BRDF brdf() const
    {
        return m_brdf;
    }

    float opacity() const
    {
        return m_opacity;
    }

    Spectrum diffuse() const
    {
        return m_diffuse;
    }

    Spectrum specular() const
    {
        return m_specular;
    }

    float phongExponent() const
    {
        return m_phongExponent;
    }

    /** Get the reflectance at normal incidence. For conductive materials like metals,
      * this should be near 1.0. For dielectrics, a lower value is appropriate:
      * ((n1 - n2) / (n1 + n2))^2, where n1 is the index of refraction of the medium
      * containing the material, and n2 is the index of refraction of the material.
      */
    float fresnelReflectance() const
    {
        return m_fresnelReflectance;
    }

    /** Return true if this material is environment mapped. */
    bool isReflective() const
    {
        return m_brdf == BlinnPhongReflective;
    }

    /** Get the specular modifier source.
      * @see setSpecularModifier
      */
    SpecularModifierSource specularModifier() const
    {
        return m_specularModifier;
    }

    Spectrum emission() const
    {
        return m_emission;
    }

    BlendMode blendMode() const
    {
        return m_blendMode;
    }

    TextureMap* baseTexture() const
    {
        return m_baseTexture.ptr();
    }

    TextureMap* normalTexture() const
    {
        return m_normalTexture.ptr();
    }

    TextureMap* specularTexture() const
    {
        return m_specularTexture.ptr();
    }

    void setBrdf(BRDF brdf)
    {
        m_brdf = brdf;
    }

    void setOpacity(float opacity)
    {
        m_opacity = opacity;
    }

    void setDiffuse(const Spectrum& diffuse)
    {
        m_diffuse = diffuse;
    }

    void setSpecular(const Spectrum& specular)
    {
        m_specular = specular;
    }

    void setPhongExponent(float phongExponent)
    {
        m_phongExponent = phongExponent;
    }

    /** Set the reflectance at normal incidence. For conductive materials like metals,
      * this should be near 1.0. For dielectrics, a lower value is appropriate:
      * ((n1 - n2) / (n1 + n2))^2, where n1 is the index of refraction of the medium
      * containing the material, and n2 is the index of refraction of the material.
      */
    void setFresnelReflectance(float f)
    {
        m_fresnelReflectance = f;
    }

    /** Set the specular color modifier. This is the value which is multiplied by the
      * specular color in the material to get the final specular color for a pixel. By
      * default, it is SpecularTextureRGB. It may be changed to DiffuseTextureAlpha to
      * indicate that a specular mask is present in the alpha channel of the diffuse
      * texture. The specular color is unchanged if the material lacks a specular texture
      * or diffuse texture.
      */
    void setSpecularModifier(SpecularModifierSource source)
    {
        m_specularModifier = source;
    }

    void setEmission(const Spectrum& emission)
    {
        m_emission = emission;
    }

    void setBaseTexture(TextureMap* texture)
    {
        m_baseTexture = texture;
    }

    void setNormalTexture(TextureMap* texture)
    {
        m_normalTexture = texture;
    }

    void setSpecularTexture(TextureMap* texture)
    {
        m_specularTexture = texture;
    }

    void setBlendMode(BlendMode blend)
    {
        m_blendMode = blend;
    }

private:
    BRDF m_brdf;
    float m_opacity;
    Spectrum m_diffuse;
    Spectrum m_specular;
    float m_phongExponent;
    float m_fresnelReflectance;
    Spectrum m_emission;
    BlendMode m_blendMode;
    counted_ptr<TextureMap> m_baseTexture;
    counted_ptr<TextureMap> m_normalTexture;
    counted_ptr<TextureMap> m_specularTexture;
    SpecularModifierSource m_specularModifier;
};

}

#endif // _VESTA_MATERIAL_H_

