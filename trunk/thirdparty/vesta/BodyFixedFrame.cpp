/*
 * $Revision: 475 $ $Date: 2010-08-31 08:09:34 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "BodyFixedFrame.h"

using namespace vesta;
using namespace Eigen;


/** Construct a new frame fixed with respect to the orientation
  * of the specified body.
  */
BodyFixedFrame::BodyFixedFrame(Entity* body) :
    m_body(body)
{
}


BodyFixedFrame::~BodyFixedFrame()
{
}


/** Get the orientation of the body-fixed frame at time tdbSec.
  */
Quaterniond
BodyFixedFrame::orientation(double tdbSec) const
{
    return m_body->orientation(tdbSec);
}


/** Get the angular velocity of the body-fixed frame at time tdbSec.
  */
Vector3d
BodyFixedFrame::angularVelocity(double tdbSec) const
{
    return m_body->angularVelocity(tdbSec);
}


/** Get the body to which this frame is fixed.
  */
Entity*
BodyFixedFrame::body() const
{
    return m_body.ptr();
}
