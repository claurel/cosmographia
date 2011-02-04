/*
 * $Revision: 523 $ $Date: 2010-10-07 10:06:21 -0700 (Thu, 07 Oct 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_FADE_RANGE_H_
#define _VESTA_FADE_RANGE_H_

#include "Object.h"
#include <limits>


namespace vesta
{

/** A FadeRange can be attached to certain VESTA objects to control fading from
  * transparent to opaque as the apparent screen size changes. It can be used to
  * prevent labels and other annotations attached to distant objects from
  * cluttering the screen.
  *
  * The opacity changes according to the apparent size in pixels p as follows:
  *
  * p < minPixels : invisible<br>
  * minPixels <= p < minPixels + minFadeExtent : linear transition from transparent to opaque<br>
  * minPixels + minFadeExtent <= p < maxPixels : fully visible<br>
  * maxPixels <= p < maxPixels + maxFadeExtent : linear transition from opaque to transparent<br>
  * p >= maxPixels + maxFadeExtent  : invisible<br>
  *
  * The behavior of FadeRange deliberately resembles that of the Lod element
  * in KML. See http://code.google.com/apis/kml/documentation/kmlreference.html#lod
  */
class FadeRange : public Object
{
public:
    /** Construct a new fade range with extents and limits set so that an object
      * will always remain visible: minPixels = 0, maxPixels = infinity, fade
      * extents both zero.
      */
    FadeRange() :
        m_minPixels(0.0f),
        m_maxPixels(std::numeric_limits<float>::infinity()),
        m_minFadeExtent(0.0f),
        m_maxFadeExtent(0.0f)
    {
    }

    /** Construct a new fade range with the specified limits and extents.
      */
    FadeRange(float minPixels, float maxPixels, float minFadeExtent, float maxFadeExtent) :
        m_minPixels(minPixels),
        m_maxPixels(maxPixels),
        m_minFadeExtent(minFadeExtent),
        m_maxFadeExtent(maxFadeExtent)
    {
    }

    /** Construct a new fade range with the specified minimum limit and extent.
      */
    FadeRange(float minPixels, float minFadeExtent) :
        m_minPixels(minPixels),
        m_maxPixels(std::numeric_limits<float>::infinity()),
        m_minFadeExtent(minFadeExtent),
        m_maxFadeExtent(0.0f)
    {
    }


    /** Get the minimum size at which an object will remain visible.
      */
    float minPixels() const
    {
        return m_minPixels;
    }

    /** Get the maximum size at which an object will remain visible.
      */
    float maxPixels() const
    {
        return m_maxPixels;
    }

    /** Get the range of pixels over which an object will fade from
      * invisible to completely visible.
      */
    float minFadeExtent() const
    {
        return m_minFadeExtent;
    }

    /** Get the range of pixels over which an object will fade from
      * completely to invisible.
      */
    float maxFadeExtent() const
    {
        return m_maxFadeExtent;
    }

    /** Set the minimum size at which an object will remain visible.
      */
    void setMinPixels(float minPixels)
    {
        m_minPixels = minPixels;
    }

    /** Set the maximum size at which an object will remain visible.
      */
    void setMaxPixels(float maxPixels)
    {
        m_maxPixels = maxPixels;
    }

    /** Set the range of pixels over which an object will fade from
      * invisible to completely visible.
      */
    void setMinFadeExtent(float minFadeExtent)
    {
        m_minFadeExtent = minFadeExtent;
    }

    /** Get the range of pixels over which an object will fade from
      * completely to invisible.
      */
    void setMaxFadeExtent(float maxFadeExtent)
    {
        m_maxFadeExtent = maxFadeExtent;
    }

    /** Compute the opacity of an object with the specified pixel size.
      *
      * \return an opacity in the range from 0 (completely transparent) to
      * 1 (opaque.)
      */
    float opacity(float pixelSize)
    {
        float alpha;

        if (pixelSize < m_minPixels)
        {
            alpha = 0.0f;
        }
        else if (pixelSize < m_minPixels + m_minFadeExtent)
        {
            alpha = (pixelSize - m_minPixels) / m_minFadeExtent;
        }
        else if (pixelSize < m_maxPixels - m_maxFadeExtent)
        {
            alpha = 1.0f;
        }
        else if (pixelSize < m_maxPixels)
        {
            alpha = (m_maxPixels - pixelSize) / m_maxFadeExtent;
        }
        else
        {
            alpha = 0.0f;
        }

        return alpha;
    }

private:
    float m_minPixels;
    float m_maxPixels;
    float m_minFadeExtent;
    float m_maxFadeExtent;
};

}

#endif // _VESTA_FRAME_H_
