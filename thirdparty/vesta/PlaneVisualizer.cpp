/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "PlaneVisualizer.h"
#include "PlaneGeometry.h"

using namespace vesta;
using namespace Eigen;


PlaneVisualizer::PlaneVisualizer(double size) :
    Visualizer(NULL)
{
    m_planeGeometry = new PlaneGeometry();
    m_planeGeometry->setScale(size);
    setGeometry(m_planeGeometry.ptr());
}


PlaneVisualizer::~PlaneVisualizer()
{
}


Eigen::Quaterniond
PlaneVisualizer::orientation(const Entity* parent, double t) const
{
    if (m_frame.isNull())
    {
        return parent->orientation(t);
    }
    else
    {
        return m_frame->orientation(t);
    }
}


