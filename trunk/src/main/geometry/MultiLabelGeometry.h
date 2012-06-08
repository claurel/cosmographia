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
