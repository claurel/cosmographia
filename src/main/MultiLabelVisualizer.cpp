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
