/*
 * $Revision: 585 $ $Date: 2011-03-23 20:18:25 -0700 (Wed, 23 Mar 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_SHADER_INFO_H_
#define _VESTA_SHADER_INFO_H_

#include "IntegerTypes.h"


namespace vesta
{

/** ShaderInfo is used internally in VESTA as a proxy for actual shader
  * programs. Shader programs are generated as needed and stored in
  * the shader cache, which is a table indexed ShaderInfo objects.
  */
class ShaderInfo
{
public:
    enum ReflectanceModel
    {
        Emissive    = 0,
        Lambert     = 1,
        BlinnPhong  = 2,
        Particulate = 3,
    };

    enum
    {
        NoTextures        = 0x00,
        DiffuseTexture    = 0x01,
        SpecularTexture   = 0x02,
        NormalTexture     = 0x04,
        EmissiveTexture   = 0x08,
        ReflectionTexture = 0x10,
    };

    enum
    {
        MaxLightCount     = 3
    };

    ShaderInfo() :
        m_data(0)
    {
        setReflectanceModel(Emissive);
        setTextures(NoTextures);
    }

    // Ordering operator used for the shader cache
    bool operator<(const ShaderInfo& other) const
    {
        return m_data < other.m_data;
    }

    bool operator==(const ShaderInfo& other) const
    {
        return m_data == other.m_data;
    }

    bool operator!=(const ShaderInfo& other) const
    {
        return m_data != other.m_data;
    }

    ReflectanceModel reflectanceModel() const
    {
        return ReflectanceModel(m_data & 0xf);
    }

    void setReflectanceModel(ReflectanceModel reflectance)
    {
        m_data |= (unsigned int) reflectance;
    }

    unsigned int textures() const
    {
        return (m_data & TextureUsageMask) >> 4;
    }

    bool hasTexture(unsigned int texture) const
    {
        return (m_data & (texture << TextureUsageMaskShift)) != 0;
    }

    bool hasTextureCoord() const
    {
        // We only need texture coordinates when there's a texture. Texture coordinates for the
        // reflection map are generated in the fragment shader, so we don't need texCoord if there
        // are no other textures.
        return textures() != 0 && textures() != ReflectionTexture;
    }

    void setTextures(unsigned int textures)
    {
        m_data |= ((textures << TextureUsageMaskShift) & TextureUsageMask);
    }

    void clearTextures(unsigned int textures)
    {
        m_data &= ~((textures << TextureUsageMaskShift) & TextureUsageMask);
    }

    unsigned int directionalLightCount() const
    {
        return (m_data & DirectionalLightCountMask) >> DirectionalLightCountMaskShift;
    }

    void setDirectionalLightCount(unsigned int count)
    {
        m_data |= (count << DirectionalLightCountMaskShift) & DirectionalLightCountMask;
    }

    unsigned int pointLightCount() const
    {
        return (m_data & PointLightCountMask) >> PointLightCountMaskShift;
    }

    void setPointLightCount(unsigned int count)
    {
        m_data |= (count << PointLightCountMaskShift) & PointLightCountMask;
    }

    /** Get the combined count of point and directional light sources.
      */
    unsigned int totalLightCount() const
    {
        return pointLightCount() + directionalLightCount();
    }

    unsigned int shadowCount() const
    {
        return (m_data & ShadowCountMask) >> ShadowCountMaskShift;
    }

    bool hasShadows() const
    {
        return shadowCount() > 0;
    }

    void setShadowCount(unsigned int count)
    {
        m_data |= ((count << ShadowCountMaskShift) & ShadowCountMask);
    }

    /** Get the number of omnidirectional shadows.
      */
    unsigned int omniShadowCount() const
    {
        return (m_data & OmniShadowCountMask) >> OmniShadowCountMaskShift;
    }

    /** Returns true if there are any omnidirectional shadows.
      */
    bool hasOmniShadows() const
    {
        return omniShadowCount() > 0;
    }

    void setOmniShadowCount(unsigned int count)
    {
        m_data |= ((count << OmniShadowCountMaskShift) & OmniShadowCountMask);
    }

    unsigned int eclipseShadowCount() const
    {
        return (m_data & EclipseShadowCountMask) >> EclipseShadowCountMaskShift;
    }

    bool hasEclipseShadows() const
    {
        return eclipseShadowCount() > 0;
    }

    void setEclipseShadowCount(unsigned int count)
    {
        m_data |= ((count << EclipseShadowCountMaskShift) & EclipseShadowCountMask);
    }

    bool hasVertexColors() const
    {
        return (m_data & VertexColorMask) != 0;
    }

    void setVertexColors(bool enable)
    {
        m_data = (m_data & ~VertexColorMask) | (enable ? VertexColorMask : 0x0);
    }

    bool hasAlphaTexture() const
    {
        return (m_data & AlphaTextureMask) != 0;
    }

    void setAlphaTexture(bool enable)
    {
        m_data = (m_data & ~AlphaTextureMask) | (enable ? AlphaTextureMask : 0x0);
    }

    bool hasScattering() const
    {
        return (m_data & ScatteringMask) != 0;
    }

    void setScattering(bool enable)
    {
        m_data = (m_data & ~ScatteringMask) | (enable ? ScatteringMask : 0x0);
    }

    bool isSpherical() const
    {
        return (m_data & SphericalGeometryMask) != 0;
    }

    void setSphericalGeometry(bool enable)
    {
        m_data = (m_data & ~SphericalGeometryMask) | (enable ? SphericalGeometryMask : 0x0);
    }

    bool hasSpecularMaskInDiffuseAlpha() const
    {
        return (m_data & SpecularInAlphaMask) != 0;
    }

    void setSpecularMaskInDiffuseAlpha(bool enable)
    {
        m_data = (m_data & ~SpecularInAlphaMask) | (enable ? SpecularInAlphaMask : 0x0);
    }

    bool hasFresnelFalloff() const
    {
        return (m_data & FresnelFalloffMask) != 0;
    }

    void setFresnelFalloff(bool enable)
    {
        m_data = (m_data & ~FresnelFalloffMask) | (enable ? FresnelFalloffMask : 0x0);
    }

    bool hasCompressedNormalMap() const
    {
        return (m_data & CompressedNormalMapMask) != 0;
    }

    void setCompressedNormalMap(bool enable)
    {
        m_data = (m_data & ~CompressedNormalMapMask) | (enable ? CompressedNormalMapMask : 0x0);
    }

private:
    enum
    {
        ReflectanceModelMask      = 0x000000f,
        TextureUsageMask          = 0x00001f0,
        DirectionalLightCountMask = 0x0000e00,
        PointLightCountMask       = 0x0003000,
        ShadowCountMask           = 0x000c000,
        OmniShadowCountMask       = 0x0030000,
        VertexColorMask           = 0x0040000,
        AlphaTextureMask          = 0x0080000,
        ScatteringMask            = 0x0100000,
        SphericalGeometryMask     = 0x0200000,
        SpecularInAlphaMask       = 0x0400000,
        FresnelFalloffMask        = 0x0800000,
        CompressedNormalMapMask   = 0x1000000,
        EclipseShadowCountMask    = 0xe000000,
    };

    enum
    {
        TextureUsageMaskShift          =  4,
        DirectionalLightCountMaskShift =  9,
        PointLightCountMaskShift       = 12,
        ShadowCountMaskShift           = 14,
        OmniShadowCountMaskShift       = 16,
        EclipseShadowCountMaskShift    = 25,
    };

private:
    v_uint32 m_data;
};

}

#endif // _VESTA_SHADER_INFO_H_
