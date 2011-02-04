/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_AXES_VISUALIZER_H_
#define _VESTA_AXES_VISUALIZER_H_

#include "Visualizer.h"
#include "ArrowGeometry.h"

namespace vesta
{
class AxesVisualizer : public Visualizer
{
public:
    enum AxesType
    {
        FrameAxes = 1,
        BodyAxes  = 2,
    };

    AxesVisualizer(AxesType axesType, double size);
    virtual ~AxesVisualizer();

    virtual Eigen::Quaterniond orientation(const Entity* parent, double t) const;

    ArrowGeometry* arrows();

private:
    AxesType m_axesType;
};

}

#endif // _VESTA_AXES_VISUALIZER_H_

