/*
 * $Revision: 600 $ $Date: 2011-03-31 18:37:21 -0700 (Thu, 31 Mar 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "TextureFont.h"
#include "DataChunk.h"
#include "Debug.h"
#include "internal/InputDataStream.h"
#include "internal/DefaultFont.h"
#include <GL/glew.h>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


static const unsigned int InvalidGlyphIndex = ~0u;


counted_ptr<TextureFont> TextureFont::ms_defaultFont;


/** Create a new texture font with no glyphs and an undefined 
 *  glyph texture.
 */
TextureFont::TextureFont() :
    m_maxCharacterId(0),
    m_maxAscent(0.0f),
    m_maxDescent(0.0f)
{
}


TextureFont::~TextureFont()
{
}


/** Find the glyph representing the specified character ID.
  *
  * @return A pointer the to glyph record, or NULL if the font doesn't
  * defined a glyph for the character.
  */
const TextureFont::Glyph*
TextureFont::lookupGlyph(wchar_t ch) const
{
    unsigned int charIndex = (unsigned int) ch;
    if (charIndex < m_characterSet.size())
    {
        unsigned int glyphIndex = m_characterSet[charIndex];
        if (glyphIndex != InvalidGlyphIndex)
        {
            return &m_glyphs[glyphIndex];
        }
    }

    return 0;
}


Vector2f
TextureFont::render(const string& text, const Vector2f& startPosition) const
{
    Vector2f currentPosition = startPosition;

    glBegin(GL_QUADS);
    for (unsigned int i = 0; i < text.length(); ++i)
    {
        // The cast to unsigned char is critical for glyph lookup to work correctly;
        // otherwise, extended characters will generate negative indices.
        const Glyph* glyph = lookupGlyph((unsigned char) text[i]);

        if (glyph)
        {
            Vector2f p = currentPosition + glyph->offset;

            glTexCoord2fv(glyph->textureCoords[0].data());
            glVertex2f(p.x(), p.y());
            glTexCoord2fv(glyph->textureCoords[1].data());
            glVertex2f(p.x() + glyph->size.x(), p.y());
            glTexCoord2fv(glyph->textureCoords[2].data());
            glVertex2f(p.x() + glyph->size.x(), p.y() + glyph->size.y());
            glTexCoord2fv(glyph->textureCoords[3].data());
            glVertex2f(p.x(), p.y() + glyph->size.y());

            currentPosition.x() += glyph->advance;
        }
    }
    glEnd();

    return currentPosition;
}


/** Compute the width of a string of text in pixels.
 */
float
TextureFont::textWidth(const string& text) const
{
    float width = 0.0f;

    for (unsigned int i = 0; i < text.length(); ++i)
    {
        const Glyph* glyph = lookupGlyph((unsigned char) text[i]);
        if (glyph)
        {
            width += glyph->advance;
        }
    }

    return width;
}


/** Get the maximum height above the baseline of any glyph in the font.
  * The returned value is in units of pixels.
  */
float
TextureFont::maxAscent() const
{
    return m_maxAscent;
}


/** Get the maximum distance that any glyph extends below the baseline.
  * The returned value is in units of pixels.
  */
float
TextureFont::maxDescent() const
{
    return m_maxDescent;
}


/** Bind the font texture. */
void
TextureFont::bind() const
{
    if (!m_glyphTexture.isNull())
    {
        glBindTexture(GL_TEXTURE_2D, m_glyphTexture->id());
    }
}


/** Generate an OpenGL texture with all the glyph bitmaps for this font.
 *
 * @param width width of the font texture
 * @param height height of the font texture
 * @param pixels an array of pixels with dimensions width * height. Each
 *        pixel is 8-bit value with 0 = transparent, 255 = opaque, and other
 *        values indicating intermediate opacities
 * @return true if the font texture was created successfully
 */
bool
TextureFont::buildFontTexture(unsigned int width,
                              unsigned int height,
                              unsigned char* pixels)
{
    GLuint texId = 0;

    glGenTextures(1, &texId);
    if (texId == 0)
    {
        return false;
    }

    // Delete the old texture (if any)

    glBindTexture(GL_TEXTURE_2D, texId);

    // Disable filtering to prevent blurriness
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_ALPHA,
                 width, height,
                 0,
                 GL_ALPHA,
                 GL_UNSIGNED_BYTE,
                 pixels);

    TextureProperties glyphTextureProperties(TextureProperties::Clamp);
    glyphTextureProperties.usage = TextureProperties::AlphaTexture;

    m_glyphTexture = new TextureMap(texId, glyphTextureProperties);

    return true;
}


/** Add a new glyph to the font.
 */
void
TextureFont::addGlyph(const Glyph& glyph)
{
    m_glyphs.push_back(glyph);
    m_maxCharacterId = max(m_maxCharacterId, glyph.characterId);
    m_maxAscent = max(m_maxAscent, glyph.size.y() + glyph.offset.y());
    m_maxDescent = max(m_maxDescent, -glyph.offset.y());
}


/** Build the table that maps character ids to glyphs.
 */
void
TextureFont::buildCharacterSet()
{
    // Initialize an empty character table of adequate size
    m_characterSet.resize((unsigned int) m_maxCharacterId + 1);
    for (unsigned int i = 0; i <= m_maxCharacterId; ++i)
    {
        m_characterSet[i] = InvalidGlyphIndex;
    }

    for (vector<Glyph>::const_iterator iter = m_glyphs.begin();
         iter != m_glyphs.end(); iter++)
    {
        if (iter->characterId < m_characterSet.size())
        {
            m_characterSet[iter->characterId] = (unsigned int) (iter - m_glyphs.begin());
        }
    }
}


