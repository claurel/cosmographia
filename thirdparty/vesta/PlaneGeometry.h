/*
 * $Revision: 477 $ $Date: 2010-08-31 11:49:37 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_PLANE_GEOMETRY_H_
#define _VESTA_PLANE_GEOMETRY_H_

#include <Eigen/Core>
#include "Geometry.h"
#include "Submesh.h"
#include "Material.h"
#include <vector>

namespace vesta
{
class PrimitiveBatch;
class VertexArray;

/** PlaneGeometry is a Geometry object that is not yet documented
  */
class PlaneGeometry : public Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    PlaneGeometry();
    virtual ~PlaneGeometry();

    virtual void render(RenderContext& rc,
                        double clock) const;

    virtual float boundingSphereRadius() const;

    virtual bool isOpaque() const;

    double scale() const
    {
        return m_scale;
    }

    void setScale(double scale)
    {
        m_scale = scale;
    }

    Spectrum color() const
    {
        return m_color;
    }

    void setColor(const Spectrum& color)
    {
        m_color = color;
    }

    /** Get the opacity of the plane. */
    float opacity() const
    {
        return m_opacity;
    }

    /** Set the opacity of the plane. The opacity is a value between
      * 0 and 1, with 0 indicating the plane is completely transparent,
      * and 1 meaning completely opaque. The grid lines are always
      * drawn completely opaque.
      */
    void setOpacity(float opacity)
    {
        m_opacity = opacity;
    }

    /** Get the number of kilometers between each grid line.
      */
    double gridLineSpacing() const
    {
        return m_gridLineSpacing;
    }

    void setGridLineSpacing(double gridLineSpacing);

private:
    void buildGeometry();

private:
    double m_scale;
    Spectrum m_color;
    float m_opacity;
    double m_gridLineSpacing;

    PrimitiveBatch* m_grid;
    PrimitiveBatch* m_solidPlane;
    VertexArray* m_vertices;
};

}

#endif // _VESTA_PLANE_GEOMETRY_H_

