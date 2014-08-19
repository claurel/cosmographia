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

#ifndef _TIME_SWITCHED_GEOMETRY_H_
#define _TIME_SWITCHED_GEOMETRY_H_

#include <vesta/Geometry.h>
#include <vector>

/** TimeSwitchedGeometry contains a time-tagged sequence of geometry objects.
  * Only one geometry object will be shown at a time.
  */
class TimeSwitchedGeometry : public vesta::Geometry
{
public:
    TimeSwitchedGeometry();
    virtual ~TimeSwitchedGeometry();

    void render(vesta::RenderContext& rc, double clock) const;

    /** \reimp */
    float boundingSphereRadius() const
    {
        return m_boundingRadius;
    }

    /** \reimp */
    virtual bool isOpaque() const
    {
        return m_opaque;
    }

    vesta::Geometry* geometry(unsigned int index) const;
    double startTime(unsigned int index) const;
    void addGeometry(double startTime, vesta::Geometry* label);

    vesta::Geometry* activeGeometry(double tdb) const;

private:
    std::vector<double> m_times;
    std::vector<vesta::counted_ptr<vesta::Geometry> > m_geometries;
    float m_boundingRadius;
    bool m_opaque;
};

#endif // _TIME_SWITCHED_GEOMETRY_H_
