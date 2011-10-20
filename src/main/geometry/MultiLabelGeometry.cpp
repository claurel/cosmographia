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

#include "MultiLabelGeometry.h"
#include <vesta/RenderContext.h>
#include <algorithm>

using namespace vesta;


MultiLabelGeometry::MultiLabelGeometry() :
    m_lastRenderedTime(0.0),
    m_lastRenderedOpacity(0.0f)
{
    setFixedApparentSize(true);
}


MultiLabelGeometry::~MultiLabelGeometry()
{
}


/** \reimpl
  */
void
MultiLabelGeometry::render(RenderContext& rc, double clock) const
{
    LabelGeometry* label = activeLabel(clock);
    if (label)
    {
        label->render(rc, clock);

        // Save the last computed opacity. This is part of the workaround to prevent
        // picking of faded out labels. A better solution would require the handleRayPick
        // method of Visualizer to accept a time parameter.
        m_lastRenderedOpacity = 1.0;
        if (label->fadeRange())
        {
            float cameraDistance = rc.modelview().translation().norm();
            float pixelSize = label->fadeSize() / (rc.pixelSize() * cameraDistance);
            m_lastRenderedOpacity = label->fadeRange()->opacity(pixelSize);
        }
    }
    else
    {
        m_lastRenderedOpacity = 0.0;
    }

    // This is a workaround for the fact that the current time is not available
    // in Visualizer::handleRayPick. We cache the rendered time and will use that the next time
    // ray pick is called. It's not completely reliable, but effective enough for
    // the usage pattern in Cosmographia.
    m_lastRenderedTime = clock;
}


/** \reimpl
  */
float
MultiLabelGeometry::boundingSphereRadius() const
{
    return 0.1f;
}


float
MultiLabelGeometry::apparentSize() const
{
    float s = 0.0f;
    for (unsigned int i = 0; i < m_labels.size(); ++i)
    {
        s = std::max(s, m_labels[i]->apparentSize());
    }

    return s;
}


LabelGeometry*
MultiLabelGeometry::label(unsigned int index) const
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
MultiLabelGeometry::startTime(unsigned int index) const
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
LabelGeometry*
MultiLabelGeometry::activeLabel(double tdb) const
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
MultiLabelGeometry::addLabel(double startTime, LabelGeometry* label)
{
    m_labels.push_back(counted_ptr<LabelGeometry>(label));
    m_times.push_back(startTime);
}
