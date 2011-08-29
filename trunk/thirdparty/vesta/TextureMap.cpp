/*
 * $Revision: 621 $ $Date: 2011-08-29 13:17:00 -0700 (Mon, 29 Aug 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "TextureMap.h"
#include "TextureMapLoader.h"
#include "Debug.h"
#include <GL/glew.h>
#include <cassert>
#include <algorithm>

using namespace vesta;
using namespace std;


struct VestaFormatInfo
{
    TextureMap::ImageFormat format;
    GLenum glFormat;
    GLenum glInternalFormat;
    GLenum bytesPerPixel;
    const char* name;
};


// Table containing mappings from VESTA formats to OpenGL formats
static struct VestaFormatInfo FormatInfo[] =
{
    { TextureMap::R8G8B8A8,      GL_RGBA,     GL_RGBA8,            4, "R8G8B8A8" },
    { TextureMap::B8G8R8A8,      GL_BGRA_EXT, GL_RGBA8,            4, "B8G8R8A8" },
    { TextureMap::R8G8B8,        GL_RGB,      GL_RGB8,             3, "R8G8B8" },
    { TextureMap::B8G8R8,        GL_BGR_EXT,  GL_RGB8,             3, "B8G8R8" },
    { TextureMap::DXT1,          GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 8, "DXT1" },
    { TextureMap::DXT3,          GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 16, "DXT3" },
    { TextureMap::DXT5,          GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 16, "DXT5" },
    { TextureMap::RGB16F,        GL_RGB,      GL_RGB16F_ARB,       6, "RGB16F" },
    { TextureMap::RGBA16F,       GL_RGBA,     GL_RGBA16F_ARB,      8, "RGBA16F" },
    { TextureMap::RGB32F,        GL_RGB,      GL_RGB32F_ARB,      12, "RGB32F" },
    { TextureMap::RGBA32F,       GL_RGBA,     GL_RGBA32F_ARB,     16, "RGBA32F" },
    { TextureMap::R16F,          GL_RED,      GL_R16F,             2, "R16F" },
    { TextureMap::R32F,          GL_RED,      GL_R32F,             4, "R32F" },
    { TextureMap::RG16F,         GL_RG,       GL_RG16F,            4, "RG16F" },
    { TextureMap::RG32F,         GL_RG,       GL_RG32F,            8, "RG32F" },
    { TextureMap::Depth24,       GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, 3, "Depth24" },
    { TextureMap::R8G8B8_sRGB,   GL_RGB,      GL_SRGB8_EXT,        3, "R8G8B8 sRGB" },
    { TextureMap::R8G8B8A8_sRGB, GL_RGBA,     GL_SRGB8_ALPHA8_EXT, 4, "R8G8B8A8 sRGB" },
    { TextureMap::DXT1_sRGB,     GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, 8, "DXT1 sRGB" },
    { TextureMap::DXT3_sRGB,     GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 16, "DXT3 sRGB" },
    { TextureMap::DXT5_sRGB,     GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 16, "DXT5 sRGB" },
    { TextureMap::Depth16,       GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, 2, "Depth16" },
    { TextureMap::Depth32,       GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32, 4, "Depth32" },
    { TextureMap::Depth32F,      GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, 4, "Depth32F" },
};


static GLenum
ToGlFormat(TextureMap::ImageFormat format)
{
    int formatIndex = int(format);
    if (format == TextureMap::InvalidFormat)
    {
        return 0;
    }
    else
    {
        return FormatInfo[formatIndex].glFormat;
    }
}


static GLenum
ToGlInternalFormat(TextureMap::ImageFormat format)
{
    int formatIndex = int(format);
    if (format == TextureMap::InvalidFormat)
    {
        return 0;
    }
    else
    {
        return FormatInfo[formatIndex].glInternalFormat;
    }
}


/** Get the size in bytes of a texel. For compressed formats,
  * return the size of a block.
  */
