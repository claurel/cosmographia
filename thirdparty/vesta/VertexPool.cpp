/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "VertexPool.h"
#include "VertexArray.h"
#include <algorithm>

using namespace vesta;
using namespace std;


VertexPool::VertexPool()
{
}


VertexPool::VertexPool(const VertexPool& other) :
    m_vertexData(other.m_vertexData)
{
}


VertexPool::~VertexPool()
{
}


VertexPool& VertexPool::operator=(const VertexPool& other)
{
    if (&other != this)
    {
        m_vertexData = other.m_vertexData;
    }
    return *this;
}


unsigned int
VertexPool::size() const
{
    return m_vertexData.size();
}


/** Create a new vertex array from this vertex pool. Return a pointer
  * to the new array, or null if the array could not be created (if
  * the vertex pool isn't large enough for the requested vertex array
  * size.)
  *
  * @param vertexCount Number of vertices in the array. A vertexCount of
  *        zero is illegal.
  */
VertexArray*
VertexPool::createVertexArray(unsigned int vertexCount,
                              const VertexSpec& vertexSpec) const
{
    if (vertexCount == 0)
    {
        return 0;
    }

    if (vertexSpec.size() * vertexCount > size() * 4)
    {
        // VertexPool is too small.
        return 0;
    }

    float* vertexDataCopy = new float[m_vertexData.size()];
    copy(m_vertexData.begin(), m_vertexData.end(), vertexDataCopy);

    return new VertexArray(vertexDataCopy, vertexCount, vertexSpec);
}
