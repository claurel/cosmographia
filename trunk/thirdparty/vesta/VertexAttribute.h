/*
 * $Revision: 368 $ $Date: 2010-07-19 17:29:08 -0700 (Mon, 19 Jul 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_VERTEX_ATTRIBUTE_H_
#define _VESTA_VERTEX_ATTRIBUTE_H_

#include "IntegerTypes.h"


namespace vesta
{

class VertexAttribute
{
public:
    enum Semantic
    {
        Position     = 0,
        Normal       = 1,
        TextureCoord = 2,
        Color        = 3,
        PointSize    = 4,
        Tangent      = 5,
        InvalidAttributeSemantic = -1,
    };

    enum Format
    {
        Float1 = 0,
        Float2 = 1,
        Float3 = 2,
        Float4 = 3,
        UByte4 = 4,
        InvalidAttributeFormat = -1,
    };

    // All components are four bytes
    union Component
    {
        float f;
        v_uint32 u;
    };

    VertexAttribute() :
        m_semantic(InvalidAttributeSemantic),
        m_format(InvalidAttributeFormat)
    {
    }

    VertexAttribute(Semantic semantic, Format format) :
        m_semantic(semantic),
        m_format(format)
    {
    }


    bool operator==(const VertexAttribute& other) const
    {
        return m_semantic == other.m_semantic && m_format == other.m_format;
    }


    bool operator!=(const VertexAttribute& other) const
    {
        return m_semantic != other.m_semantic || m_format != other.m_format;
    }


    Semantic semantic() const
    {
        return m_semantic;
    }

    Format format() const
    {
        return m_format;
    }

    static unsigned int formatSize(Format format)
    {
        switch (format)
        {
        case Float1: return 4;
        case Float2: return 8;
        case Float3: return 12;
        case Float4: return 16;
        case UByte4: return 4;
        default: return 0;
        }
    }

private:
    Semantic m_semantic;
    Format m_format;
};

}

#endif // _VESTA_VERTEX_ATTRIBUTE_H_
