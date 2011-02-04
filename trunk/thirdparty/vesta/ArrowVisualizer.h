/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_ARROW_VISUALIZER_H_
#define _VESTA_ARROW_VISUALIZER_H_

#include "Visualizer.h"
#include "Spectrum.h"


namespace vesta
{

/** The abstract base class for all visualizers which show a single
  * arrow. Subclasses override the direction method to compute the
  * direction of the arrow at a particular instant.
  */
class ArrowVisualizer : public Visualizer
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    ArrowVisualizer(double size);
    virtual ~ArrowVisualizer();

    Eigen::Quaterniond orientation(const Entity* parent, double t) const;

    /** Get the direction that the arrow is pointing at the specified instant.
      * This method is implemented by all ArrowVisualizer subclasses. It must
      * return a normalized vector in the fundamental coordinate system (J2000).
      */
    virtual Eigen::Vector3d direction(const Entity* parent, double t) const = 0;

    Spectrum color() const;
    void setColor(const Spectrum& color);
};

}

#endif // _VESTA_ARROW_VISUALIZER_H_

