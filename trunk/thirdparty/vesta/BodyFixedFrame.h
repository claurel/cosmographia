/*
 * $Revision: 475 $ $Date: 2010-08-31 08:09:34 -0700 (Tue, 31 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */
 
#ifndef _VESTA_BODY_FIXED_FRAME_H_
#define _VESTA_BODY_FIXED_FRAME_H_

#include "Frame.h"
#include "Entity.h"


namespace vesta
{

class BodyFixedFrame : public Frame
{
public:
    BodyFixedFrame(Entity* body);
    virtual ~BodyFixedFrame();

    virtual Eigen::Quaterniond orientation(double t) const;
    virtual Eigen::Vector3d angularVelocity(double t) const;

    Entity* body() const;

private:
    counted_ptr<Entity> m_body;
};

}

#endif // _VESTA_BODY_FIXED_FRAME_H_
