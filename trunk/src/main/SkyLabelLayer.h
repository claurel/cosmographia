// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

#ifndef _SKY_LABEL_LAYER_H_
#define _SKY_LABEL_LAYER_H_

#include <vesta/SkyLayer.h>
#include <vesta/TextureFont.h>
#include <vesta/Spectrum.h>
#include <Eigen/Core>
#include <string>


class SkyLabelLayer : public vesta::SkyLayer
{
public:
    SkyLabelLayer();
    virtual ~SkyLabelLayer();

    /** Draw the sky layer. Subclasses must implement this method.
      */
    virtual void render(vesta::RenderContext& rc);

    void addLabel(const std::string& labelText, double latitude, double longitude, const vesta::Spectrum& color, float minimumFov = 3.141592f);

    vesta::TextureFont* font() const
    {
        return m_font.ptr();
    }

    void setFont(vesta::TextureFont* font)
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

    /** Set to true to omit labels attached to points that lie outside the
      * view.
      */
    void setLabelCulling(bool enable)
    {
        m_labelCulling = enable;
    }

    bool labelCulling() const
    {
        return m_labelCulling;
    }

private:
    struct SkyLabel
    {
        Eigen::Vector3f position;
        std::string text;
        float color[3];
        Eigen::Vector2f offset;
        float minimumFov;
    };

private:
    std::vector<SkyLabel> m_labels;
    vesta::counted_ptr<vesta::TextureFont> m_font;
    float m_opacity;
    bool m_labelCulling;
};

#endif // _SKY_LABEL_LAYER_H_
