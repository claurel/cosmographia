/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_BODY_H_
#define _VESTA_BODY_H_

#include "Entity.h"
#include "Geometry.h"

namespace vesta
{

class Body : public Entity
{
public:
    Body();
    ~Body();

    Geometry* geometry() const
    {
        return &*m_geometry;
    }

    void setGeometry(Geometry* geometry);

private:
    counted_ptr<Geometry> m_geometry;
};

}

#endif // _VESTA_BODY_H_
