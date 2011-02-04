/*
 * $Revision: 403 $ $Date: 2010-08-03 13:02:31 -0700 (Tue, 03 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_VERTEX_SPEC_H_
#define _VESTA_VERTEX_SPEC_H_

#include "VertexAttribute.h"

namespace vesta
{

/** An instance of VertexSpec describes the layout of a vertex in memory. The
  * specification is composed of one or more vertex attributes, each of which
  * has a data type, byte offset, and semantic, which indicates how the
  * attribute will be used (position, surface normal, color, etc.)
  *
  * For convenience, several standard vertex specs are available as static
  * members of the class:
  *    Position
  *    PositionNormal
  *    PositionTex
  *    PositionNormalTex
  *    PositionColor
  *    PositionColorTex
  *
  * The name of each of these standard VertexSpecs gives the order of the
  * attributes within the vertex.
  */
class VertexSpec
{
public:
    enum { InvalidAttribute = 1000 };

    VertexSpec(unsigned int attributeCount,
               VertexAttribute* attributes,
               unsigned int* attributeOffsets = 0);
    VertexSpec(const VertexSpec& other);
    ~VertexSpec();

    VertexSpec& operator=(const VertexSpec& other);

    bool operator==(const VertexSpec& other) const;
    bool operator!=(const VertexSpec& other) const;

    /** Get the number of attributes in the vertex spec.
      */
    unsigned int attributeCount() const
    {
        return m_attributeCount;
    }

    /** Get the attribute with the specified index. The index
      * must be valid (0 <= index < attributeCount)
      */
    VertexAttribute attribute(unsigned int index) const
    {
        return m_attributes[index];
    }

    /** Get the byte offset of an attribute within a vertex. The index
      * must be valid (0 <= index < attributeCount)
      */
    unsigned int attributeOffset(unsigned int index) const
    {
        return m_attributeOffsets[index];
    }

    /** Return the index of the attribute with the requested semantic.
      * Returns VertexSpec::InvalidAttribute if an attribute with the
      * requested semantic isn't present in this vertex spec.
      */
    unsigned int attributeIndex(VertexAttribute::Semantic semantic) const
    {
        for (unsigned int i = 0; i < m_attributeCount; ++i)
        {
            if (m_attributes[i].semantic() == semantic)
            {
                return i;
            }
        }

        return InvalidAttribute;
    }

    /** Get the size of a vertex in bytes.
      */
    unsigned int size() const
    {
        return m_size;
    }

public:
    static VertexSpec PositionNormal;
    static VertexSpec PositionNormalTex;
    static VertexSpec PositionTex;
    static VertexSpec PositionColor;
    static VertexSpec PositionColorTex;
    static VertexSpec Position;

private:
    void initAttributes(unsigned int attributeCount,
                        VertexAttribute* attributes);
    void copyAttributeOffsets(unsigned int* attributeOffsets);
    void computeAttributeOffsets();

private:
    unsigned int m_attributeCount;
    VertexAttribute* m_attributes;
    unsigned int* m_attributeOffsets;
    unsigned int m_size;
};

}

#endif // _VESTA_VERTEX_ARRAY_H_
