/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_VERTEX_POOL_H_
#define _VESTA_VERTEX_POOL_H_

#include <Eigen/Core>
#include <vector>


namespace vesta
{

class VertexArray;
class VertexSpec;

/** VertexPool is a helper class designed to make it easy to create
  * mesh geometry. Typically, a function will create the vertex pool,
  * add some vertex attributes, then call createVertexArray(). For
  * example, the following code could be used to create a square:
  *
  * \code
  * VertexPool pool;
  * pool.addVec3(-1, -1, 0);
  * pool.addVec3(-1,  1, 0);
  * pool.addVec3( 1,  1, 0);
  * pool.addVec3( 1, -1, 0);
  * VertexArray* va = pool.createVertexArray(4, VertexSpec::Position);
  * \endcode
  */
class VertexPool
{
public:
    VertexPool();
    VertexPool(const VertexPool& other);
    ~VertexPool();

    VertexPool& operator=(const VertexPool& other);

    unsigned int size() const;

    /** Add a single floating point attribute to the vertex pool. */
    void addFloat(float x)
    {
        m_vertexData.push_back(x);
    }

    /** Add a 2-vector attribute to the vertex pool. */
    void addVec2(float x, float y)
    {
        m_vertexData.push_back(x);
        m_vertexData.push_back(y);
    }

    /** Add a 2-vector attribute to the vertex pool. */
    void addVec2(const float* data)
    {
        m_vertexData.push_back(data[0]);
        m_vertexData.push_back(data[1]);
    }

    /** Add a 2-vector attribute to the vertex pool. */
    void addVec2(const Eigen::Vector2f v)
    {
        addVec2(v.data());
    }

    /** Add a 3-vector attribute to the vertex pool. */
    void addVec3(float x, float y, float z)
    {
        m_vertexData.push_back(x);
        m_vertexData.push_back(y);
        m_vertexData.push_back(z);
    }

    /** Add a 3-vector attribute to the vertex pool. */
    void addVec3(const float* data)
    {
        m_vertexData.push_back(data[0]);
        m_vertexData.push_back(data[1]);
        m_vertexData.push_back(data[2]);
    }

    /** Add a 3-vector attribute to the vertex pool. */
    void addVec3(const Eigen::Vector3f v)
    {
        addVec3(v.data());
    }

    VertexArray* createVertexArray(unsigned int vertexCount, const VertexSpec& vertexSpec) const;

private:
    std::vector<float> m_vertexData;
};

}

#endif // _VESTA_VERTEX_POOL_H_
