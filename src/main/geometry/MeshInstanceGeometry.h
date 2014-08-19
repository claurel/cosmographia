// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _MESH_INSTANCE_GEOMETRY_H_
#define _MESH_INSTANCE_GEOMETRY_H_

#include <vesta/MeshGeometry.h>


/** MeshInstanceGeometry is a wrapper for a VESTA MeshGeometry. It allows separate scale factors to be
  * be assigned to the same mesh geometry.
  */
class MeshInstanceGeometry : public vesta::Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    MeshInstanceGeometry(vesta::MeshGeometry* mesh);
    virtual ~MeshInstanceGeometry();

    void render(vesta::RenderContext& rc,
                double animationClock) const;
    void renderShadow(vesta::RenderContext& rc,
                      double animationClock) const;

    float boundingSphereRadius() const;

    /** \reimp */
    bool isOpaque() const
    {
        return m_mesh.isNull() || m_mesh->isOpaque();
    }

    /** Set the scale factor that will be applied to the mesh. The scale
      * factor is multiplied by the scale factor of the mesh geometry.
      */
    void setScale(float scale)
    {
        m_scale = scale;
    }

    /** Get the uniform scaling factor for the mesh.
      */
    float scale() const
    {
        return m_scale;
    }

    /** Set the translation that will be applied after scaling and before
      * the rotation.
      */
    void setMeshOffset(const Eigen::Vector3f& v)
    {
        m_meshOffset = v;
    }

    /** Get the translation that is applied after scaling and before the
      * rotation.
      */
    Eigen::Vector3f meshOffset() const
    {
        return m_meshOffset;
    }

    /** Set the mesh rotation. This is an extra rotation applied before
      * the rotation model and frame rotations. The default mesh orientation
      * is the identity.
      */
    void setMeshRotation(const Eigen::Quaternionf& q)
    {
        m_meshRotation = q;
    }

    /** Get the mesh rotation. This is an extra rotation applied before
      * the rotation model and frame rotations.
      */
    Eigen::Quaternionf meshRotation() const
    {
        return m_meshRotation;
    }

    vesta::MeshGeometry* mesh() const
    {
        return m_mesh.ptr();
    }

    vesta::BoundingBox boundingBox() const;

protected:
    virtual bool handleRayPick(const Eigen::Vector3d& pickOrigin,
                               const Eigen::Vector3d& pickDirection,
                               double clock,
                               double* distance ) const;

private:
    vesta::counted_ptr<vesta::MeshGeometry> m_mesh;
    float m_scale;
    Eigen::Vector3f m_meshOffset;
    Eigen::Quaternionf m_meshRotation;
};

#endif // _MESH_INSTANCE_GEOMETRY_H_

