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

#ifndef _MULTI_LABEL_GEOMETRY_H_
#define _MULTI_LABEL_GEOMETRY_H_

#include <vesta/LabelGeometry.h>
#include <vector>

/** MultiLabelGeometry is a container for multiple label geometry objects. It is used
  * to create a label visualizer that changes over time.
  */
class MultiLabelGeometry : public vesta::Geometry
{
public:
    MultiLabelGeometry();
    virtual ~MultiLabelGeometry();

    void render(vesta::RenderContext& rc, double clock) const;
    float boundingSphereRadius() const;

    virtual bool isOpaque() const
    {
        return false;
    }

    virtual float apparentSize() const;

    unsigned int labelCount() const
    {
        return m_labels.size();
    }

    vesta::LabelGeometry* label(unsigned int index) const;
    double startTime(unsigned int index) const;
    void addLabel(double startTime, vesta::LabelGeometry* label);

    vesta::LabelGeometry* activeLabel(double tdb) const;

private:
    std::vector<double> m_times;
    std::vector<vesta::counted_ptr<vesta::LabelGeometry> > m_labels;
};

#endif // _MULTI_LABEL_GEOMETRY_H_
