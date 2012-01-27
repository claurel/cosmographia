/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_LOCAL_VISUALIZER_H_
#define _VESTA_LOCAL_VISUALIZER_H_

#include "Visualizer.h"


namespace vesta
{

/** LocalVisualizer is a visualizer with geometry defined in the local coordinate system
  * of whatever object it is attached to.
  */
class LocalVisualizer : public Visualizer
{
public:
    LocalVisualizer(Geometry* geometry);
    virtual ~LocalVisualizer();

    virtual Eigen::Quaterniond orientation(const Entity* parent, double t) const;
};

}

#endif // _VESTA_LOCAL_VISUALIZER_H_
