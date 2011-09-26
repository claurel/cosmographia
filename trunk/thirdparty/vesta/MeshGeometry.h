/*
 * $Revision: 622 $ $Date: 2011-09-21 16:03:59 -0700 (Wed, 21 Sep 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_MESH_GEOMETRY_H_
#define _VESTA_MESH_GEOMETRY_H_

#include <Eigen/Core>
#include "Geometry.h"
#include "Submesh.h"
#include "Material.h"
#include <vector>

namespace vesta
{
class PrimitiveBatch;
class VertexArray;
class TextureMapLoader;
class GLVertexBuffer;

/** MeshGeometry is a Geometry object for triangle meshes, typically
  * loaded from a 3D model file.
  *
  * Optimization of meshes:
  * Often, 3D mesh files are not well-conditioned for rendering on
  * graphics hardware. They may contain redundant vertexes and materials.
  * Or, the geometry may be split into many small chunks that result
  * in extra overhead for the hardware or driver. MeshGeometry has three
  * methods to preprocess meshes for better hardware performance:
  *
  * - mergeSubmeshes
  * - unquifyVertices
  * - mergeMaterials
  *
  * For the best possible results, all three methods should be called
  * after a model is loaded. The sequence is important: the methods should
  * be called in the order given above.
  *
  * If mesh files are saved in optimized form, then preprocessing at load
  * time can be skipped. This is ideal, as the optimization functions can
  * require significant amounts of computation for complex models with many
  * triangles.
  */
class MeshGeometry : public Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    MeshGeometry();
    virtual ~MeshGeometry();

    void render(RenderContext& rc,
                double animationClock) const;
    void renderShadow(RenderContext& rc,
                      double animationClock) const;

    float boundingSphereRadius() const;

    void addSubmesh(Submesh* submesh);
    void addMaterial(Material* material);

    unsigned int materialCount() const
    {
        return m_materials.size();
    }

    Material* material(unsigned int index) const
    {
        if (index < m_materials.size())
        {
            return m_materials[index].ptr();
        }
        else
        {
            return 0;
        }
    }

    Eigen::Vector3f meshScale() const
    {
        return m_meshScale;
    }

    /** Set the scale factor that will be applied along each axis of the
      * mesh. The default scale is one in all directions.
      */
    void setMeshScale(const Eigen::Vector3f& scale)
    {
        m_meshScale = scale;
    }

    /** Set the uniform scaling factor for the mesh. */
    void setMeshScale(float scale)
    {
        m_meshScale.fill(scale);
    }

    /** Return the bounding box of the mesh, not accounting for the mesh
      * scale factor.
      */
    BoundingBox meshBoundingBox() const
    {
        return m_boundingBox;
    }

    void setMeshChanged();

    bool mergeSubmeshes();
    bool uniquifyVertices(float positionTolerance = 0.0f, float normalTolerance = 0.0f, float texCoordTolerance = 0.0f);
    bool mergeMaterials();
    void compressIndices();

    static MeshGeometry* loadFromFile(const std::string& filename, TextureMapLoader* textureLoader);

private:
    void freeSubmeshBuffers() const;
    bool realize() const;

protected:
    virtual bool handleRayPick(const Eigen::Vector3d& pickOrigin,
                               const Eigen::Vector3d& pickDirection,
                               double clock,
                               double* distance ) const;

private:
    std::vector< counted_ptr<Submesh> > m_submeshes;
    std::vector< counted_ptr<Material> > m_materials;
    float m_boundingSphereRadius;
    BoundingBox m_boundingBox;
    Eigen::Vector3f m_meshScale;

    // TODO: these values are only mutable because the render() method
    // must be const. Consider changing this requirement.
    mutable std::vector<GLVertexBuffer*> m_submeshBuffers;
    mutable bool m_hwBuffersCurrent;
};

}

#endif // _VESTA_MESH_GEOMETRY_H_