static unsigned int
BytesPerPixel(TextureMap::ImageFormat format)
{
    int formatIndex = int(format);
    if (format == TextureMap::InvalidFormat)
    {
        return 0;
    }
    else
    {
        return FormatInfo[formatIndex].bytesPerPixel;
    }
}


static GLenum
ToGlWrap(TextureProperties::AddressMode addressMode)
{
    switch (addressMode)
    {
    case TextureProperties::Wrap:
        return GL_REPEAT;
    case TextureProperties::Clamp:
        return GL_CLAMP_TO_EDGE_EXT;
    default:
        return 0;
    }
}


/** Create a default texture properties object.
  *
  * s-coordinate addressing: wrap
  * t-coordinate addressing: wrap
  */
TextureProperties::TextureProperties() :
    addressS(Wrap),
    addressT(Wrap),
    usage(ColorTexture),
    useMipmaps(true),
    maxAnisotropy(1),
    maxMipmapLevel(1000)
{
}


/** Create a new texture properties with the specified address mode
  * used for both the s and t coordinates.
  */
TextureProperties::TextureProperties(TextureProperties::AddressMode stAddress) :
    addressS(stAddress),
    addressT(stAddress),
    usage(ColorTexture),
    useMipmaps(true),
    maxAnisotropy(1),
    maxMipmapLevel(1000)
{
}


TextureMap::TextureMap(const string& name, TextureMapLoader* loader) :
    m_status(Uninitialized),
    m_id(0),
    m_memoryUsage(0),
    m_loader(loader),
    m_name(name),
    m_lastUsed(0)
{
}


TextureMap::TextureMap(const string& name, TextureMapLoader* loader, const TextureProperties& properties) :
    m_status(Uninitialized),
    m_id(0),
    m_memoryUsage(0),
    m_loader(loader),
    m_name(name),
    m_properties(properties),
    m_lastUsed(0)
{
}


/** Construct a new texture map object that wraps an OpenGL texture handle. This is
  * useful when the texture doesn't need to be created via a texture loader. The
  * TextureMap instance takes ownership of the texture handle and will call GL to
  * delete it in the destructor.
  */
TextureMap::TextureMap(unsigned int glTexId, const TextureProperties& properties) :
    m_status(Ready),
    m_id(glTexId),
    m_memoryUsage(0),
    m_loader(0),
    m_properties(properties),
    m_lastUsed(0)
{
}


/** Construct a new texture map object that wraps an OpenGL texture handle. This is
  * useful when the texture doesn't need to be created via a texture loader. The
  * TextureMap instance takes ownership of the texture handle and will call GL to
  * delete it in the destructor. Texture properties are not modified.
  */
TextureMap::TextureMap(unsigned int glTexId) :
    m_status(Ready),
    m_id(glTexId),
    m_memoryUsage(0),
    m_loader(0),
    m_lastUsed(0)
{
}


TextureMap::~TextureMap()
{
    if (m_id)
    {
        glDeleteTextures(1, &m_id);
    }
}


/** Load the texture map and return true if it's ready to be used for rendering.
  * The texture may not be immediately available if it has an asynchronous
  * loader. The call to makeResident() has no effect if the texture is already
  * loaded.
  */
bool
TextureMap::makeResident()
{
    if (m_loader)
    {
        if (status() == Uninitialized)
        {
            m_loader->makeResident(this);
        }
        m_lastUsed = m_loader->frameCount();
    }

    return isResident();
}


static void setTextureFiltering(GLenum target, const TextureProperties& properties)
{
    GLint minFilter = properties.useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);

    if (GLEW_EXT_texture_filter_anisotropic && properties.maxAnisotropy > 1)
    {
        GLint maxAnisotropy = 1;
        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        GLint anisotropy = min((GLint) properties.maxAnisotropy, maxAnisotropy);

        glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
    }
}


/** Generate a texture map without initializing the texture data.
  */
