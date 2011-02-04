/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "AxesVisualizer.h"
#include "ArrowGeometry.h"
#include "Arc.h"
#include "Frame.h"
#include <cassert>

using namespace vesta;
using namespace Eigen;


AxesVisualizer::AxesVisualizer(AxesType axesType, double size) :
    Visualizer(NULL),
    m_axesType(axesType)
{
    ArrowGeometry* geometry = new ArrowGeometry(0.9f, 0.01f, 0.1f, 0.02f);
    geometry->setScale(size);
    geometry->setVisibleArrows(ArrowGeometry::AllAxes);
    setGeometry(geometry);
}


AxesVisualizer::~AxesVisualizer()
{
}


Eigen::Quaterniond
AxesVisualizer::orientation(const Entity* parent, double t) const
{
    switch (m_axesType)
    {
    case BodyAxes:
        return parent->orientation(t);

    case FrameAxes:
        {
            Arc* arc = parent->chronology()->activeArc(t);
            if (arc)
            {
                return arc->bodyFrame()->orientation(t);
            }
        }

    default:
        return Quaterniond::Identity();
    }
}


ArrowGeometry*
AxesVisualizer::arrows()
{
    return dynamic_cast<ArrowGeometry*>(geometry());
}


