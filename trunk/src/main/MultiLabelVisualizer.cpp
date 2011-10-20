// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
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

#include "MultiLabelVisualizer.h"
#include <vesta/RenderContext.h>
#include <algorithm>

using namespace vesta;


MultiLabelVisualizer::MultiLabelVisualizer() :
    Visualizer(new MultiLabelGeometry())
{
    m_geometry = dynamic_cast<MultiLabelGeometry*>(geometry());
}


MultiLabelVisualizer::~MultiLabelVisualizer()
{
}


/** \reimpl
  */
bool
MultiLabelVisualizer::handleRayPick(const PickContext* pc, const Eigen::Vector3d& pickOrigin, double t) const
{
    LabelVisualizer* label = activeLabel(t);
    if (label)
    {
        return label->rayPick(pc, pickOrigin, t);
    }
    else
    {
        return false;
    }
}


LabelVisualizer*
MultiLabelVisualizer::label(unsigned int index) const
{
    if (index < m_labels.size())
    {
        return m_labels[index].ptr();
    }
    else
    {
        return NULL;
    }
}


double
MultiLabelVisualizer::startTime(unsigned int index) const
{
    if (index < m_times.size())
    {
        return m_times[index];
    }
    else
    {
        return 0.0;
    }
}


/** Return the label that should be visible at the specified time.
  */
LabelVisualizer*
MultiLabelVisualizer::activeLabel(double tdb) const
{
    for (unsigned int i = 1; i < m_labels.size(); ++i)
    {
        if (tdb >= m_times[i - 1] && tdb < m_times[i])
        {
            return m_labels[i - 1].ptr();
        }
    }

    if (!m_labels.empty())
    {
        return m_labels.back().ptr();
    }
    else
    {
        return NULL;
    }
}


void
MultiLabelVisualizer::addLabel(double startTime, LabelVisualizer* label)
{
    // Note that we store references to the labels both as a list of visualizers
    // and as a list of geometry. This is necessary because the rendering of a visualizer
    // is completely handled by its geometry, while the picking is handdled in the
    // Visualizer subclass.
    m_labels.push_back(counted_ptr<LabelVisualizer>(label));
    m_times.push_back(startTime);

    m_geometry->addLabel(startTime, label->label());
}
