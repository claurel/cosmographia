/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_BOUNDING_SPHERE_H_
#define _VESTA_BOUNDING_SPHERE_H_

#include <Eigen/Core>


/** BoundingSphere represents a spherical bounding volume.
  */
template<typename SCALAR> class BoundingSphere
{
public:
    typedef Eigen::Matrix<SCALAR, 3, 1> VectorType;

    /** Construct a null bounding sphere.
      */
    BoundingSphere() :
        m_center(VectorType::Zero()),
        m_radius(SCALAR(-1.0))
    {
    }

    /** Construct a bounding sphere with the specified center and radius.
      */
    BoundingSphere(const VectorType& center, SCALAR radius) :
        m_center(center),
        m_radius(radius)
    {
    }

    /** Get the radius of this bounding sphere.
      */
    SCALAR radius() const
    {
        return m_radius;
    }

    /** Center the center of this bounding sphere.
      */
    VectorType center() const
    {
        return m_center;
    }

    /** Return true if the sphere is empty. Note that a zero-radius sphere is distinct from
      * a null sphere.
      */
    bool isEmpty() const
    {
        // Negative radius indicates an empty bounding volume
        return m_radius < 0.0;
    }

    /** Return true if this bounding sphere completely contains the specified sphere.
      */
    bool contains(const BoundingSphere& other) const
    {
        SCALAR rdiff = m_radius - other.m_radius;
        if (other.m_radius < m_radius)
        {
            return (m_center - other.m_center).squaredNorm() <= rdiff * rdiff;
        }
        else
        {
            return false;
        }
    }

    /** Return true if this bounding sphere intersects another the specified sphere.
      */
    bool intersects(const BoundingSphere& other) const
    {
        SCALAR rsum = m_radius + other.m_radius;
        return (m_center - other.m_center).squaredNorm() <= rsum * rsum;
    }

    /** Set this sphere to be the minimum radius bounding sphere containing it and another
      * sphere.
      */
    void merge(const BoundingSphere& other)
    {
        if (isEmpty())
        {
            *this = other;
        }
        else if (other.isEmpty())
        {
            // No change when merging an empty sphere
        }
        else
        {
            VectorType v = other.m_center - m_center;
            SCALAR centerDistance = v.norm();

            if (centerDistance + other.m_radius <= m_radius)
            {
                // Other sphere is contained in this one, no adjustment required.
            }
            else if (centerDistance + m_radius <= other.m_radius)
            {
                // This sphere completely contained in other
                *this = other;
            }
            else
            {
                m_center = m_center + (SCALAR(0.5) * (other.m_radius + centerDistance - m_radius) / centerDistance) * v;
                m_radius = SCALAR(0.5) * (other.m_radius + centerDistance + m_radius);
            }
        }
    }

private:
    VectorType m_center;
    SCALAR m_radius;
};

#endif // _VESTA_BOUNDING_SPHERE_H_
