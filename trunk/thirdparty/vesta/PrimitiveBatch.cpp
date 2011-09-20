/*
 * $Revision: 610 $ $Date: 2011-04-29 14:45:37 -0700 (Fri, 29 Apr 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "PrimitiveBatch.h"
#include "Debug.h"
#include <algorithm>
#include <exception>
#include <cassert>

using namespace vesta;


/** Copy constructor.
  */
PrimitiveBatch::PrimitiveBatch(const PrimitiveBatch& other) :
    m_primitiveType(other.m_primitiveType),
    m_indexSize(other.m_indexSize),
    m_indexData(0),
    m_primitiveCount(other.m_primitiveCount),
    m_firstVertex(other.m_firstVertex)
{
    if (other.m_indexData && m_primitiveCount > 0)
    {
        unsigned int length = indexCount();
        if (m_indexSize == Index16)
        {
            v_uint16* indexData = new v_uint16[length];
            std::copy(reinterpret_cast<v_uint16*>(other.m_indexData), reinterpret_cast<v_uint16*>(other.m_indexData) + length, indexData);
            m_indexData = indexData;
        }
        else
        {
            v_uint32* indexData = new v_uint32[length];
            std::copy(reinterpret_cast<v_uint32*>(other.m_indexData), reinterpret_cast<v_uint32*>(other.m_indexData) + length, indexData);
            m_indexData = indexData;
        }
    }
}


/** Create a new PrimitiveBatch wrapping the given set of 16-bit vertex
 *  indices. Note that the fact that the use of 16-bit indices means that
 *  a maximum of 65536 vertices can be addressed. For larger vertex sets,
 *  use 32-bit indices.
 *
 *  @param type the type of geometry represented in this primitive batch
 *  @param indices an array of 16-bit indices. A copy of the index data is
 *         stored in the PrimitiveBatch object.
 *  @param primitiveCount the number of primitives (which is generally less than the number of indices)
 */
PrimitiveBatch::PrimitiveBatch(PrimitiveType type, const v_uint16* indices, int primitiveCount) :
    m_primitiveType(type),
    m_indexSize(Index16),
    m_indexData(0),
    m_primitiveCount(primitiveCount),
    m_firstVertex(0)
{
    if (primitiveCount > 0)
    {
        unsigned int length = indexCount();
        v_uint16* indexData = new v_uint16[length];
        std::copy(indices, indices + length, indexData);
        m_indexData = indexData;
    }
}


/** Create a new PrimitiveBatch wrapping the given set of vertex indices.
 *  @param type the type of geometry represented in this primitive batch
 *  @param indices an array of 32-bit indices. A copy of the index data is
 *         stored in the PrimitiveBatch object.
 *  @param primitiveCount the number of primitives (which is generally less than the number of indices)
 */
PrimitiveBatch::PrimitiveBatch(PrimitiveType type, const v_uint32* indices, int primitiveCount) :
    m_primitiveType(type),
    m_indexSize(Index32),
    m_indexData(0),
    m_primitiveCount(primitiveCount),
    m_firstVertex(0)
{
    if (primitiveCount > 0)
    {
        unsigned int length = indexCount();
        v_uint32* indexData = new v_uint32[length];
        std::copy(indices, indices + length, indexData);
        m_indexData = indexData;
    }
}


/** Create a new PrimitiveBatch without explicit indices. This is used when there is
 *  a unique vertex per primitive point (i.e. if there were indices, they'd just be
 *  consecutive integers: 0, 1, 2, 3, 4, ...)
 *
 *  @param type the type of geometry represented in this primitive batch
 *  @param primitiveCount the number of primitives (which is generally less than the number of indices)
 *  @param firstVertex the index of the vertex at the hardware should begin fetching (default is 0)
 */
PrimitiveBatch::PrimitiveBatch(PrimitiveType type, int primitiveCount, unsigned int firstVertex) :
    m_primitiveType(type),
    m_indexSize(Index16),
    m_indexData(0),
    m_primitiveCount(primitiveCount),
    m_firstVertex(firstVertex)
{
    // Set this to a sensible value--even though it's not used internally for
    // unindexed data, an app author might call the indexSize() method.
    m_indexSize = firstVertex + indexCount() <= MaxIndex16 ? Index16 : Index32;
}

    
PrimitiveBatch::~PrimitiveBatch()
{
    freeIndices();
}


