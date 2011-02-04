/*
 * $Revision: 296 $ $Date: 2010-06-16 16:28:49 -0700 (Wed, 16 Jun 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_VIEWPORT_H_
#define _VESTA_VIEWPORT_H_

namespace vesta
{

/** A viewport is the rectangular region of a drawing surface used when
  * during rendering.
  */
class Viewport
{
public:
    /** Create a new viewport with the specified dimensions and an origin at (0, 0)
      */
    Viewport(unsigned int width, unsigned int height) :
        m_x(0),
        m_y(0),
        m_width(width),
        m_height(height)
    {
    }

    /** Create a new viewport with the specified origin and dimensions.
      */
    Viewport(int x, int y, unsigned int width, unsigned int height) :
        m_x(x),
        m_y(y),
        m_width(width),
        m_height(height)
    {
    }

    int x() const
    {
        return m_x;
    }

    void setX(int x)
    {
        m_x = x;
    }

    int y() const
    {
        return m_y;
    }

    void setY(int y)
    {
        m_y = y;
    }

    void setOrigin(int x, int y)
    {
        m_x = x;
        m_y = y;
    }

    unsigned int width() const
    {
        return m_width;
    }

    void setWidth(unsigned int width)
    {
        m_width = width;
    }

    unsigned int height() const
    {
        return m_height;
    }

    void setHeight(unsigned int height)
    {
        m_height = height;
    }

    void setSize(unsigned int width, unsigned int height)
    {
        m_width = width;
        m_height = height;
    }

    /** Get the ratio of width to height.
      */
    float aspectRatio() const
    {
        return float(m_width) / float(m_height);
    }

private:
    int m_x;
    int m_y;
    unsigned int m_width;
    unsigned int m_height;
};

}

#endif // _VESTA_VIEWPORT_H_

