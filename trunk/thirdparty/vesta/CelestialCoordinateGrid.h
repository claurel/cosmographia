/*
 * $Revision: 408 $ $Date: 2010-08-03 14:38:16 -0700 (Tue, 03 Aug 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_CELESTIAL_COORDINATE_GRID_H_
#define _VESTA_CELESTIAL_COORDINATE_GRID_H_

#include "SkyLayer.h"
#include "Spectrum.h"
#include <Eigen/Geometry>


namespace vesta
{

class CelestialCoordinateGrid : public SkyLayer
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    enum GridFrame
    {
        InertialFrame   = 0,
        HorizontalFrame = 1,
    };

    enum LongitudeUnits
    {
        Degrees   = 0,
        Hours     = 1,
    };

    enum GridStyle
    {
        LabeledGrid   = 0,
        UnlabeledGrid = 1,
        EquatorOnly   = 2,
    };

    CelestialCoordinateGrid();
    ~CelestialCoordinateGrid();


    GridFrame frame() const
    {
        return m_frame;
    }

    void setFrame(GridFrame frame)
    {
        m_frame = frame;
    }

    /** Get the orientation of the grid within its frame.
      */
    Eigen::Quaterniond orientation() const
    {
        return m_orientation;
    }

    /** Set the orientation of the grid within its frame.
      */
    void setOrientation(const Eigen::Quaterniond& orientation)
    {
        m_orientation = orientation;
    }

    /** Get the units of longitude for this grid (either hours or degrees)
      */
    LongitudeUnits longitudeUnits() const
    {
        return m_longitudeUnits;
    }

    /** Set the units of longitude for this grid (which will be either
      * hours or degrees.)
      */
    void setLongitudeUnits(LongitudeUnits units)
    {
        m_longitudeUnits = units;
    }

    Spectrum color() const
    {
        return m_color;
    }

    void setColor(const Spectrum& color)
    {
        m_color = color;
    }

    GridStyle gridStyle() const
    {
        return m_style;
    }

    void setGridStyle(GridStyle style)
    {
        m_style = style;
    }

    virtual void render(RenderContext& rc);

private:
    GridFrame m_frame;
    Eigen::Quaterniond m_orientation;
    LongitudeUnits m_longitudeUnits;
    Spectrum m_color;
    GridStyle m_style;
};

}
#endif // _VESTA_CELESTIAL_COORDINATE_GRID_H_
