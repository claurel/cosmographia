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
    m_opacity(1.0f)
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

#if 0
    glPointSize(10.0f);
    glBegin(GL_POINTS);
    glVertex3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -1.0f);
    glEnd();
    glPointSize(1.0f);
#endif

    for (vector<SkyLabel>::const_iterator iter = m_labels.begin(); iter != m_labels.end(); ++iter)
    {
        const SkyLabel& label = *iter;

        Spectrum color(label.color[0], label.color[1], label.color[2]);

        rc.pushModelView();
        rc.translateModelView(label.position);
        rc.drawText(Vector3f::Zero(), label.text, m_font.ptr(), color, m_opacity);
        rc.popModelView();
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
SkyLabelLayer::addLabel(const std::string& labelText, double latitude, double longitude, const Spectrum& color)
{
    SkyLabel label;
    label.position = Vector3d(cos(latitude) * cos(longitude), cos(latitude) * sin(longitude), sin(latitude)).cast<float>();
    label.text = labelText;
    label.color[0] = color.red();
    label.color[1] = color.green();
    label.color[2] = color.blue();

    m_labels.push_back(label);
}
