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

#include "SkyLabelLayer.h"
#include <vesta/RenderContext.h>

using namespace vesta;
using namespace Eigen;
using namespace std;


SkyLabelLayer::SkyLabelLayer() :
    m_opacity(1.0f),
    m_labelCulling(true)
{
}


SkyLabelLayer::~SkyLabelLayer()
{
}


void
SkyLabelLayer::render(vesta::RenderContext& rc)
{
    rc.setVertexInfo(VertexSpec::Position);

    Material material;
    material.setDiffuse(Spectrum::White());
    rc.bindMaterial(&material);
    glDepthMask(GL_FALSE);

    Quaterniond orientation = Quaterniond::Identity();

    rc.pushModelView();
    rc.rotateModelView(orientation.cast<float>());

    Transform3f mvp = rc.projection() * rc.modelview();

    float fov = rc.pixelSize() * rc.viewportHeight();

    for (vector<SkyLabel>::const_iterator iter = m_labels.begin(); iter != m_labels.end(); ++iter)
    {
        const SkyLabel& label = *iter;

        if (fov < label.minimumFov)
        {
            Vector3f v = mvp * label.position;
            bool cull = v.x() < -1.0f || v.x() > 1.0f || v.y() < -1.0f || v.y() > 1.0f;

            if (!m_labelCulling || !cull)
            {
                Spectrum color(label.color[0], label.color[1], label.color[2]);

                rc.pushModelView();
                rc.translateModelView(label.position);
                rc.drawEncodedText(Vector3f::Zero(), label.text, m_font.ptr(), TextureFont::Utf8, color, m_opacity);
                rc.popModelView();
            }
        }
    }

    rc.popModelView();
}


/** Add a label to this layer.
  *
  * \param labelText a UTF-8 encoded string containing the text of the label
  * \param latitude the celestial latitude (in radians) of the labeled point
  * \param longitude the celestial longitude (in radians) of the labeled point
  */
void
SkyLabelLayer::addLabel(const std::string& labelText, double latitude, double longitude, const Spectrum& color, float minimumFov)
{
    SkyLabel label;
    label.position = Vector3d(cos(latitude) * cos(longitude), cos(latitude) * sin(longitude), sin(latitude)).cast<float>() * 0.99f;
    label.text = labelText;
    label.color[0] = color.red();
    label.color[1] = color.green();
    label.color[2] = color.blue();
    label.minimumFov = minimumFov;

    m_labels.push_back(label);
}