bool
TextureMap::generate(unsigned int width, unsigned int height, ImageFormat format)
{
    GLenum glImageFormat = ToGlFormat(format);
    if (glImageFormat == 0)
    {
        VESTA_LOG("Bad image format provided to TextureMap::generate()");
        setStatus(LoadingFailed);
        return false;
    }

    GLenum glInternalFormat = ToGlInternalFormat(format);

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGlWrap(m_properties.addressS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGlWrap(m_properties.addressT));

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 (GLint) glInternalFormat,
                 width, height,
                 0,
                 glImageFormat,
                 GL_UNSIGNED_BYTE,
                 NULL);

    setTextureFiltering(GL_TEXTURE_2D, m_properties);

    setStatus(Ready);

    return true;
}


/** Realize this texture on the GPU using the specified image data.
  * Mipmaps will be generated automatically if the useMipmaps property
  * was set when the texture was constructed.
  */
bool
TextureMap::generate(const unsigned char imageData[],
                     unsigned int imageDataSize,
                     unsigned int width,
                     unsigned int height,
                     ImageFormat format)
{
    GLenum glImageFormat = ToGlFormat(format);
    if (glImageFormat == 0)
    {
        VESTA_LOG("Bad image format provided to TextureMap::generate()");
        setStatus(LoadingFailed);
        return false;
    }

    GLenum glInternalFormat = ToGlInternalFormat(format);

    // Verify that imageData is large enough to hold the entire texture
    if (imageDataSize < width * height * BytesPerPixel(format))
    {
        VESTA_LOG("Incomplete image data provided to TextureMap::generate()");
        setStatus(LoadingFailed);
        return false;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ToGlWrap(m_properties.addressS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ToGlWrap(m_properties.addressT));

    m_memoryUsage = width * height * BytesPerPixel(format);

    if (m_properties.useMipmaps)
    {
        if (m_properties.maxMipmapLevel < 1000)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_properties.maxMipmapLevel);
        }

        if (GLEW_EXT_framebuffer_object)
        {
            // Fast path uses glGenerateMipmap() when driver/hardware supports it.
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         (GLint) glInternalFormat,
                         width, height,
                         0,
                         glImageFormat,
                         GL_UNSIGNED_BYTE,
                         imageData);

            glGenerateMipmapEXT(GL_TEXTURE_2D);
        }
        else
        {
            // Legacy path for hardware that doesn't have ARB_framebuffer_object. Slower than
            // glGenerateMipmap() and doesn't support as many texture features.
            gluBuild2DMipmaps(GL_TEXTURE_2D,
                              (GLint) glInternalFormat,
                              width, height,
                              glImageFormat,
                              GL_UNSIGNED_BYTE,
                              imageData);
        }

        // A complete mipmap chain uses about 1/3 more memory
        m_memoryUsage += m_memoryUsage / 3;
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     (GLint) glInternalFormat,
                     width, height,
                     0,
                     glImageFormat,
                     GL_UNSIGNED_BYTE,
                     imageData);        
    }

    setTextureFiltering(GL_TEXTURE_2D, m_properties);

    setStatus(Ready);

    return true;
}


/** Realize this texture on the GPU using the specified compressed image data.
  * Mipmaps will be used if the level count is set to > 1. Note that mipmaps for
  * block compressed textures are not generated automatically; they are only
  * enabled when pregenerated mipmaps are provided.
  */
