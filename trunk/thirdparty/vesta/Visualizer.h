/*
 * $Revision: 530 $ $Date: 2010-10-12 11:26:43 -0700 (Tue, 12 Oct 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_VISUALIZER_H_
#define _VESTA_VISUALIZER_H_

#include "Object.h"
#include "Geometry.h"
#include <Eigen/Geometry>
#include <string>


namespace vesta
{

class Entity;
class PickContext;

/** A visualizer is extra geometry that represents something other than
  * the solid body of an object. Visualizers are attached to entities for
  * highlighting, labeling, showing regions of visibility, etc.
  */
class Visualizer : public Object
{
public:
    Visualizer(Geometry* geometry);
    virtual ~Visualizer();

    /** Return whether or not the visualizer is visible.
      */
    bool isVisible() const
    {
        return m_visible;
    }

    /** Set the visibility of the visualizer. The visible flag is set to true for
     *  newly constructed visualizer.
     */
    void setVisibility(bool visible)
    {
        m_visible = visible;
    }

    virtual Eigen::Quaterniond orientation(const Entity* parent, double t) const;

    /** Return the geometry for this visualizer.
      */
    Geometry* geometry() const
    {
        return m_geometry.ptr();
    }

    enum DepthAdjustment
    {
        NoAdjustment   = 0,
        AdjustToFront  = 1,
        AdjustToBack   = 2,
    };

    DepthAdjustment depthAdjustment() const
    {
        return m_depthAdjustment;
    }

    /** Set the depth adjustment for this visualizer. The depth adjustment
      * can be used to ensure that the visualizer will always appear either
      * in front of or behind the object that it is attached to.
      */
    void setDepthAdjustment(DepthAdjustment adjustment)
    {
        m_depthAdjustment = adjustment;
    }

    bool rayPick(const PickContext* pc, const Eigen::Vector3d& pickOrigin, double t) const;

protected:
    void setGeometry(Geometry* geometry)
    {
        m_geometry = geometry;
    }

    virtual bool handleRayPick(const PickContext* pc, const Eigen::Vector3d& pickOrigin, double t) const;
    virtual bool handleRayPick(const Eigen::Vector3d& pickOrigin,
                               const Eigen::Vector3d& pickDirection,
                               double pixelAngle) const;

private:
    bool m_visible;
    counted_ptr<Geometry> m_geometry;
    DepthAdjustment m_depthAdjustment;
};

}

#endif // _VESTA_VISUALIZER_H_
