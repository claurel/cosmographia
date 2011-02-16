/*
 * $Revision: 559 $ $Date: 2010-12-13 06:33:07 -0800 (Mon, 13 Dec 2010) $
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

    /** Enables/Disables the drawing of labels for an arrow specified by which
      */
    void setLabelEnabled(bool state, unsigned int which);

    /** Sets the text of the label for an arrow specified by which
      */
    void setLabelText(std::string text, unsigned int which);

private:
    AxesType m_axesType;
};

}

#endif // _VESTA_AXES_VISUALIZER_H_

