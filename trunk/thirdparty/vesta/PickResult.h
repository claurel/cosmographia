/*
 * $Revision: 421 $ $Date: 2010-08-11 14:35:48 -0700 (Wed, 11 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_PICK_RESULT_H_
#define _VESTA_PICK_RESULT_H_

#include "Entity.h"
#include <Eigen/Core>


namespace vesta
{

class PickResult
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    PickResult() :
        m_distance(0.0),
        m_intersectionPoint(Eigen::Vector3d::Zero())
    {
    }

    PickResult(const PickResult& other) :
        m_entity(other.m_entity),
        m_distance(other.m_distance),
        m_intersectionPoint(other.m_intersectionPoint)
    {
    }

    PickResult& operator=(const PickResult& other)
    {
        m_entity = other.m_entity;
        m_distance = other.m_distance;
        m_intersectionPoint = other.m_intersectionPoint;
        return *this;
    }

    ~PickResult()
    {
    }

    /** Return true if an object was intersected by the pick ray.
      */
    bool hit() const
    {
        return !m_entity.isNull();
    }

    /** Get the object that was hit by the pick geometry. Returns
      * null if no object was hit.
      */
    Entity* hitObject() const
    {
        return m_entity.ptr();
    }

    /** Get the intersection point of the pick ray with the
      * hit object. The return value is undefined if no object was
      * hit.
      */
    Eigen::Vector3d intersectionPoint() const
    {
        return m_intersectionPoint;
    }

    /** Get the distance to the picked object from the origin of the
      * pick ray. The return value is undefined if no object was hit.
      */
    double distance() const
    {
        return m_distance;
    }

    void setHit(Entity* hitObject, double distance, const Eigen::Vector3d& intersectionPoint)
    {
        m_entity = hitObject;
        m_distance = distance;
        m_intersectionPoint = intersectionPoint;
    }

    void setMiss()
    {
        *this = PickResult();
    }

private:
    counted_ptr<Entity> m_entity;
    double m_distance;
    Eigen::Vector3d m_intersectionPoint;
};

}

#endif // _VESTA_PICK_RESULT_H_
