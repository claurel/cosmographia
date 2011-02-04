/*
 * $Revision: 402 $ $Date: 2010-08-03 13:00:55 -0700 (Tue, 03 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_ENTITY_H_
#define _VESTA_ENTITY_H_

#include "Object.h"
#include "Chronology.h"
#include "StateVector.h"
#include "LightSource.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <string>
#include <map>

namespace vesta
{
class Geometry;
class Visualizer;

class Entity : public Object
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Entity();
    ~Entity();

    Eigen::Vector3d position(double t) const;
    StateVector state(double t) const;
    Eigen::Quaterniond orientation(double t) const;
    Eigen::Vector3d angularVelocity(double t) const;

    /** Return the geometry object assigned to this entity. It is
      * legal for an entity not to have any geometry at all (for
      * entities such as barycenters, other dynamical points, and
      * 'placeholder' entities.)
      */
    virtual Geometry* geometry() const
    {
        return NULL;
    }

    const Chronology* chronology() const
    {
        return &*m_chronology;
    }

    Chronology* chronology()
    {
        return &*m_chronology;
    }

    /** Return true if this entity's visible attribute is set.
      */
    bool isVisible() const
    {
        return m_visible;
    }

    /** Return true if this entity's visible attribute is set and
      * it exists at the specified time.
      * @param t time in seconds since J2000.0
      */
    bool isVisible(double t) const
    {
        return m_visible && chronology() && chronology()->includesTime(t);
    }

    void setVisible(bool visible);

    std::string name() const
    {
        return m_name;
    }

    void setName(const std::string& name);

    typedef std::map<std::string, counted_ptr<Visualizer> > VisualizerTable;
    const VisualizerTable* visualizers() const
    {
        return m_visualizers;
    }

    void setVisualizer(const std::string& tag, Visualizer* visualizer);
    void removeVisualizer(const std::string& tag);
    Visualizer* visualizer(const std::string& tag) const;
    bool hasVisualizers() const;
    void clearVisualizers();

    LightSource* lightSource() const
    {
        return m_lightSource.ptr();
    }

    void setLightSource(LightSource* light)
    {
        m_lightSource = light;
    }

private:
    std::string m_name;
    counted_ptr<Chronology> m_chronology;
    bool m_visible : 1;

    counted_ptr<LightSource> m_lightSource;

    VisualizerTable* m_visualizers;
};

}

#endif // _VESTA_ENTITY_H_
