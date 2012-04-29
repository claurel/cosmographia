/*
 * $Revision: 671 $ $Date: 2012-04-28 18:42:13 -0700 (Sat, 28 Apr 2012) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_LABEL_GEOMETRY_H_
#define _VESTA_LABEL_GEOMETRY_H_

#include "Geometry.h"
#include "Spectrum.h"
#include "TextureFont.h"
#include "TextureMap.h"
#include "FadeRange.h"
#include <string>


namespace vesta
{

/** LabelGeometry is a geometry type used for single-line screen aligned text
  * and an icon. Both the icon and the label may be omitted.
  */
class LabelGeometry : public Geometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    LabelGeometry(const std::string& text, TextureFont* font, const Spectrum& color = Spectrum(1.0f, 1.0f, 1.0f), float iconSize = 20.0f);
    LabelGeometry();

    virtual ~LabelGeometry();

    // Implementations of abstract methods for Geometry
    void render(RenderContext& rc,
                double clock) const;

    float boundingSphereRadius() const;

    virtual bool isOpaque() const
    {
        return false;
    }

    virtual float apparentSize() const;
    // end geometry methods


    std::string text() const
    {
        return m_text;
    }

    void setText(const std::string& text)
    {
        m_text = text;
    }

    const TextureFont* font() const
    {
        return m_font.ptr();
    }

    void setFont(TextureFont* font)
    {
        m_font = font;
    }

    float opacity() const
    {
        return m_opacity;
    }
    
    void setOpacity(float opacity)
    {
        m_opacity = opacity;
    }
    
    Spectrum color() const
    {
        return m_color;
    }

    void setColor(const Spectrum& color)
    {
        m_color = color;
    }

    /** Get the icon texture map; this method returns null
      * if no icon is set.
      */
    TextureMap* icon() const
    {
        return m_icon.ptr();
    }

    /** Set the icon texture map. Setting it to null means
      * that no icon will be shown.
      */
    void setIcon(TextureMap* icon)
    {
        m_icon = icon;
    }

    /** Get the icon size in pixels. */
    float iconSize() const
    {
        return m_iconSize;
    }

    /** Set the icon size. The default size of the icon is 20 pixels.
      *
      * \param pixels the size of the icon in pixels.
      */
    void setIconSize(float pixels)
    {
        m_iconSize = pixels;
    }

    Spectrum iconColor() const
    {
        return m_iconColor;
    }

    void setIconColor(const Spectrum& color)
    {
        m_iconColor = color;
    }

    /** Get the fade range for this label.
      */
    FadeRange* fadeRange() const
    {
        return m_fadeRange.ptr();
    }

    /** Set the fade range for this label. The fade range how the visibility
      * of the label changes as its apparent size changes. Setting it to null will disable
      * fading.
      */
    void setFadeRange(FadeRange* fadeRange);

    /** Get the size used for calculating visibility. The size has no effect
      * unless a fadeRange is set.
      */
    float fadeSize() const
    {
        return m_fadeSize;
    }

    /** Set the size used for calculating visibility. The size has no effect
      * unless a fadeRange is set. To avoid screen clutter, it is useful to
      * set the fade size of labels for orbiting objects to approximately the size of
      * the orbit. For objects on the surface of a planet, the radius of the object
      * is a good choice for the fade size.
      */
    void setFadeSize(float fadeSize)
    {
        m_fadeSize = fadeSize;
    }

private:
    std::string m_text;
    counted_ptr<TextureFont> m_font;
    counted_ptr<TextureMap> m_icon;
    Spectrum m_color;
    float m_opacity;
    float m_iconSize;
    Spectrum m_iconColor;
    counted_ptr<FadeRange> m_fadeRange;
    float m_fadeSize;
};

}

#endif // _VESTA_LABEL_GEOMETRY_H_

