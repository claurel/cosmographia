/*
 * $Revision: 475 $ $Date: 2010-08-31 08:09:34 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "Frame.h"
#include "InertialFrame.h"

using namespace vesta;
using namespace Eigen;


/** Get the 6x6 matrix for converting state vectors in this frame to
  * the ICRF at the specified time.
  *
  * The transformation T looks like this:
  * <tt>
  * | R(t)   0    |
  * | dR/dt  R(t) |
  * </tt>
  *
  * Where R(t) is the rotation matrix derived from the quaternion returned
  * by the orientation() method and dR/dt is the derivative of the rotation
  * matrix, given by W* R(t). W* is the angular velocity tensor, a skew
  * symmetric matrix derived from the angular velocity vector W:
  *
  * <tt>
  * |   0  -Wz   Wy |
  * |  Wz    0  -Wx |
  * | -Wy   Wx    0 |
  * </tt>
  *
  * \param tsec time in seconds since J2000 TDB
  */
StateTransform
Frame::stateTransform(double tsec) const
{
    Matrix3d R = orientation(tsec).toRotationMatrix();
    Vector3d W = angularVelocity(tsec);
    Matrix3d Wstar;
    Wstar <<    0.0, -W.z(),  W.y(),
              W.z(),    0.0, -W.x(),
             -W.y(),  W.x(),    0.0;

    StateTransform T = Matrix<double, 6, 6>::Zero();
    T.corner<3, 3>(TopLeft) = R;
    T.corner<3, 3>(BottomRight) = R;
    T.corner<3, 3>(BottomLeft) = Wstar * R;

    return T;
}

/** Get the 6x6 matrix for converting state vectors in the ICRF
  * to this frame.
  *
  * \see Frame::stateTransform()
  *
  * \param tsec time in seconds since J2000 TDB
  */
StateTransform
Frame::inverseStateTransform(double tsec) const
{
    // We're converting states from an inertial frame to the this frame (the target),
    // which may or may not be inertial. We start with the 6x6 state transformation
    // matrix T, which converts states from the target frame to the inertial frame:
    //
    // S_i = T S_t
    //
    // The state vectors contain position and velocity:
    //
    //       | P_i |         | P_t |
    // S_i = | V_i |,  S_t = | V_t |
    //
    // The transformation T looks like this:
    //
    // | R(t)   0    |
    // | dR/dt  R(t) |
    //
    // Where R(t) is time dependent rotation that converts positions from the target
    // frame to the inertial frame. dR/dt is the derivative of the rotation, given
    // by W* R(t). W* is the angular velocity tensor:
    //
    // |   0  -Wz   Wy |
    // |  Wz    0  -Wx |
    // | -Wy   Wx    0 |
    //
    // To compute the inverse of T, we note that:
    //
    // P_i = R(t) P_t
    // V_i = dR/dt P_t + R(t) V_t
    //
    // A simple matrix inversion is adequate to find P_t:
    //
    // P_t = R(t)-1 P_i
    //
    // More effort is required to find V_t:
    //
    // V_t = R(t)-1 (V_i - dR/dt P_t)
    //
    // Substituting P_t gives:
    //
    // V_t = R(t)-1 (V_i - dR/dt R(t)-1 P_i)
    //
    // V_t = R(t)-1 (V_i - W* R(t) R(t)-1 P_i)
    //
    // Canceling terms:
    //
    // V_t = R(t)-1 (V_i - W* P_i)
    //
    // T-1 is thus:
    //
    // |  R(t)-1      0      |
    // | -R(t)-1 W*   R(t)-1 |
    //
    Matrix3d Rinv = orientation(tsec).conjugate().toRotationMatrix();

    // Calculate the additve inverse of the angular velocity tensor
    Vector3d W = -angularVelocity(tsec);
    Matrix3d Wstar;
    Wstar <<    0.0, -W.z(),  W.y(),
              W.z(),    0.0, -W.x(),
             -W.y(),  W.x(),    0.0;

    StateTransform T = Matrix<double, 6, 6>::Zero();

    T.corner<3, 3>(TopLeft) = Rinv;
    T.corner<3, 3>(BottomRight) = Rinv;
    T.corner<3, 3>(BottomLeft) = Rinv * Wstar;

    return T;
}


/** Return the transformation matrix that will convert vectors from
  * on frame to another at the specified time.
  *
  * \param from frame to convert vectors from (use ICRF if null)
  * \param to frame to convert vectors to (use ICRF if null)
  * \param tsec time in seconds since J2000 TDB
  */
StateTransform
Frame::stateTransform(const Frame* from, const Frame* to, double tsec)
{
    // Optimize for the case when one of the frames is the ICRF: skip
    // multiplication by identity.
    if (from == NULL || from == InertialFrame::icrf())
    {
        if (to == NULL)
        {
            return StateTransform::Identity();
        }
        else
        {
            return to->inverseStateTransform(tsec);
        }
    }
    else if (to == NULL || to == InertialFrame::icrf())
    {
        return from->stateTransform(tsec);
    }
    else
    {
        return to->inverseStateTransform(tsec) * from->stateTransform(tsec);
    }
}

