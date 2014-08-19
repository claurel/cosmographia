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

#include "MultiLabelGeometry.h"
#include <vesta/RenderContext.h>
#include <algorithm>

using namespace vesta;


MultiLabelGeometry::MultiLabelGeometry()
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
    }
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
