/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_STATEVECTOR_H_
#define _VESTA_STATEVECTOR_H_

#include <Eigen/Core>

namespace vesta
{

typedef Eigen::Matrix<double, 6, 1> Vector6d;

class StateVector
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    StateVector() :
        m_state(Vector6d::Zero())
    {
    }

    explicit StateVector(const Vector6d& v) :
        m_state(v)
    {
    }

    StateVector(const Eigen::Vector3d& position, const Eigen::Vector3d& velocity)
    {
        m_state.start<3>() = position;
        m_state.end<3>() = velocity;
    }

    Vector6d state() const { return m_state; }

    Eigen::Vector3d position() const
    {
        return m_state.start<3>();
    }

    Eigen::Vector3d velocity() const
    {
        return m_state.end<3>();
    }

    StateVector operator+(const StateVector& other)
    {
        return StateVector(m_state + other.m_state);
    }

    StateVector operator-(const StateVector& other)
    {
        return StateVector(m_state - other.m_state);
    }

private:
    Vector6d m_state;
};

}

#endif // _VESTA_STATEVECTOR_H_
