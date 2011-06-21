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

#ifndef _MARKER_LAYER_H_
#define _MARKER_LAYER_H_

#include <vesta/Viewport.h>
#include <vesta/PlanarProjection.h>
#include <vesta/Entity.h>
#include <vesta/Spectrum.h>
#include <QList>


/** Marker layer draws and manages a set of onscreen indicators that are
  * overlaid on the VESTA 3D view.
  */
class MarkerLayer
{
public:
    MarkerLayer();
    ~MarkerLayer();

    void expireMarkers(double realTime);
    void renderMarkers(const Eigen::Vector3d& viewerPosition,
                       const Eigen::Quaterniond& viewerOrientation,
                       const vesta::PlanarProjection& projection,
                       const vesta::Viewport& viewport,
                       double simulationTime,
                       double realTime);

    void addMarker(vesta::Entity* body, const vesta::Spectrum& color, float size, double startTime, double lifetime);

private:
    class Marker
    {
    public:
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        Marker();
        ~Marker();

        vesta::Entity* body() const
        {
            return m_body.ptr();
        }

        void setBody(vesta::Entity* body);

        vesta::Spectrum color() const
        {
            return m_color;
        }

        void setColor(const vesta::Spectrum& color);

        double startTime() const
        {
            return m_startTime;
        }
        void setStartTime(double startTime);

        double lifeTime() const
        {
            return m_lifeTime;
        }
        void setLifeTime(double lifeTime);
        bool isFinished(double realTime)
        {
            return realTime >= m_startTime + m_lifeTime;
        }

        float size() const
        {
            return m_size;
        }

        void setSize(float size)
        {
            m_size = size;
        }

    private:
        vesta::counted_ptr<vesta::Entity> m_body;
        vesta::Spectrum m_color;
        double m_startTime;
        double m_lifeTime;
        float m_size;
    };

private:
    QList<Marker*> m_markers;
};

#endif // _MARKER_LAYER_H_
