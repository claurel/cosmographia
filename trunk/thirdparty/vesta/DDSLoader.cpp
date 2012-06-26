/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "DDSLoader.h"
#include "IntegerTypes.h"
#include "Debug.h"
#include "OGLHeaders.h"
#include <algorithm>

using namespace vesta;
using namespace std;


#ifndef DDSD_CAPS

// DDS related defines
#define DDSD_CAPS           0x00000001
#define DDSD_HEIGHT	        0x00000002
#define DDSD_WIDTH          0x00000004
#define DDSD_PITCH          0x00000008
#define DDSD_PIXELFORMAT    0x00001000
#define DDSD_MIPMAPCOUNT    0x00020000
#define DDSD_LINEARSIZE     0x00080000
#define DDSD_DEPTH          0x00800000

#define DDPF_ALPHAPIXELS    0x00000001
#define DDPF_FOURCC         0x00000004
#define DDPF_RGB            0x00000040

#define DDSCAPS_COMPLEX     0x00000008
#define DDSCAPS_TEXTURE     0x00001000
#define DDSCAPS_MIPMAP      0x00400000

#define DDSCAPS2_CUBEMAP            0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX  0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX  0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY  0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY  0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ  0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ  0x00008000
#define DDSCAPS2_VOLUME             0x00200000


#define D3DFMT_A16B16G16R16                 36
#define D3DFMT_A16B16G16R16F        113
#define D3DFMT_DXT1                 0x31545844
#define D3DFMT_DXT3                 0x33545844
#define D3DFMT_DXT5                 0x35545844

#endif


namespace vesta
{

struct DDSPixelFormat
{
    v_uint32 dwSize;
    v_uint32 dwFlags;
    v_uint32 dwFourCC;
    v_uint32 dwRGBBitCount;
    v_uint32 dwRBitMask;
    v_uint32 dwGBitMask;
    v_uint32 dwBBitMask;
    v_uint32 dwABitMask;
};

struct DDSHeader
{
    v_uint32           dwSize;
    v_uint32           dwFlags;
    v_uint32           dwHeight;
    v_uint32           dwWidth;
    v_uint32           dwLinearSize;
    v_uint32           dwDepth;
    v_uint32           dwMipMapCount;
    v_uint32           dwReserved1[11];
    DDSPixelFormat     ddpf;
    v_uint32           dwCaps;
    v_uint32           dwCaps2;
    v_uint32           dwCaps3;
    v_uint32           dwCaps4;
    v_uint32           dwReserved2;
};

struct DDSFileHeader
{
    v_uint32           dwMagic;
    DDSHeader          header;
};

}


DDSLoader::DDSLoader()
{
}


DDSLoader::~DDSLoader()
{
}


union Uint32Bytes
{
    v_uint32 u;
    char bytes[4];
};


// Get the log base 2 of an integer (rounded down)
static int log2int(unsigned int x)
{
    int n = -1;
    while (x != 0)
    {
        x >>= 1;
        n++;
    }

    return n;
}


static bool isPow2(unsigned int x)
{
    return (x & (x - 1)) == 0 && x != 0;
}


// Byte swap a block of 32-bit integers
static void
ByteSwapBlock(v_uint32* data, unsigned int size)
{
    for (unsigned int i = 0; i < size; ++i)
    {
        Uint32Bytes ub;
        ub.u = data[i];
        swap(ub.bytes[0], ub.bytes[3]);
        swap(ub.bytes[1], ub.bytes[2]);
        data[i] = ub.u;
    }
}


/** Generate a GL texture from data read directly from a DDS file.
  */
bool
DDSLoader::load(TextureMap* tex, const DataChunk* data)
{
    if (data->size() < sizeof(DDSFileHeader))
    {
        reportError("DDS data is smaller than the valid header size.");
        return false;
    }

    DDSFileHeader fileHeader;
    copy(data->data(), data->data() + sizeof(DDSFileHeader), reinterpret_cast<char*>(&fileHeader));

    // DDS files are stored in little endian byte order. We will have to byte swap
    // header data on big endian machines. Texture data will also have to be swapped
    // when the component size is larger than one byte.
    bool byteSwap;
    if (fileHeader.dwMagic == 0x20534444)
    {
        byteSwap = false;
    }
    else if (fileHeader.dwMagic == 0x44445320)
    {
        byteSwap = true;
        ByteSwapBlock(reinterpret_cast<v_uint32*>(&fileHeader), sizeof(fileHeader));
    }
    else
    {
        reportError("Bad magic number in DDS file header.");
        return false;
    }

    if (fileHeader.header.ddpf.dwFourCC != 0)
    {
        TextureMap::ImageFormat format = TextureMap::InvalidFormat;
        switch (fileHeader.header.ddpf.dwFourCC)
        {
        case D3DFMT_DXT1:
            format = TextureMap::DXT1;
            break;
        case D3DFMT_DXT3:
            format = TextureMap::DXT3;
            break;
        case D3DFMT_DXT5:
            format = TextureMap::DXT5;
            break;
        default:
            break;
        }

        if (format == TextureMap::InvalidFormat)
        {
            reportError("DDS file contains an unsupported texture type.");
            return false;
        }
        else
        {
            return loadCompressedTexture(tex, data, &fileHeader.header, format);
        }
    }
    else
    {
        reportError("Only DDS compressed formats are currently supported.");
        return false;
    }
}


bool
DDSLoader::loadCompressedTexture(TextureMap* tex, const DataChunk* data, const DDSHeader* dds, TextureMap::ImageFormat format)
{
    if (!isPow2(dds->dwWidth) || !isPow2(dds->dwHeight))
    {
        reportError("DDS file has non-power-of-two dimensions (this limitation will be removed eventually)");
        return false;
    }

    unsigned int mipLevelCount = (unsigned int) log2int(max(dds->dwWidth, dds->dwHeight)) + 1;
    if (dds->dwMipMapCount > mipLevelCount)
    {
        reportError("DDS file contains too many mip levels.");
        return false;
    }

    // Not enough mipmaps in file; disable mipmaps
    if (dds->dwMipMapCount < mipLevelCount)
    {
        mipLevelCount = 1;
    }

    unsigned int imageDataSize = data->size() - sizeof(DDSFileHeader);
    unsigned int mipChainSize = TextureMap::MipmapChainSize(format, dds->dwWidth, dds->dwHeight, mipLevelCount);
    if (mipChainSize > imageDataSize)
    {
        tex->setStatus(TextureMap::LoadingFailed);
        reportError("Not enough data in DDS file to contain image.");
        return false;
    }

    tex->generateCompressedFit(data->data() + sizeof(DDSFileHeader), imageDataSize, dds->dwWidth, dds->dwHeight, format, mipLevelCount);

    return true;
}


void
DDSLoader::reportError(const std::string& message)
{
    m_errorMessage = message;
}
