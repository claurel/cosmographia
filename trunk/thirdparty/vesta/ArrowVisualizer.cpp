/*
 * $Revision: 575 $ $Date: 2011-03-16 16:39:49 -0700 (Wed, 16 Mar 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "ArrowVisualizer.h"
#include "ArrowGeometry.h"
#include <cassert>

using namespace vesta;
using namespace Eigen;


/** Create a new arrow visualizer. The color of the arrow is white by
  * default.
  */
ArrowVisualizer::ArrowVisualizer(double size) :
    Visualizer(NULL)
{
    ArrowGeometry* geometry = new ArrowGeometry(0.9f, 0.01f, 0.1f, 0.02f);
    geometry->setScale(size);
    geometry->setVisibleArrows(ArrowGeometry::XAxis);
    setGeometry(geometry);

    geometry->setArrowColor(0, Spectrum(1.0f, 1.0f, 1.0f));
}


ArrowVisualizer::~ArrowVisualizer()
{
}


Quaterniond
ArrowVisualizer::orientation(const Entity* parent, double t) const
{
    // The subclass computes the direction
    Vector3d targetDirection = direction(parent, t);

    Quaterniond rotation = Quaterniond::Identity();

    // The arrow geometry points in the +x direction, so calculate the rotation
    // required to make the arrow point in target direction
    rotation.setFromTwoVectors(Vector3d::UnitX(), targetDirection);

    return rotation;
}


Spectrum
ArrowVisualizer::color() const
{
    ArrowGeometry* arrow = dynamic_cast<ArrowGeometry*>(geometry());
    assert(arrow != NULL);
    return arrow->arrowColor(0);
}


void
ArrowVisualizer::setColor(const Spectrum& color)
{
    ArrowGeometry* arrow = dynamic_cast<ArrowGeometry*>(geometry());
    assert(arrow != NULL);
    arrow->setArrowColor(0, color);
}

/** Enables/Disables the drawing of a label
  */
void ArrowVisualizer::setLabelEnabled(bool state)
{
    ArrowGeometry* arrow = dynamic_cast<ArrowGeometry*>( geometry() );
    arrow->setLabelEnabled(state, ArrowGeometry::XAxis);
}

/** Sets the text of a label
  */
void ArrowVisualizer::setLabelText(std::string text)
{
    ArrowGeometry* arrow = dynamic_cast<ArrowGeometry*>( geometry() );
    arrow->setLabelText(text, ArrowGeometry::XAxis);
}


/** Get the font used for the label text.
  */
TextureFont* ArrowVisualizer::labelFont() const
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
void ArrowVisualizer::setLabelFont(TextureFont* font)
{
    ArrowGeometry* arrow = dynamic_cast<ArrowGeometry*>( geometry() );
    if (arrow)
    {
        arrow->setLabelFont(font);
    }
}
