/*
 * $Revision: 530 $ $Date: 2010-10-12 11:26:43 -0700 (Tue, 12 Oct 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "Entity.h"
#include "Chronology.h"
#include "Arc.h"
#include "Frame.h"
#include "Trajectory.h"
#include "RotationModel.h"
#include "Visualizer.h"

using namespace vesta;
using namespace Eigen;
using namespace std;


/** Create a new entity with an empty chronology.
  */
Entity::Entity() :
    m_visible(true),
    m_visualizers(NULL)
{
    m_chronology = new Chronology();
}


Entity::~Entity()
{
    delete m_visualizers;
}


/** Get the position of the entity in universal coordinates.
  * \param t the time in seconds since J2000 TDB
  */
Vector3d
Entity::position(double t) const
{
    Arc* arc = m_chronology->activeArc(t);
    if (arc)
    {
        Vector3d centerPosition = Vector3d::Zero();
        if (arc->center())
            centerPosition = arc->center()->position(t);
        return centerPosition + arc->trajectoryFrame()->orientation(t) * arc->trajectory()->position(t);
    }
    else
    {
        return Vector3d::Zero();
    }
}


/** Get the state vector of the entity in the fundamental coordinate
  * system (J200).
  * \param t the time in seconds since J2000 TDB
  */
StateVector
Entity::state(double t) const
{
    Arc* arc = m_chronology->activeArc(t);
    if (arc)
    {
        StateVector centerState(Vector3d::Zero(), Vector3d::Zero());
        if (arc->center())
        {
            centerState = arc->center()->state(t);
        }

        StateVector state = arc->trajectory()->state(t);

        Matrix3d m = arc->trajectoryFrame()->orientation(t).toRotationMatrix();
        Vector3d omega = arc->trajectoryFrame()->angularVelocity(t);
        Vector3d position = m * state.position();
        Vector3d velocity = m * state.velocity() + omega.cross(state.position());

        return centerState + StateVector(position, velocity);
    }
    else
    {
        return StateVector(Vector3d::Zero(), Vector3d::Zero());
    }
}


/** Get the orientation of the entity in universal coordinates.
  * \param t the time in seconds since J2000 TDB
  */
Quaterniond
Entity::orientation(double t) const
{
    Arc* arc = m_chronology->activeArc(t);
    if (arc)
    {
        return arc->bodyFrame()->orientation(t) * arc->rotationModel()->orientation(t);
    }
    else
    {
        return Quaterniond::Identity();
    }
}


/** Get the angular velocity of the entity in universal coordinates.
  * \param t the time in seconds since J2000 TDB
  */
Vector3d
Entity::angularVelocity(double t) const
{
    Arc* arc = m_chronology->activeArc(t);
    if (arc)
    {
        return arc->bodyFrame()->orientation(t) * arc->rotationModel()->angularVelocity(t);
    }
    else
    {
        return Vector3d::Zero();
    }
}


/** Set whether the body should be visible. Neither geometry nor attached
  * visualizers are shown for bodies with the visible flag set to false.
  * The value of the visible flag is true by default.
  */
void
Entity::setVisible(bool visible)
{
    m_visible = visible;
}


/** Set the name of the body.
  */
void
Entity::setName(const std::string& name)
{
    m_name = name;
}


/** Add a new visualizer with a specified tag. If a visualizer with the
  * same tag already exists, it will be replaced.
  */
void
Entity::setVisualizer(const std::string& tag, Visualizer* visualizer)
{
    if (!m_visualizers)
    {
        m_visualizers = new VisualizerTable;
    }
    (*m_visualizers)[tag] = counted_ptr<Visualizer>(visualizer);
}


/** Remove the visualizer with the specified tag. The method has no
  * effect if the tag is not found.
  */
void
Entity::removeVisualizer(const std::string& tag)
{
    if (m_visualizers)
    {
        m_visualizers->erase(tag);
    }
}


/** Get the visualizer with the specified tag. If no visualizer with
  * the requested tag exists, the method returns null.
  */
Visualizer*
Entity::visualizer(const std::string& tag) const
{
    if (m_visualizers)
    {
        VisualizerTable::iterator iter = m_visualizers->find(tag);
        if (iter == m_visualizers->end())
        {
            return NULL;
        }
        else
        {
            return iter->second.ptr();
        }
    }
    else
    {
        return NULL;
    }
}


/** Returns true if the body has at least one attached visualizer.
  */
bool
Entity::hasVisualizers() const
{
    return m_visualizers && !m_visualizers->empty();
}


/** Remove all attached visualizers.
  */
void
Entity::clearVisualizers()
{
    if (m_visualizers)
    {
        m_visualizers->clear();
        delete m_visualizers;
        m_visualizers = NULL;
    }
}
