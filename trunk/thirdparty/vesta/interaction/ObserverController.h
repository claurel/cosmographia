/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_INTERACTION_OBSERVERT_CONTROLLER_H_
#define _VESTA_INTERACTION_OBSERVERT_CONTROLLER_H_

#include "../Observer.h"


namespace vesta
{

class ObserverController : public Object
{
public:
    ObserverController();
    ~ObserverController();

    void tick(double dt);

    Observer* observer() const
    {
        return m_observer.ptr();
    }

    void setObserver(Observer* observer)
    {
        m_observer = observer;
    }

    void applyTorque(const Eigen::Vector3d& torque);
    void roll(double f);
    void pitch(double f);
    void yaw(double f);
    void applyOrbitTorque(const Eigen::Vector3d& torque);
    void dolly(double factor);

    void stop();

    double rotationDampingFactor() const
    {
        return m_rotationDampingFactor;
    }

    void setRotationDampingFactor(double rotationDampingFactor)
    {
        m_rotationDampingFactor = rotationDampingFactor;
    }

private:
    counted_ptr<Observer> m_observer;
    Eigen::Vector3d m_orbitAngularVelocity;
    Eigen::Vector3d m_panAngularVelocity;
    double m_dollyVelocity;
    double m_rotationDampingFactor;
};

}

#endif // _VESTA_INTERACTION_OBSERVERT_CONTROLLER_H_
