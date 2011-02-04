/*
 * $Revision: 499 $ $Date: 2010-09-10 18:18:05 -0700 (Fri, 10 Sep 2010) $
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

    unsigned int lightCount() const
    {
        return (m_data & LightCountMask) >> LightCountMaskShift;
    }

    void setLightCount(unsigned int count)
    {
        m_data |= (count << LightCountMaskShift) & LightCountMask;
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
        ReflectanceModelMask    = 0x00000f,
        TextureUsageMask        = 0x0001f0,
        LightCountMask          = 0x000e00,
        ShadowCountMask         = 0x003000,
        OmniShadowCountMask     = 0x00c000,
        VertexColorMask         = 0x010000,
        AlphaTextureMask        = 0x020000,
        ScatteringMask          = 0x040000,
        SphericalGeometryMask   = 0x080000,
        SpecularInAlphaMask     = 0x100000,
        FresnelFalloffMask      = 0x200000,
        CompressedNormalMapMask = 0x400000,
    };

    enum
    {
        TextureUsageMaskShift     =  4,
        LightCountMaskShift       =  9,
        ShadowCountMaskShift      = 12,
        OmniShadowCountMaskShift  = 14,
    };

private:
    v_uint32 m_data;
};

}

#endif // _VESTA_SHADER_INFO_H_
