// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
