/*
 * $Revision: 490 $ $Date: 2010-09-07 13:34:23 -0700 (Tue, 07 Sep 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_INERTIAL_FRAME_H_
#define _VESTA_INERTIAL_FRAME_H_

#include "Frame.h"


namespace vesta
{

class InertialFrame : public Frame
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    InertialFrame(const Eigen::Quaterniond& q) : m_orientation(q) {}
    virtual ~InertialFrame() {}

    /** Return the orientation of the frame. For a inertial frames,
     *  this value is not time dependent.
     */
    virtual Eigen::Quaterniond orientation(double /* t */) const
    {
        return m_orientation;
    }

    /** Return the orientation of the inertial frame.
     */
    Eigen::Quaterniond orientation() const
    {
        return m_orientation;
    }

    Eigen::Vector3d angularVelocity(double /* t */) const
    {
        return Eigen::Vector3d::Zero();
    }

    /** International Celestial Reference Frame, the base reference frame
      * of VESTA.
      */
    static InertialFrame* icrf()
    {
        return s_ICRF.ptr();
    }

    /** Ecliptic frame based on the Earth mean equator J2000 frame.
      */
    static InertialFrame* eclipticJ2000()
    {
        return s_EclipticJ2000.ptr();
    }

    /** Earth mean equator of J2000.0.
      */
    static InertialFrame* equatorJ2000()
    {
        return s_EquatorJ2000.ptr();
    }

    /** Earth mean equator and equinox of B1950 (Besselian year 1950)
      */
    static InertialFrame* equatorB1950()
    {
        return s_EquatorB1950.ptr();
    }

    /** Galactic System 2. The xy-plane is aligned with the galactic equator.
      */
    static InertialFrame* galactic()
    {
        return s_Galactic.ptr();
    }

    static counted_ptr<InertialFrame> s_ICRF;
    static counted_ptr<InertialFrame> s_EclipticJ2000;
    static counted_ptr<InertialFrame> s_EquatorJ2000;
    static counted_ptr<InertialFrame> s_EquatorB1950;
    static counted_ptr<InertialFrame> s_Galactic;

private:
    Eigen::Quaterniond m_orientation;
};

}

#endif // _VESTA_INERTIAL_FRAME_H_