/** Get the number of indices in this primitive batch. The count
  * will vary based on the primitive count and type, e.g. for a
  * triangle list, it is three times the number of triangles.
  */
unsigned int
PrimitiveBatch::indexCount() const
{
    switch (m_primitiveType)
    {
        case Triangles:
            return m_primitiveCount * 3;
        case TriangleStrip:
            return m_primitiveCount + 2;
        case TriangleFan:
            return m_primitiveCount + 2;
        case Lines:
            return m_primitiveCount * 2;
        case LineStrip:
            return m_primitiveCount + 1;
        case Points:
            return m_primitiveCount;
        default:
            return 0;
    }
}


/** Return the highest vertex index referenced in this primitive
  * batch. This function can be used for validation when constructing
  * mesh. maxVertexIndex() should be less than the vertex count.
  */
unsigned int
PrimitiveBatch::maxVertexIndex() const
{
    if (m_indexData)
    {
        unsigned int length = indexCount();
        if (length == 0)
        {
            return 0;
        }
        else if (m_indexSize == Index16)
        {
            return *(std::max_element(static_cast<v_uint16*>(m_indexData), static_cast<v_uint16*>(m_indexData) + length));
        }
        else
        {
            return *(std::max_element(static_cast<v_uint32*>(m_indexData), static_cast<v_uint32*>(m_indexData) + length));
        }
    }
    else
    {
        return m_firstVertex + indexCount();
    }
}


/** Add an offset to all indices in the batch. 16-bit indices are automatically
  * promoted to 32-bit indices if adding an offset would generate an index
  * greater than 65535.
  *
  * \return true if the offset was apply successfully, false if there was a problem
  * (index too large, or promotion to 32-bit indices failed because of inadequate
  * memory.)
  */
bool
PrimitiveBatch::offsetIndices(unsigned int offset)
{
    unsigned int maxIndex = maxVertexIndex();
    if (maxIndex > MaxIndex32 - offset)
    {
        // Exceeded the maximum 32-bit index value
        return false;
    }

    if (m_indexData)
    {
        if (m_indexSize == Index16 && maxIndex + offset > MaxIndex16)
        {
            // New index will be too large to fit in 16-bits; promote to
            // 32-bits.
            bool ok = promoteTo32Bit();
            if (!ok)
            {
                return false;
            }
        }

        unsigned int nIndices = indexCount();
        if (m_indexSize == Index16)
        {
            v_uint16* indexData = reinterpret_cast<v_uint16*>(m_indexData);
            for (unsigned int i = 0; i < nIndices; ++i)
            {
                indexData[i] += v_uint16(offset);
            }
        }
        else
        {
            v_uint32* indexData = reinterpret_cast<v_uint32*>(m_indexData);
            for (unsigned int i = 0; i < nIndices; ++i)
            {
                indexData[i] += offset;
            }
        }
    }
    else
    {
        m_firstVertex += offset;
        if (indexCount() + m_firstVertex > MaxIndex16)
        {
            m_indexSize = Index32;
        }
    }

    return true;
}


/** Promote 16-bit indices to 32-bit indices. Return true if
  * the promotion was successful, or false if there was not enough
  * memory to expand the index array.
  */
bool
PrimitiveBatch::promoteTo32Bit()
{
    if (m_indexSize == Index16 && m_indexData)
    {
        unsigned int nIndices = indexCount();
        assert(nIndices > 0);

        try
        {
            v_uint16* indexData16 = reinterpret_cast<v_uint16*>(m_indexData);
            v_uint32* indexData32 = new v_uint32[nIndices];
            for (unsigned int i = 0; i < nIndices; ++i)
            {
                indexData32[i] = indexData16[i];
            }

            freeIndices();
            m_indexData = indexData32;
            m_indexSize = Index32;
        }
        catch (std::bad_alloc&)
        {
            return false;
        }
    }
    else
    {
        m_indexSize = Index32;
    }

    return true;
}


/** Compress 32-bit indices to 16-bit indices. Return true if
  * the promotion was successful, or false if compression was
  * not possible. If the batch already uses 16-bit indices, this
  * method has no effect and returns true.
  */