bool
TextureMap::generateCompressed(const char compressedImageData[],
                               unsigned int imageDataSize,
                               unsigned int width,
                               unsigned int height,
                               ImageFormat format,
                               unsigned int mipLevelCount)
{
    unsigned int mipChainSize = MipmapChainSize(format, width, height, mipLevelCount);
    if (mipChainSize > imageDataSize)
    {
        VESTA_LOG("Incomplete compressed image data provided to TextureMap::generateCompressed()");
        setStatus(LoadingFailed);
        return false;
    }

    if (GLEW_ARB_texture_compression == GL_FALSE)
    {
        VESTA_LOG("Attempted to create compressed texture, but hardware doesn't support the feature.");
        setStatus(LoadingFailed);
        return false;
    }

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    GLenum glInternalFormat = ToGlFormat(format);

    unsigned int mipLevelOffset = 0;

    for (unsigned int level = 0; level < mipLevelCount; ++level)
    {
        unsigned int mipLevelWidth  = max(width >> level, 1u);
        unsigned int mipLevelHeight = max(height >> level, 1u);
        unsigned int mipLevelSize = MipmapLevelSize(format, width, height, level);

        glCompressedTexImage2DARB(GL_TEXTURE_2D,
                                  level,
                                  (GLint) glInternalFormat,
                                  mipLevelWidth, mipLevelHeight,
                                  0,
                                  mipLevelSize,
                                  compressedImageData + mipLevelOffset);

        // Advance to the next mip level
        mipLevelOffset += mipLevelSize;
    }

    if (mipLevelCount <= 1)
    {
        m_properties.useMipmaps = false;
    }
    applyProperties(m_properties);

    setStatus(Ready);
    m_memoryUsage = mipLevelOffset;

    return true;
}


/** Realize this texture on the GPU using the specified compressed image data.
  *
  * This method is identical to TextureMap::generateCompressed except that it
  * will automatically only part of the mipmap chain (if available) when the
  * base mipmap level is too large for the GPU. For example, a 8192x4096 texture
  * will is too large by for a GPU with a maximum texture size of 2048.
  * generateCompressedFit() will discard the top two mipmap levels (8192x4096 and
  * 4096x2048) and load the third mipmap level (2048x1024) as the base.
  *
  * This strategy is only available when mipmap levels are provided; texture loading
  * will fail when the texture is too large for the GPU and no mipmaps are provided.
  *
  * \returns true if the texture data was successfully loaded on the GPU
  *
  * \see TextureMap::generateCompressed
  */
bool
TextureMap::generateCompressedFit(const char compressedImageData[],
                                  unsigned int imageDataSize,
                                  unsigned int width,
                                  unsigned int height,
                                  ImageFormat format,
                                  unsigned int mipLevelCount)
{
    GLint maxTextureSize = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);

    // If the size of the compressed texture exceeds the maximum texture size permitted
    // by the GPU, try using a lower mip level that's small enough for the GPU to handle.
    if (width > (unsigned int) maxTextureSize || height > (unsigned int) maxTextureSize)
    {
        unsigned int maxDimension = max(width, height);
        unsigned int mipLevel = 0;
        while (maxDimension > (unsigned int) maxTextureSize)
        {
            maxDimension >>= 1;
            mipLevel++;
        }

        if (mipLevel < mipLevelCount)
        {
            unsigned int dataOffset = MipmapChainSize(format, width, height, mipLevel);
            return generateCompressed(compressedImageData + dataOffset, imageDataSize - dataOffset,
                                      width >> mipLevel, height >> mipLevel,
                                      format,
                                      mipLevelCount - mipLevel);
        }
        else
        {
            // Not enough mip levels available; fail texture generation
            return false;
        }
    }
    else
    {
        return generateCompressed(compressedImageData, imageDataSize,
                                  width, height,
                                  format,
                                  mipLevelCount);
    }
}


/** Release the graphics memory used by the texture and mark it as uninitialized.
  */
void
TextureMap::evict()
{
    if (m_id)
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
    m_memoryUsage = 0;
    setStatus(Uninitialized);
}


