// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

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

protected:
    virtual bool handleRayPick(const Eigen::Vector3d& pickOrigin,
                               const Eigen::Vector3d& pickDirection,
                               double clock,
                               double* distance ) const;

private:
    vesta::counted_ptr<vesta::MeshGeometry> m_mesh;
    float m_scale;
    Eigen::Quaternionf m_meshRotation;
};

#endif // _MESH_INSTANCE_GEOMETRY_H_

