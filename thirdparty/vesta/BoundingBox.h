/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_BOUNDING_BOX_H_
#define _VESTA_BOUNDING_BOX_H_

#include <Eigen/Core>


namespace vesta
{

class BoundingBox
{
    public:
        BoundingBox() :
            m_minPoint(Eigen::Vector3f::Zero()),
            m_maxPoint(Eigen::Vector3f::Zero())
        {
        }

        BoundingBox(const Eigen::Vector3f& extents) :
            m_minPoint(extents * -0.5f),
            m_maxPoint(extents *  0.5f)
        {
        }

        BoundingBox(const Eigen::Vector3f& minPoint, const Eigen::Vector3f& maxPoint) :
            m_minPoint(minPoint),
            m_maxPoint(maxPoint)
        {
        }

        Eigen::Vector3f extents() const
        {
            return m_maxPoint - m_minPoint;
        }

        Eigen::Vector3f minPoint() const
        {
            return m_minPoint;
        }

        Eigen::Vector3f maxPoint() const
        {
            return m_maxPoint;
        }

        bool containsPoint(const Eigen::Vector3f& point)
        {
            return (point.cwise() > m_minPoint).all() && (point.cwise() < m_maxPoint).all();
        }

        BoundingBox merged(const BoundingBox& other) const
        {
            return BoundingBox(m_minPoint.cwise().min(other.m_minPoint),
                               m_maxPoint.cwise().max(other.m_maxPoint));
        }

        void include(const Eigen::Vector3f& point)
        {
            m_minPoint = m_minPoint.cwise().min(point);
            m_maxPoint = m_maxPoint.cwise().max(point);
        }

    private:
        Eigen::Vector3f m_minPoint;
        Eigen::Vector3f m_maxPoint;
};

}

#endif // _VESTA_BOUNDING_BOX_H_
