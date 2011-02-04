/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "ObserverController.h"
#include "../WorldGeometry.h"
#include "../Debug.h"
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;

ObserverController::ObserverController() :
    m_orbitAngularVelocity(Vector3d::Zero()),
    m_panAngularVelocity(Vector3d::Zero()),
    m_dollyVelocity(1.0),
    m_rotationDampingFactor(5.0)
{
}


ObserverController::~ObserverController()
{
}


/** Update the position and orientation of the observer.
  *
  * \param dt the amount of real time in seconds elapsed since the last tick
  */
void
ObserverController::tick(double dt)
{
    double damping = exp(-dt * m_rotationDampingFactor);
    m_orbitAngularVelocity *= damping;
    m_panAngularVelocity *= damping;
    m_dollyVelocity = pow(m_dollyVelocity, damping);

    double w = m_orbitAngularVelocity.norm();
    if (w > 1.0e-6)
    {
        m_observer->orbit(Quaterniond(AngleAxisd(w * dt, m_orbitAngularVelocity / w)));
    }

    w = m_panAngularVelocity.norm();
    if (w > 1.0e-6)
    {
        m_observer->rotate(Quaterniond(AngleAxisd(w * dt, m_panAngularVelocity / w)));
    }

    if (abs(m_dollyVelocity - 1.0) > 1.0e-6)
    {
        Entity* center = m_observer->center();
        double f = pow(m_dollyVelocity, dt * 1000.0);
        if (center)
        {
            // Special case for world geometry, where the distance is to the surface of the
            // planet, not the center.
            // TODO: It would be a better design to have a method that reports the appropriate
            // distance to use for any geometry.
            WorldGeometry* world = dynamic_cast<WorldGeometry*>(center->geometry());
            if (world)
            {
                double distance = m_observer->position().norm() - world->maxRadius();
                m_observer->setPosition(m_observer->position().normalized() * (world->maxRadius() + distance * f));
            }
            else
            {
                m_observer->changeDistance(f);
            }
        }
    }
}


/** Apply a torque to the observer that causes it to rotate
  * about its center.
  */
void
ObserverController::applyTorque(const Vector3d& torque)
{
    m_panAngularVelocity += torque;
}


/** Apply a 'torque' that causes the observer to rotate about the
  * center object.
  */
void
ObserverController::applyOrbitTorque(const Vector3d& torque)
{
    m_orbitAngularVelocity += torque;
}

/** Rotate the observer about its local x axis (horizontal axis on
  * the screen.
  */
void
ObserverController::pitch(double f)
{
    applyTorque(Vector3d::UnitX() * f);
}


/** Rotate the observer about its local y axis (vertical axis on
  * the screen.
  */
void
ObserverController::yaw(double f)
{
    applyTorque(Vector3d::UnitY() * f);
}


/** Rotate the observer about its local z axis (which points out of
  * the screen back toward the user.)
  */
void
ObserverController::roll(double f)
{
    applyTorque(Vector3d::UnitZ() * f);
}


/** Move the camera along a line between the positions of the observer and
  * the object of interest. The rate of movement varies exponentially with
  * the distance to the object of interest. A factor of less than one moves
  * the observer toward the object of interest, and a factor greater than
  * one moves the observer away.
  */
void
ObserverController::dolly(double factor)
{
    m_dollyVelocity *= factor;
}


/** Stop all translational and rotational motion.
  */
void
ObserverController::stop()
{
    m_dollyVelocity = 1.0;
    m_panAngularVelocity = Vector3d::Zero();
    m_orbitAngularVelocity = Vector3d::Zero();
}
