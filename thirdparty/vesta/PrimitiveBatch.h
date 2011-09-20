/*
 * $Revision: 368 $ $Date: 2010-07-19 17:29:08 -0700 (Mon, 19 Jul 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_PRIMITIVE_BATCH_H_
#define _VESTA_PRIMITIVE_BATCH_H_

#include "IntegerTypes.h"
#include <vector>


namespace vesta
{

class PrimitiveBatch
{
public:

    /** PrimitiveType describes how the vertices are used to
     *  contruct triangles. They correspondence with the OpenGL
     *  primitive types with similar names. Quads and polygons
     *  are not available as they are unsupported in D3D and have
     *  been deprecated in OpenGL 3.0.
     */
    enum PrimitiveType
    {
        Triangles,
        TriangleStrip,
        TriangleFan,
        Lines,
        LineStrip,
        Points
    };

    enum IndexSize
    {
        Index16,
        Index32,
    };

    static const unsigned int DefaultMaterialIndex = 0xffffffff;

    PrimitiveBatch(const PrimitiveBatch& other);
    PrimitiveBatch(PrimitiveType type, const v_uint16* indices, int primitiveCount);
    PrimitiveBatch(PrimitiveType type, const v_uint32* indices, int primitiveCount);
    PrimitiveBatch(PrimitiveType type, int primitiveCount, unsigned int firstVertex = 0);
    ~PrimitiveBatch();

    PrimitiveType primitiveType() const
    {
        return m_primitiveType;
    }

    IndexSize indexSize() const
    {
        return m_indexSize;
    }

    void* indexData() const
    {
        return m_indexData;
    }

    /** Get the number of primitives in this batch. Note that number of indices will be
      * different for most primitive types.
      */
    unsigned int primitiveCount() const
    {
        return m_primitiveCount;
    }

    /** Return the index of starting vertex for a unindexed primitive batch.
      */
    unsigned int firstVertex() const
    {
        return m_firstVertex;
    }

    /** Get the number of vertex indices in this primitive batch. This value is a function
      * of the primitive type and the number of primitives.
      */
    unsigned int indexCount() const;

    bool isIndexed() const
    {
        return m_indexData != 0;
    }

    unsigned int maxVertexIndex() const;

    bool offsetIndices(unsigned int offset);
    bool promoteTo32Bit();
    bool compressTo16Bit();
    bool remapIndices(const std::vector<v_uint32>& indexMap);

    static const v_uint16 MaxIndex16 = 0xffff;
    static const v_uint32 MaxIndex32 = 0xffffffff;

private:
    void freeIndices();
    bool convertToIndexed();

private:
    PrimitiveType m_primitiveType;
    IndexSize m_indexSize;
    void* m_indexData;
    unsigned int m_primitiveCount;
    unsigned int m_firstVertex;
};

}

#endif // _VESTA_VERTEX_ARRAY_H_
