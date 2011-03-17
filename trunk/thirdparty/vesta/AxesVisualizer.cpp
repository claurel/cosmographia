/*
 * $Revision: 575 $ $Date: 2011-03-16 16:39:49 -0700 (Wed, 16 Mar 2011) $
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

/** Enables/Disables the drawing of labels for an arrow specified by which
  */
void AxesVisualizer::setLabelEnabled(bool state, unsigned int which)
{
    ArrowGeometry* arrow = dynamic_cast<ArrowGeometry*>( geometry() );
    arrow->setLabelEnabled(state, which);
}

/** Sets the text of the label for an arrow specified by which
  */
void AxesVisualizer::setLabelText(std::string text, unsigned int which)
{
    ArrowGeometry* arrow = dynamic_cast<ArrowGeometry*>( geometry() );
    arrow->setLabelText(text, which);
}


/** Get the font used for the label text.
  */
TextureFont*
AxesVisualizer::labelFont() const
{
    ArrowGeometry* arrow = dynamic_cast<ArrowGeometry*>( geometry() );
    if (arrow)
    {
        return arrow->labelFont();
    }
    else
    {
        return NULL;
    }
}


/** Sets a font for the label text.
  */
void
AxesVisualizer::setLabelFont(TextureFont* font)
{
    ArrowGeometry* arrow = dynamic_cast<ArrowGeometry*>( geometry() );
    if (arrow)
    {
        arrow->setLabelFont(font);
    }
}

