/*
 * $Revision: 565 $ $Date: 2011-02-15 16:00:43 -0800 (Tue, 15 Feb 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_ECLIPSE_SHADOW_VOLUME_SET_H_
#define _VESTA_ECLIPSE_SHADOW_VOLUME_SET_H_

#include "../Frustum.h"
#include "../Object.h"
#include "../GeneralEllipse.h"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/StdVector>
#include <vector>

namespace vesta
{

class Entity;

// An internal class for keeping track of shadows cast by ellipsoidal bodies
class EclipseShadowVolumeSet : public Object
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    EclipseShadowVolumeSet();
    ~EclipseShadowVolumeSet();

    struct EclipseShadow
    {
        const Entity* occluder;
        Eigen::Vector3d position;
        Eigen::Vector3d direction;
        GeneralEllipse projection;
        float umbraSlope;
        float penumbraSlope;
    };
    typedef std::vector<EclipseShadow, Eigen::aligned_allocator<EclipseShadow> > EclipseShadowVector;

    void clear();
    void addShadow(const Entity* occluder,
                   const Eigen::Vector3d& occluderPosition,
                   const Eigen::Quaternionf& occluderOrientation,
                   const Eigen::Vector3d& lightPosition,
                   double lightRadius);
    bool frustumCull(const Frustum& frustum);
    bool findIntersectingShadows(const Entity* entity, const Eigen::Vector3d& sphereCenter, double sphereRadius);

    const EclipseShadowVector& intersectingShadows() const
    {
        return m_intersectingShadows;
    }

    /** Returns true if the last call to findIntersectingShadows found the test sphere to
      * like completely inside a shadow umbra.
      */
    bool insideUmbra() const
    {
        return m_insideUmbra;
    }

private:
    struct ConicShadowVolume
    {
        const Entity* occluder;
        Eigen::Vector3d apex;
        Eigen::Vector3d center;
        Eigen::Vector3d direction;
        double sphereRadius;
        double front;
        double back;
        double cosAngle;
        double sinAngle;

        double umbraLength;
        double cosUmbraConeAngle;
        double sinUmbraConeAngle;

        Eigen::Quaternionf orientation;
        bool ellipseComputed;
        GeneralEllipse ellipse;
    };

    static bool coneIntersectsSphere(const ConicShadowVolume& cone, const Eigen::Vector3d& center, double r);

private:
    typedef std::vector<ConicShadowVolume, Eigen::aligned_allocator<ConicShadowVolume> > ShadowVolumeVector;

    ShadowVolumeVector m_allShadows;
    std::vector<ConicShadowVolume*> m_frustumShadows;
    EclipseShadowVector m_intersectingShadows;
    bool m_insideUmbra;
};

}

#endif // _VESTA_ECLIPSE_SHADOW_VOLUME_SET_H_
