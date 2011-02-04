/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_BODY_DIRECTION_VISUALIZER_H_
#define _VESTA_BODY_DIRECTION_VISUALIZER_H_

#include "ArrowVisualizer.h"


namespace vesta
{
class Entity;

/** The BodyDirection visualizer displays an arrow pointing in the direction
  * of another object.
  */
class BodyDirectionVisualizer : public ArrowVisualizer
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    BodyDirectionVisualizer(double size, Entity* body);
    virtual ~BodyDirectionVisualizer();

    virtual Eigen::Vector3d direction(const Entity* parent, double t) const;

    /** Return the body that the visualizer arrow points toward.
      */
    Entity* body() const
    {
        return m_body.ptr();
    }

    /** Set the body that the visualizer arrow points toward.
      */
    void setBody(Entity* body);

private:
    counted_ptr<Entity> m_body;
};

}

#endif // _VESTA_BODY_DIRECTION_VISUALIZER_H_