bool
PrimitiveBatch::compressTo16Bit()
{
    if (m_indexSize == Index16)
    {
        // Already 16-bit, nothing to do
        return true;
    }

    if (maxVertexIndex() > MaxIndex16)
    {
        // Can't compress index buffers that have index values > 65535
        return false;
    }

    // Skip unindexed batches (reporting success as long as the vertex
    // count is less or equal to MaxIndex16)
    if (!isIndexed())
    {
        return true;
    }

    unsigned int count = indexCount();
    v_uint16* index16 = new v_uint16[count];
    if (!index16)
    {
        VESTA_WARNING("Out of memory while compressing 32-bit indices to 16-bit. Indices will remain 32-bit.");
        return false;
    }

    v_uint32* index32 = static_cast<v_uint32*>(m_indexData);
    for (unsigned int i = 0; i < count; ++i)
    {
        index16[i] = v_uint16(index32[i]);
    }

    // Replace the old 32-bit indices with 16-bit indices
    delete[] index32;
    m_indexData = index16;
    m_indexSize = Index16;

    return true;
}


/** Remap indices using the specified map. Return true if successful,
  * false if there was an error.
  *
  * 32-bit indices will automatically be compacted to 16-bit indices
  * if the maximum vertex index is less than 2^16. This reduces memory
  * usage and can improve rendering performance.
  */
bool
PrimitiveBatch::remapIndices(const std::vector<v_uint32>& indexMap)
{
    v_uint32 maxVertexIndex = *max_element(indexMap.begin(), indexMap.end());

    if (!m_indexData)
    {
        bool ok = convertToIndexed();
        if (!ok)
        {
            return false;
        }
    }
    assert(m_indexData != NULL);

    unsigned int nIndices = indexCount();
    if (m_indexSize == Index16)
    {
        // Check for vertex indices that are too large to fit in 16-bits
        if (maxVertexIndex > MaxIndex16)
        {
            return false;
        }

        v_uint16* index16 = reinterpret_cast<v_uint16*>(m_indexData);
        for (unsigned int i = 0; i < nIndices; ++i)
        {
            index16[i] = v_uint16(indexMap[index16[i]]);
        }
    }
    else
    {
        v_uint32* index32 = reinterpret_cast<v_uint32*>(m_indexData);
        v_uint16* index16 = NULL;

        if (maxVertexIndex <= MaxIndex16)
        {
            // The remapped indices will fit in 16-bits. Optimize the index buffer
            // by converting it form 32- to 16-bits. If the allocation fails for some
            // reason, we'll skip the optimization
            index16 = new v_uint16[nIndices];
        }

        if (index16)
        {
            for (unsigned int i = 0; i < nIndices; ++i)
            {
                index16[i] = v_uint16(indexMap[index32[i]]);
            }

            // Replace the old 32-bit indices with 16-bit indices
            delete[] static_cast<v_uint32*>(m_indexData);
            m_indexData = index16;
            m_indexSize = Index16;
        }
        else
        {
            for (unsigned int i = 0; i < nIndices; ++i)
            {
                index32[i] = indexMap[index32[i]];
            }
        }
    }

    return true;
}


// Convert a range batch to a an index array
bool
PrimitiveBatch::convertToIndexed()
{
    if (m_indexData)
    {
        // Already indexed
        return true;
    }

    unsigned int nIndices = indexCount();

    try
    {
        if (m_indexSize == Index32)
        {
            v_uint32* indexData = new v_uint32[nIndices];
            for (unsigned int i = 0; i < nIndices; ++i)
            {
                indexData[i] = m_firstVertex + i;
            }
            m_indexData = indexData;
        }
        else
        {
            v_uint16* indexData = new v_uint16[nIndices];
            for (unsigned int i = 0; i < nIndices; ++i)
            {
                indexData[i] = v_uint16(m_firstVertex + i);
            }
            m_indexData = indexData;
        }
    }
    catch (std::bad_alloc&)
    {
        return false;
    }

    return true;
}


void
PrimitiveBatch::freeIndices()
{
    if (m_indexData)
    {
        if (m_indexSize == Index16)
        {
            delete[] static_cast<v_uint16*>(m_indexData);
        }
        else
        {
            delete[] static_cast<v_uint32*>(m_indexData);
        }
    }
}
