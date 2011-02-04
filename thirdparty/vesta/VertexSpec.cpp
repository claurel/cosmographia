/*
 * $Revision: 366 $ $Date: 2010-07-19 13:31:56 -0700 (Mon, 19 Jul 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "VertexSpec.h"

using namespace vesta;


// Commonly used vertex specs
static VertexAttribute posNormAttributes[] = {
    VertexAttribute(VertexAttribute::Position,     VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::Normal,       VertexAttribute::Float3),
};

static VertexAttribute posNormTexAttributes[] = {
    VertexAttribute(VertexAttribute::Position,     VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::Normal,       VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::TextureCoord, VertexAttribute::Float2),
};

static VertexAttribute posColorAttributes[] = {
    VertexAttribute(VertexAttribute::Position,     VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::Color,        VertexAttribute::UByte4),
};

static VertexAttribute posColorTexAttributes[] = {
    VertexAttribute(VertexAttribute::Position,     VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::Color,        VertexAttribute::UByte4),
    VertexAttribute(VertexAttribute::TextureCoord, VertexAttribute::Float2),
};

static VertexAttribute posTexAttributes[] = {
    VertexAttribute(VertexAttribute::Position,     VertexAttribute::Float3),
    VertexAttribute(VertexAttribute::TextureCoord, VertexAttribute::Float2),
};

static VertexAttribute posAttributes[] = {
    VertexAttribute(VertexAttribute::Position,     VertexAttribute::Float3),
};

VertexSpec VertexSpec::PositionNormal   (2, posNormAttributes);
VertexSpec VertexSpec::PositionNormalTex(3, posNormTexAttributes);
VertexSpec VertexSpec::PositionTex      (2, posTexAttributes);
VertexSpec VertexSpec::PositionColor    (2, posColorAttributes);
VertexSpec VertexSpec::PositionColorTex (3, posColorTexAttributes);
VertexSpec VertexSpec::Position         (1, posAttributes);

// end standard vertex spec definitions

VertexSpec::VertexSpec(unsigned int attributeCount,
                       VertexAttribute* attributes,
                       unsigned int* attributeOffsets) :
    m_attributeCount(attributeCount),
    m_attributes(0),
    m_attributeOffsets(0)
{
    initAttributes(attributeCount, attributes);
    if (attributeOffsets)
        copyAttributeOffsets(attributeOffsets);
    else
        computeAttributeOffsets();
}


VertexSpec::VertexSpec(const VertexSpec& other) :
    m_attributeCount(other.attributeCount()),
    m_attributes(0),
    m_attributeOffsets(0),
    m_size(other.m_size)
{
    initAttributes(other.m_attributeCount, other.m_attributes);
    copyAttributeOffsets(other.m_attributeOffsets);
}


/** Assignment operator
  */
VertexSpec&
VertexSpec::operator=(const VertexSpec& other)
{
    if (this != &other)
    {
        delete[] m_attributes;
        m_attributes = 0;
        delete[] m_attributeOffsets;
        m_attributeOffsets = 0;

        initAttributes(other.m_attributeCount, other.m_attributes);
        copyAttributeOffsets(other.m_attributeOffsets);
        this->m_size = other.m_size;
    }

    return *this;
}


/** Equality operator
  */
bool
VertexSpec::operator==(const VertexSpec& other) const
{
    if (m_attributeCount != other.m_attributeCount)
    {
        return false;
    }

    for (unsigned int i = 0; i < m_attributeCount; ++i)
    {
        if (m_attributes[i] != other.m_attributes[i] ||
            m_attributeOffsets[i] != other.m_attributeOffsets[i])
        {
            return false;
        }
    }

    return true;
}


bool
VertexSpec::operator!=(const VertexSpec& other) const
{
    return !(*this == other);
}


VertexSpec::~VertexSpec()
{
    delete[] m_attributes;
    delete[] m_attributeOffsets;
}


// Internal method to initialize attributes from an array
void
VertexSpec::initAttributes(unsigned int attributeCount,
                           VertexAttribute* attributes)
{
    if (attributeCount == 0)
    {
        m_attributes = 0;
    }
    else
    {
        m_attributes = new VertexAttribute[attributeCount];
    }

    for (unsigned int i = 0; i < attributeCount; ++i)
    {
        m_attributes[i] = attributes[i];
    }
}


// Internal method
void
VertexSpec::copyAttributeOffsets(unsigned int* attributeOffsets)
{
    if (m_attributeCount != 0)
    {
        m_attributeOffsets = new unsigned int[attributeCount()];
        for (unsigned int i = 0; i < attributeCount(); i++)
        {
            m_attributeOffsets[i] = attributeOffsets[i];
        }
    }
}


// Internal method to calculate attribute offset automatically when
// explicit offsets are not provided.
void
VertexSpec::computeAttributeOffsets()
{
    if (m_attributeCount == 0)
    {
        m_attributeOffsets = 0;
    }
    else
    {
        m_attributeOffsets = new unsigned int[m_attributeCount];
    }
    
    m_size = 0;
    for (unsigned int i = 0; i < m_attributeCount; ++i)
    {
        m_attributeOffsets[i] = m_size;
        m_size += VertexAttribute::formatSize(m_attributes[i].format());
    }
}
