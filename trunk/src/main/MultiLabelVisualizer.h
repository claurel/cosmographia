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

#ifndef _MULTI_LABEL_VISUALIZER_H_
#define _MULTI_LABEL_VISUALIZER_H_

#include "geometry/MultiLabelGeometry.h"
#include <vesta/LabelVisualizer.h>
#include <vector>


/** MultiLabelVisualizer is a container for multiple label visualizer objects. It is used
  * to create a label visualizer that changes over time.
  */
class MultiLabelVisualizer : public vesta::Visualizer
{
public:
    MultiLabelVisualizer();
    ~MultiLabelVisualizer();

    vesta::LabelVisualizer* label(unsigned int index) const;
    double startTime(unsigned int index) const;
    void addLabel(double startTime, vesta::LabelVisualizer* label);

    vesta::LabelVisualizer* activeLabel(double tdb) const;

protected:
    virtual bool handleRayPick(const vesta::PickContext* pc,
                               const Eigen::Vector3d& pickOrigin,
                               double t) const;

private:
    std::vector<double> m_times;
    std::vector<vesta::counted_ptr<vesta::LabelVisualizer> > m_labels;
    vesta::counted_ptr<MultiLabelGeometry> m_geometry;
};

#endif // _MULTI_LABEL_VISUALIZER_H_
