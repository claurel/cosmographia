/*
 * $Revision: 232 $ $Date: 2010-04-07 21:39:40 -0700 (Wed, 07 Apr 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_PLANE_VISUALIZER_H_
#define _VESTA_PLANE_VISUALIZER_H_

#include "Visualizer.h"
#include "PlaneGeometry.h"
#include "Frame.h"

namespace vesta
{

class PlaneVisualizer : public Visualizer
{
public:
    PlaneVisualizer(double size);
    virtual ~PlaneVisualizer();

    virtual Eigen::Quaterniond orientation(const Entity* parent, double t) const;

    PlaneGeometry* plane() const
    {
        return m_planeGeometry.ptr();
    }

    /** Get the frame of the plane. The frame may be null, in which case the body-fixed
      * frame of the parent body is used.
      */
    Frame* frame() const
    {
        return m_frame.ptr();
    }

    /** Set the frame for the plane. If set to null, the frame will be shown in the
      * body-fixed frame of the parent body.
      */
    void setFrame(Frame* frame)
    {
        m_frame = frame;
    }

private:
    counted_ptr<PlaneGeometry> m_planeGeometry;
    counted_ptr<Frame> m_frame;
};

}

#endif // _VESTA_PLANE_VISUALIZER_H_