/** Load a texture font from a chunk of data containing font data
  * in the TXF format used by GLUT.
  *
  * \returns true if the data is a valid font and the font texture
  * could be created.
  */
bool
TextureFont::loadTxf(const DataChunk* data)
{
    string str(data->data(), data->size());
    InputDataStream in(str);
    in.setByteOrder(InputDataStream::BigEndian);

    char header[4];
    in.readData(header, sizeof(header));
    if (in.status() != InputDataStream::Good)
    {
        VESTA_LOG("Incomplete header in texture font.");
        return NULL;
    }

    if (string(header, sizeof(header)) != "\377txf")
    {
        VESTA_LOG("Bad header in texture font file.");
        return NULL;
    }

    v_uint32 endianness = in.readUint32();
    if (endianness == 0x12345678)
    {
        in.setByteOrder(InputDataStream::BigEndian);
    }
    else if (endianness == 0x78563412)
    {
        in.setByteOrder(InputDataStream::LittleEndian);
    }
    else
    {
        VESTA_LOG("Bad endianness in texture font header.");
        return NULL;
    }

    v_uint32 format = in.readUint32();
    v_uint32 glyphTextureWidth = in.readUint32();
    v_uint32 glyphTextureHeight = in.readUint32();
    v_uint32 maxAscent = in.readUint32();
    v_uint32 maxDescent = in.readUint32();
    v_uint32 glyphCount = in.readUint32();

    if (in.status() != InputDataStream::Good)
    {
        VESTA_LOG("Error reading texture font header values.");
        return NULL;
    }

    if (format != 0)
    {
        VESTA_LOG("Texture font has wrong type (bitmap fonts not supported.)");
        return NULL;
    }

    if (glyphTextureWidth == 0 || glyphTextureWidth > 4096 ||
        glyphTextureHeight == 0 || glyphTextureHeight > 4096)
    {
        VESTA_LOG("Bad glyph texture size in font (%dx%d)", glyphTextureWidth, glyphTextureHeight);
        return NULL;
    }

    Vector2f texelScale(1.0f / glyphTextureWidth, 1.0f / glyphTextureHeight);
    Vector2f halfTexel = texelScale * 0.5f;

    for (v_uint32 i = 0; i < glyphCount; i++)
    {
        v_uint16 characterId = in.readUint16();
        v_uint8 glyphWidth = in.readUbyte();
        v_uint8 glyphHeight = in.readUbyte();
        v_int8 xoffset = in.readByte();
        v_int8 yoffset = in.readByte();
        v_int8 advance = in.readByte();
        v_int8 unused = in.readByte();
        v_uint16 x = in.readUint16();
        v_uint16 y = in.readUint16();

        if (in.status() != InputDataStream::Good)
        {
            VESTA_LOG("Error reading glyph %d in texture font.", i + 1);
            return false;
        }

        Vector2f normalizedSize = texelScale.cwise() * Vector2f(glyphWidth, glyphHeight);
        Vector2f normalizedPosition = texelScale.cwise() * Vector2f(x, y) + halfTexel;

        TextureFont::Glyph glyph;
        glyph.characterId = characterId;
        glyph.advance = advance;
        glyph.offset = Vector2f(xoffset, yoffset);
        glyph.size = Vector2f(glyphWidth, glyphHeight);
        glyph.textureCoords[0] = normalizedPosition;
        glyph.textureCoords[1] = normalizedPosition + Vector2f(normalizedSize.x(), 0.0f);
        glyph.textureCoords[2] = normalizedPosition + normalizedSize;
        glyph.textureCoords[3] = normalizedPosition + Vector2f(0.0f, normalizedSize.y());

        addGlyph(glyph);
    }

    unsigned int pixelCount = glyphTextureWidth * glyphTextureHeight;
    unsigned char* pixels = new unsigned char[pixelCount];
    in.readData(reinterpret_cast<char*>(pixels), pixelCount);
    if (in.status() != InputDataStream::Good)
    {
        VESTA_LOG("Error reading pixel data in texture font.");
        delete[] pixels;
        return false;
    }

    buildCharacterSet();
    buildFontTexture(glyphTextureWidth, glyphTextureHeight, pixels);

    delete[] pixels;

    return true;
}


/** Load a texture font from a chunk of data containing font data
  * in the TXF format used by GLUT.
  */
TextureFont*
TextureFont::LoadTxf(const DataChunk* data)
{
    TextureFont* font = new TextureFont();
    if (font->loadTxf(data))
    {
        // TODO: Should skip the addRef
        font->addRef();
    }
    else
    {
        delete font;
        font = NULL;
    }

    return font;
}


/** Get the default font. This will always be available provided that
  * an OpenGL has been initialized (or more precisely, that there is a current
  * and valid OpenGL context.)
  */
TextureFont*
TextureFont::GetDefaultFont()
{
    if (ms_defaultFont.isNull())
    {
        VESTA_LOG("Creating default font...");
        DataChunk* data = GetDefaultFontData();
        if (data == NULL)
        {
            VESTA_WARNING("Internal error occurred when creating default font.");
        }
        else
        {
            TextureFont* font = LoadTxf(data);
            if (!font)
            {
                VESTA_WARNING("Failed to create default font. Font data is not valid.");
            }
            ms_defaultFont = font;
        }
    }

    return ms_defaultFont.ptr();
}
