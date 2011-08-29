/*
 * $Revision: 621 $ $Date: 2011-08-29 13:17:00 -0700 (Mon, 29 Aug 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_TEXTURE_MAP_H_
#define _VESTA_TEXTURE_MAP_H_

#include "Object.h"
#include "IntegerTypes.h"
#include <string>


namespace vesta
{

class TextureMapLoader;

class TextureProperties
{
public:
    enum AddressMode
    {
        Wrap      = 0,
        Clamp     = 1,
    };

    /** TextureUsage settings:
      *
      * ColorTexture - an ordinary RGB texture
      * AlphaTexture - alpha-only texture; only affects the alpha value of a fragment, not
      *                the color.
      * NormalMap    - normal map
      * CompressedNormalMap - DXT5 compressed normal map
      */
    enum TextureUsage
    {
        ColorTexture = 0,
        AlphaTexture = 1,
        NormalMap    = 2,
        CompressedNormalMap = 3,
        DepthTexture = 4,
    };

    TextureProperties();
    TextureProperties(AddressMode stAddress);

    AddressMode addressS;
    AddressMode addressT;
    TextureUsage usage;

    /** useMipmaps determines whether mipmapping will be used to improve texture filtering
      * quality and performance. Enabled by default, and appropriate for most textures.
      */
    bool useMipmaps;

    /** The maximum level of anistropic filtering to apply. Modern GPUs generally support
      * up to 16. The default value is 1. Using higher values will result in better filtering
      * quality when textures are viewed near edge-on; the trade-off is that enabling
      * anisotropic filtering can slow down rendering, especially when graphics memory
      * bandwidth is the bottleneck.
      */
    unsigned int maxAnisotropy;

    /** The maximum level of mipmap to generate. The default is 1000, meaning that a full
      * mipmap chain will be used. This property is ignored when useMipmaps is false.
      */
    unsigned int maxMipmapLevel;
};


/** TextureMap is a wrapper class for a texture resource. A TextureMap may be constructed in
  * one of two ways:
  *  - Directly, by wrapping an existing GL texture handle
  *  - Indirectly, through a TextureMapLoader object.
  *
  * A TextureMap created through a TextureMapLoader are not ready to be used for until after makeResident()
  * has been called. Even after calling makeResident, the texture may not be immediately available if the
  * texture loader operates asynchronously. The texture can be queried for residency by testing whether
  * isResident() returns true.
  */
class TextureMap : public Object
{
friend class TextureMapLoader;

public:
    TextureMap(const std::string& name, TextureMapLoader* loader);
    TextureMap(const std::string& name, TextureMapLoader* loader, const TextureProperties& properties);
    TextureMap(unsigned int glTexId, const TextureProperties& properties);
    TextureMap(unsigned int glTexId);
    ~TextureMap();

    enum Status
    {
        Uninitialized  =  0,
        Loading        =  1,
        Ready          =  2,
        LoadingFailed  = -1,
    };

    enum ImageFormat
    {
        R8G8B8A8      =  0,
        B8G8R8A8      =  1,
        R8G8B8        =  2,
        B8G8R8        =  3,
        DXT1          =  4,
        DXT3          =  5,
        DXT5          =  6,
        RGB16F        =  7,
        RGBA16F       =  8,
        RGB32F        =  9,
        RGBA32F       = 10,
        R16F          = 11,
        R32F          = 12,
        RG16F         = 13,
        RG32F         = 14,
        Depth24       = 15,
        R8G8B8_sRGB   = 16,
        R8G8B8A8_sRGB = 17,
        DXT1_sRGB     = 18,
        DXT3_sRGB     = 19,
        DXT5_sRGB     = 20,
        Depth16       = 21,
        Depth32       = 22,
        Depth32F      = 23,
        FormatCount   = 24,
        InvalidFormat = -1,
    };

    unsigned int id() const
    {
        return m_id;
    };

    const std::string name() const
    {
        return m_name;
    }

    bool isResident() const
    {
        return m_id != 0;
    }

    bool makeResident();

    bool generate(const unsigned char imageData[],
                  unsigned int imageDataSize,
                  unsigned int width,
                  unsigned int height,
                  ImageFormat format);

    // Alternate version of generate for use with Java
    // (since Java's byte type is signed.)
    bool generate(const signed char imageData[],
                  unsigned int imageDataSize,
                  unsigned int width,
                  unsigned int height,
                  ImageFormat format)
    {
        return generate(reinterpret_cast<const unsigned char*>(imageData),
                        imageDataSize, width, height, format);
    }

    bool generateCompressed(const char imageData[],
                            unsigned int imageDataSize,
                            unsigned int width,
                            unsigned int height,
                            ImageFormat format,
                            unsigned int mipLevelCount);

    bool generateCompressedFit(const char imageData[],
                               unsigned int imageDataSize,
                               unsigned int width,
                               unsigned int height,
                               ImageFormat format,
                               unsigned int mipLevelCount);

    bool generate(unsigned int width, unsigned int height, ImageFormat format);

    const TextureProperties& properties() const
    {
        return m_properties;
    }

    /** Get the status of the texture:
      *   Unitialized - the texture has not been initialized
      *   Loading     - the texture is currently being loaded
      *   Ready       - the texture was loaded successfully and can be used for rendering
      *   LoadingFailed - an error occurred while loading the texture
      */
    Status status() const
    {
        return m_status;
    }

    /** Set the texture loading status.
      * @see TextureMap::status()
      */
    void setStatus(Status status)
    {
        m_status = status;
    }

    /** Get the amount of graphics memory used by the texture in bytes. This
      * method returns 0 when the status is some value other than Ready. The
      * reported memory usage should be considered an estimate, as it's possible
      * that the OpenGL driver will expand or compress the texture by storing it
      * in a format other than the one requested.
      */
    unsigned int memoryUsage() const
    {
        if (m_status == Ready)
        {
            return m_memoryUsage;
        }
        else
        {
            return 0;
        }
    }

    /** Get a value indicating the last time that the texture was used. Larger
      * values indicate more recently used textures, though the exact interpretation
      * is up to the texture loader. The texture loader uses the value of lastUsed()
      * to determine which textures to evict.
      */
    v_int64 lastUsed() const
    {
        return m_lastUsed;
    }

    /** Set the last used value for this texture.
     *  \see lastUsed()
     */
    void setLastUsed(v_int64 lastUsed)
    {
        m_lastUsed = lastUsed;
    }

    void evict();

    void applyProperties(const TextureProperties& properties);

    static unsigned int MipmapLevelSize(ImageFormat format, unsigned int baseWidth, unsigned int baseHeight, unsigned int level);
    static unsigned int MipmapLevelSize(ImageFormat format, unsigned int width, unsigned int height);
    static unsigned int MipmapChainSize(ImageFormat format, unsigned int baseWidth, unsigned int baseHeight, unsigned int levelCount);
    static bool IsDepthFormat(ImageFormat format);
    static std::string FormatName(ImageFormat format);
    static bool IsFormatSupported(ImageFormat format);

    static TextureMap* CreateDepthTexture(unsigned int width, unsigned int height, ImageFormat format);
    static TextureMap* CreateCubeMap(unsigned int size, ImageFormat format);

private:
    Status m_status;
    unsigned int m_id;
    unsigned int m_memoryUsage;
    TextureMapLoader* m_loader;
    const std::string m_name;
    TextureProperties m_properties;
    v_int64 m_lastUsed;
};

}

#endif // _VESTA_TEXTURE_MAP_H_