static void ApplyTextureProperties(const TextureProperties& properties,
                                   GLenum target)
{
    glTexParameteri(target, GL_TEXTURE_WRAP_S, ToGlWrap(properties.addressS));
    glTexParameteri(target, GL_TEXTURE_WRAP_T, ToGlWrap(properties.addressT));

    setTextureFiltering(target, properties);

    if (properties.usage == TextureProperties::DepthTexture)
    {
        glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(target, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    }
}


// Apply texture filtering and addressing properties
void
TextureMap::applyProperties(const TextureProperties& properties)
{
    if (m_id != 0)
    {
        glBindTexture(GL_TEXTURE_2D, m_id);
        ApplyTextureProperties(properties, GL_TEXTURE_2D);
    }
}


/** Get the size in bytes of a mipmap level with the specified format and dimensions.
  */
unsigned int
TextureMap::MipmapLevelSize(ImageFormat format, unsigned int width, unsigned int height)
{
    unsigned int blockWidth = 1;
    unsigned int blockHeight = 1;
    switch (format)
    {
    case DXT1:
    case DXT1_sRGB:
        blockWidth = 4;
        blockHeight = 4;
        break;

    case DXT3:
    case DXT5:
    case DXT3_sRGB:
    case DXT5_sRGB:
        blockWidth = 4;
        blockHeight = 4;
        break;

    default:
        break;
    }

    unsigned int widthBlocks = (width + blockWidth - 1) / blockWidth;
    unsigned int heightBlocks = (height + blockHeight - 1) / blockHeight;

    return widthBlocks * heightBlocks * BytesPerPixel(format);
}


/** Get the size in bytes of the mipmap level of a texture with the specified base
  * (mipmap level 0) width and height.
  */
unsigned int
TextureMap::MipmapLevelSize(ImageFormat format, unsigned int baseWidth, unsigned int baseHeight, unsigned int level)
{
    unsigned int mipLevelWidth  = max(baseWidth >> level, 1u);
    unsigned int mipLevelHeight = max(baseHeight >> level, 1u);

    return MipmapLevelSize(format, mipLevelWidth, mipLevelHeight);
}


/** Get the size in bytes of a mipmap chain with the specified length and base texture dimensions.
  */
unsigned int
TextureMap::MipmapChainSize(ImageFormat format, unsigned int baseWidth, unsigned int baseHeight, unsigned int levelCount)
{
    unsigned int size = 0;
    for (unsigned int i = 0; i < levelCount; ++i)
    {
        size += MipmapLevelSize(format, baseWidth, baseHeight, i);
    }

    return size;
}


/** Return true if the specified format is a depth buffer format.
  */
bool
TextureMap::IsDepthFormat(ImageFormat format)
{
    switch (format)
    {
    case Depth16:
    case Depth24:
    case Depth32:
    case Depth32F:
        return true;
    default:
        return false;
    }
}


/** Return a string with a human readable name of the format. This is
  * primarily useful for logging and error handling.
  */
string
TextureMap::FormatName(ImageFormat format)
{
    int formatIndex = (int) format;
    if (formatIndex >= 0 && formatIndex < (int) FormatCount)
    {
        return FormatInfo[formatIndex].name;
    }
    else
    {
        return string("UNKNOWN");
    }
}


/** Returns true if the specified format is supported the hardware
  * and driver. This function will only return reliable results if
  * called from a thread with a valid, initialized OpenGL context.
  */
bool
TextureMap::IsFormatSupported(ImageFormat format)
{
    // Test for the presence of one or more OpenGL extensions based on
    // the format.
    bool dxtSupported = GLEW_EXT_texture_compression_s3tc == GL_TRUE;
    bool srgbSupported = GLEW_EXT_texture_sRGB == GL_TRUE;
    bool floatSupported = GLEW_ARB_texture_float == GL_TRUE;

    switch (format)
    {
    case R8G8B8A8:
    case R8G8B8:
        return true;

    case B8G8R8A8:
    case B8G8R8:
        return GLEW_EXT_bgra == GL_TRUE;

    case DXT1:
    case DXT3:
    case DXT5:
        return dxtSupported;

    case RGB16F:
    case RGBA16F:
    case RGB32F:
    case RGBA32F:
        return floatSupported;

    case R16F:
    case R32F:
    case RG16F:
    case RG32F:
        return floatSupported && GLEW_ARB_texture_rg == GL_TRUE;

    case R8G8B8_sRGB:
    case R8G8B8A8_sRGB:
        return srgbSupported;

    case DXT1_sRGB:
    case DXT3_sRGB:
    case DXT5_sRGB:
        return dxtSupported && srgbSupported;

    case Depth16:
    case Depth24:
    case Depth32:
        return GLEW_VERSION_1_4 == GL_TRUE;

    case Depth32F:
        return GLEW_ARB_depth_buffer_float == GL_TRUE;

    default:
        return false;
    }

}


/** Factory method for creating a depth texture.
  * The contents of the depth texture are not initialized and will contain
  * undefined image data.
  *
  * \param width the width of the texture in pixels
  * \param height the height of the texture in pixels
  * \param format a valid depth texture format (currently just Depth24 is allowed)
  *
  * \return either a valid, fully constructed depth texture or NULL if there was
  * an error.
  */
TextureMap*
TextureMap::CreateDepthTexture(unsigned int width, unsigned int height, ImageFormat format)
{
    if (format != Depth24)
    {
        VESTA_WARNING("Invalid depth texture format requested.");
        return NULL;
    }

    GLuint depthTexId = 0;
    glGenTextures(1, &depthTexId);
    if (depthTexId == 0)
    {
        VESTA_WARNING("Failed to create depth texture handle.");
        return 0;
    }

    glBindTexture(GL_TEXTURE_2D, depthTexId);

    // Allocate the texture
    glTexImage2D(GL_TEXTURE_2D, 0, ToGlInternalFormat(format), width, height, 0, ToGlFormat(format), GL_UNSIGNED_BYTE, 0);

    // Unbind it
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum errorCode = glGetError();
    if (errorCode != GL_NO_ERROR)
    {
        const GLubyte* errorMessage = gluErrorString(errorCode);
        if (errorMessage)
        {
            VESTA_WARNING("OpenGL error occurred when creating depth texture: %s", errorMessage);
            glDeleteTextures(1, &depthTexId);
            return NULL;
        }
    }

    // GL_NEAREST is usually the appropriate filtering for depth textures. However,
    // NVIDIA GPUs (and possibly others) perform 'free' 4x percentage close filtering
    // when the filter is set to GL_LINEAR.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    TextureProperties texProps;
    texProps.addressS = TextureProperties::Clamp;
    texProps.addressT = TextureProperties::Clamp;
    texProps.useMipmaps = false;
    texProps.usage = TextureProperties::DepthTexture;

    TextureMap* tex = new TextureMap(depthTexId, texProps);
    if (tex)
    {
        glBindTexture(GL_TEXTURE_2D, tex->id());
        ApplyTextureProperties(texProps, GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    return tex;
}


/** Factory method for creating a cube map with six faces that are size x size texels.
  * The contents of the cube map are not initialized and will contain undefined image
  * data.
  *
  * \param width the width of the texture in pixels
  * \param height the height of the texture in pixels
  * \param format a valid color format
  *
  * \return either a valid, fully constructed cube map or NULL if there was
  * an error.
  */
TextureMap*
TextureMap::CreateCubeMap(unsigned int size, ImageFormat format)
{
    GLuint cubeMapId = 0;
    glGenTextures(1, &cubeMapId);
    if (cubeMapId == 0)
    {
        VESTA_WARNING("Failed to create cube map handle.");
        return NULL;
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cubeMapId);

    // Set the dimensions for all faces
    for (int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB + i, 0, ToGlInternalFormat(format), size, size, 0, ToGlFormat(format), GL_UNSIGNED_BYTE, NULL);
    }

    // Unbind it
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0);

    GLenum errorCode = glGetError();
    if (errorCode != GL_NO_ERROR)
    {
        const GLubyte* errorMessage = gluErrorString(errorCode);
        if (errorMessage)
        {
            VESTA_WARNING("OpenGL error occurred when creating cube map texture: %s", errorMessage);
            glDeleteTextures(1, &cubeMapId);
            return NULL;
        }
    }

    TextureProperties texProps;
    texProps.addressS = TextureProperties::Clamp;
    texProps.addressT = TextureProperties::Clamp;
    texProps.useMipmaps = false;

    TextureMap* tex = new TextureMap(cubeMapId, texProps);
    if (tex)
    {
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, tex->id());
        ApplyTextureProperties(texProps, GL_TEXTURE_CUBE_MAP_ARB);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 0);
    }

    return tex;
}
