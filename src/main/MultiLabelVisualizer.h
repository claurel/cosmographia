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
