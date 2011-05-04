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
    for (vector<SkyLabel>::const_iterator iter = m_labels.begin(); iter != m_labels.end(); ++iter)
    {
        const SkyLabel& label = *iter;
        Spectrum color(label.color[0], label.color[1], label.color[2]);

        //std::cerr << label.text << ": " << label.position.transpose() << std::endl;
        rc.drawText(label.position * 5.0f, label.text, m_font.ptr(), color, m_opacity);
    }
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
