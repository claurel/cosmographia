/*
 * $Revision: 622 $ $Date: 2011-09-21 16:03:59 -0700 (Wed, 21 Sep 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_SUBMESH_H_
#define _VESTA_SUBMESH_H_

#include "Object.h"
#include "VertexArray.h"
#include "PrimitiveBatch.h"
#include "BoundingBox.h"
#include <vector>

namespace vesta
{

class Submesh : public Object
{
public:
    Submesh(VertexArray* vertices);
    ~Submesh();

    void addPrimitiveBatch(PrimitiveBatch* batch, unsigned int materialIndex = DefaultMaterialIndex);

    const VertexArray* vertices() const
    {
        return m_vertices;
    }

    const std::vector<PrimitiveBatch*>& primitiveBatches() const
    {
        return m_primitiveBatches;
    }

    unsigned int primitiveBatchCount() const
    {
        return m_primitiveBatches.size();
    }

    const std::vector<unsigned int>& materials() const
    {
        return m_materials;
    }

    void setMaterial(unsigned int batchIndex, unsigned int materialIndex);

    BoundingBox boundingBox() const
    {
        return m_boundingBox;
    }

    /** Get the radius of an origin centered sphere large enough
      * to contain the entire submesh.
      */
    float boundingSphereRadius() const
    {
        return m_boundingSphereRadius;
    }

    bool
    rayPick(const Eigen::Vector3d& pickOrigin,
            const Eigen::Vector3d& pickDirection,
            double* distance) const;

    static Submesh* mergeSubmeshes(const std::vector<Submesh*>& submeshes);
    bool uniquifyVertices(float positionTolerance = 0.0f, float normalTolerance = 0.0f, float texCoordTolerance = 0.0f);
    void compressIndices();

    bool mergeMaterials();

    static const unsigned int DefaultMaterialIndex = 0xffffffff;

private:
    VertexArray* m_vertices;
    std::vector<PrimitiveBatch*> m_primitiveBatches;
    std::vector<unsigned int> m_materials;
    BoundingBox m_boundingBox;
    float m_boundingSphereRadius;
};

}

#endif // _VESTA_SUBMESH_H_

