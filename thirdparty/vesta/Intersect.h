/*
 * $Revision: 280 $ $Date: 2010-05-27 14:00:05 -0700 (Thu, 27 May 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_INTERSECT_H_
#define _VESTA_INTERSECT_H_

#include <Eigen/Core>


namespace vesta
{

/** Calculate the intersection between a ray and a sphere. If there is an intersection,
  * the distance to the nearest intersection point will be stored in the distance parameter.
  *
  * \param rayOrigin origin of the ray
  * \param rayDirection direction of the ray (must be normalized)
  * \param sphereCenter center of the sphere
  * \param sphereRadius radius of the sphere
  * \param distance distance from the origin to the closest intersection point. May be null.
  */
template<typename DERIVED1, typename DERIVED2, typename DERIVED3> bool
TestRaySphereIntersection(const Eigen::MatrixBase<DERIVED1>& rayOrigin,
                          const Eigen::MatrixBase<DERIVED2>& rayDirection,
                          const Eigen::MatrixBase<DERIVED3>& sphereCenter,
                          typename Eigen::ei_traits<DERIVED1>::Scalar sphereRadius,
                          typename Eigen::ei_traits<DERIVED1>::Scalar * distance = 0)

{
    typedef typename Eigen::ei_traits<DERIVED1>::Scalar SCALAR;
    Eigen::Matrix<SCALAR, 3, 1> x = rayOrigin - sphereCenter;
    SCALAR xv = x.dot(rayDirection);
    SCALAR discriminant = xv * xv - x.dot(x) + sphereRadius * sphereRadius;

    if (discriminant > SCALAR(0))
    {
        double d = Eigen::ei_sqrt(discriminant);
        double i1 = -xv - d;
        double i2 = -xv + d;

        if (i1 > SCALAR(0.0))
        {
            if (distance)
            {
                *distance = i1;
            }
            return true;
        }
        else if (i2 > SCALAR(0.0))
        {
            if (distance)
            {
                *distance = i2;
            }
            return true;
        }
        else
        {
            // Intersection is behind the origin
            return false;
        }
    }
    else
    {
        return false;
    }
}


/** Calculate the intersection between a ray and an axis-aligned, origin-centered
*   ellipsoid. If there is an intersection, the distance to the nearest intersection
*   point will be stored in the distance parameter.
*
* \param rayOrigin origin of the ray
* \param rayDirection direction of the ray (must be normalized)
* \param semiAxes semi-axes of the ellipsoid
* \param distance distance from the origin to the closest intersection point. May be null.
*/
template<typename DERIVED1, typename DERIVED2, typename DERIVED3>
bool TestRayEllipsoidIntersection(const Eigen::MatrixBase<DERIVED1>& rayOrigin,
                                  const Eigen::MatrixBase<DERIVED2>& rayDirection,
                                  const Eigen::MatrixBase<DERIVED3>& semiAxes,
                                  double* distance = 0)
{
    typedef typename Eigen::ei_traits<DERIVED1>::Scalar SCALAR;
    Eigen::Matrix<SCALAR, 3, 1> A = semiAxes.cwise().inverse().cwise().square();
    Eigen::Matrix<SCALAR, 3, 1> xx = rayOrigin.cwise().square();
    Eigen::Matrix<SCALAR, 3, 1> xv = rayOrigin.cwise() * rayDirection;
    Eigen::Matrix<SCALAR, 3, 1> vv = rayDirection.cwise().square();
    SCALAR a = vv.dot(A);
    SCALAR b = xv.dot(A);
    SCALAR c = xx.dot(A) - SCALAR(1);
    SCALAR discriminant = b * b - a * c;

    if (discriminant > SCALAR(0))
    {
        double d = Eigen::ei_sqrt(discriminant);
        double i1 = (-b - d) / a;
        double i2 = (-b + d) / a;

        if (i1 > 0.0)
        {
            if (distance)
            {
                *distance = i1;
            }
            return true;
        }
        else if (i2 > 0.0)
        {
            if (distance)
            {
                *distance = i2;
            }
            return true;
        }
        else
        {
            // Intersection is behind the origin
            return false;
        }
    }
    else
    {
        return false;
    }
}

}
#endif // _VESTA_INTERSECT_H_
