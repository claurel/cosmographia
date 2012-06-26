/*
 * $Revision: 678 $ $Date: 2012-05-22 17:59:22 -0700 (Tue, 22 May 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_GEOMETRY_H_
#define _VESTA_GEOMETRY_H_

#include "Object.h"
#include "PickResult.h"
#include "AlignedEllipsoid.h"


namespace vesta
{

class RenderContext;

/** A Geometry object is the visual representation of an entity in vesta.
  * The base class is abstract; derived classes must implement the
  * render and boundingSphereRadius methods.
  */
class Geometry : public Object
{
public:
    /** NearClippingPolicy specifies how the renderer should behave when
      * an object spans a depth range large enough that there is a choice
      * between clipping the object to near plane or possibly having
      * inadequate depth precision for distant parts of the object.
      */
    enum NearClippingPolicy
    {
        /** Clip the object to the near plane. This is default value and the
          * appropriate one for ordinary geometry.
          */
        PreserveDepthPrecision,

        /** Sacrifice depth buffer precision rather than clip. This setting
          * is used for planets, where the depth buffer precision artifacts
          * won't be visible because distant parts are hidden by the
          * horizon.
          */
        PreventClipping,

        /** Prevent clipping while preserving depth buffer precision by rendering
          * the object multiple times with different near/far planes. Multiple
          * rendering passes incur a performance cost. Additionally, some slight
          * rasterization gaps or overlaps may be visible at the split planes.
          * This setting is appropriate for some visualizers where it is critical
          * to prevent clipping when the camera is close even though the geometry
          * has a very large spatial extent (e.g. trajectory plots.)
          */
        SplitToPreventClipping,
        
        /** Object is effectively a point, with no spatial extent. The renderer will
         *  make sure that it is not placed exactly on the near or far plane of
         *  a frustum, where it is likely to be clipped by the GPU. This is the
         *  appropriate policy for labeled points and some billboards.
         */
        ZeroExtent,
    };

    Geometry() :
        m_fixedApparentSize(false),
        m_shadowCaster(false),
        m_shadowReceiver(false),
        m_clippingPolicy(PreserveDepthPrecision)
    {
    }

    virtual ~Geometry() {}

    /** Render this geometry object.
      * @param rc a valid render context
      * @param clock is a time in seconds which can be used for time-driven animations
      */
    virtual void render(RenderContext& rc,
                        double clock) const = 0;

    /** The renderShadow method is called when geometry is being drawn into
      * a shadow map. By default, it just calls the regular render method. This
      * is usually ok, but subclasses may wish to override the method with an
      * optimized renderer that ignores irrelevant material state. Since shadow
      * maps don't have an alpha channel, subclasses may also want to implement
      * a strategy for drawing transparent portions.
      *
      * @param rc a valid render context
      * @param clock is a time in seconds which can be used for time-driven animations
      */
    virtual void renderShadow(RenderContext& rc,
                              double clock) const
    {
        render(rc, clock);
    }


    /** Get the radius of an origin-centered sphere large enough to contain
      * the geometry. Subclasses must implement this method.
      */
    virtual float boundingSphereRadius() const = 0;

    /** Returns true if there are no translucent portions of the geometry.
      * Geometry is treated as opaque by default. Subclasses that draw
      * translucent geometry should override this method and report true.
      */
    virtual bool isOpaque() const { return true; }

    /** Returns true if this geometry can be well approximated by an
      * ellipsoid. This affects shadow rendering: light occlusion is computed
      * analytically for ellipsoidal objects instead of by rendering the
      * geometry into a shadow buffer. The default implementation of
      * isEllipsoidal returns false.
      */
    virtual bool isEllipsoidal() const { return false; }

    /** Get the ellipsoid that approximates the shape of this geometry.
      * The result is meaningful only for geometry that reports true
      * for the isEllipsoidal() method.
      */
    virtual AlignedEllipsoid ellipsoid() const
    {
        return AlignedEllipsoid(Eigen::Vector3d::Zero());
    }

    /** Return whether this geometry is splittable. See setSplittable() for
      * an explanation.
      */
    NearClippingPolicy clippingPolicy() const
    {
        return m_clippingPolicy;
    }

    /** Compute the near plane distance given the camera position in local
      * coordinates. This will be further modified by the near clipping
      * policy. The default implementation returns the distance of the
      * camera minus the bounding sphere radius.
      */
    virtual float nearPlaneDistance(const Eigen::Vector3f& cameraPosition) const
    {
        return cameraPosition.norm() - boundingSphereRadius();
    }

    /** Returns true if this geometry occupies a fixed size on screen (i.e.
      * its apparent size isn't proportional to distance from the viewer.)
      * Ordinary geometry doesn't have a fixed screen size, but markers such
      * as labels do. The default implementation returns false;
      */
    bool hasFixedApparentSize() const
    {
        return m_fixedApparentSize;
    }

    /** Get the apparent size of geometry in pixels. This value is only
      * meaningful for geometry that has a fixed apparent size.
      * \see hasFixedApparentSize
      */
    virtual float apparentSize() const
    {
        return 1.0f;
    }

    /** Return true if this geometry casts shadows.
      */
    bool isShadowCaster() const
    {
        return m_shadowCaster;
    }

    /** Set whether this geometry should cast shadows onto other objects.
      */
    void setShadowCaster(bool castsShadows)
    {
        m_shadowCaster = castsShadows;
    }

    /** Return true if shadows can be cast onto this
      * geometry.
      */
    bool isShadowReceiver() const
    {
        return m_shadowReceiver;
    }

    /** Set whether shadows are visible on this geometry.
      */
    void setShadowReceiver(bool receivesShadows)
    {
        m_shadowReceiver = receivesShadows;
    }

    bool rayPick(const Eigen::Vector3d& pickOrigin,
                 const Eigen::Vector3d& pickDirection,
                 double clock,
                 double* distance) const;

protected:
    /** handleRayPick is called to test whether some geometry is intersected
      * by a pick ray. It is only called when it is determined that the geometry's
      * bounding sphere will be hit by the pick ray. It should be overridden
      * by any pickable geometry. The default implementation always returns
      * false, meaning that the geometry is not pickable.
      */
    virtual bool handleRayPick(const Eigen::Vector3d& /* pickOrigin */,
                               const Eigen::Vector3d& /* pickDirection */,
                               double /* clock */,
                               double* /* distance */) const
    {
        return false;
    }

    /** Sets whether this geometry has a fixed apparent size. By default, this attribute
      * is false. Subclasses should set it to true if they implement things such as labels
      * that don't change in size when the distance to the viewer changes.
      */
    void setFixedApparentSize(bool hasFixedSize)
    {
        m_fixedApparentSize = hasFixedSize;
    }

    void setClippingPolicy(NearClippingPolicy clippingPolicy)
    {
        m_clippingPolicy = clippingPolicy;
    }

private:
    bool m_fixedApparentSize : 1;
    bool m_shadowCaster : 1;
    bool m_shadowReceiver : 1;
    NearClippingPolicy m_clippingPolicy;
};

}

#endif // _VESTA_GEOMETRY_H_
