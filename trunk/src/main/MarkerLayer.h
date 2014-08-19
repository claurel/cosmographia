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

#ifndef _MARKER_LAYER_H_
#define _MARKER_LAYER_H_

#include <vesta/Viewport.h>
#include <vesta/PlanarProjection.h>
#include <vesta/Entity.h>
#include <vesta/Spectrum.h>
#include <QList>


class Marker
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Marker();
    ~Marker();

    enum Style
    {
        Pulse,
        Spin,
    };

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

    Style style() const
    {
        return m_style;
    }

    void setStyle(Style style)
    {
        m_style = style;
    }

    void setTargetSizeThreshold(float maximumSize);

    float targetSizeThreshold() const
    {
        return m_targetSizeThreshold;
    }

    bool directionIndicatorEnabled() const
    {
        return m_directionIndicatorEnabled;
    }

    void setDirectionIndicatorEnabled(bool enabled);

private:
    vesta::counted_ptr<vesta::Entity> m_body;
    vesta::Spectrum m_color;
    double m_startTime;
    double m_lifeTime;
    float m_size;
    Style m_style;
    float m_targetSizeThreshold;
    bool m_directionIndicatorEnabled;
};


/** Marker layer draws and manages a set of onscreen indicators that are
  * overlaid on the VESTA 3D view.
  */
class MarkerLayer
{
public:
    MarkerLayer();
    ~MarkerLayer();

    void expireMarkers(double realTime);
    void renderMarker(const Eigen::Vector3d& viewerPosition,
                      const Eigen::Quaterniond& viewerOrientation,
                      const vesta::PlanarProjection& projection,
                      const vesta::Viewport& viewport,
                      double simulationTime,
                      double realTime,
                      const Marker* marker);
    void renderMarkers(const Eigen::Vector3d& viewerPosition,
                       const Eigen::Quaterniond& viewerOrientation,
                       const vesta::PlanarProjection& projection,
                       const vesta::Viewport& viewport,
                       double simulationTime,
                       double realTime);

    void addMarker(vesta::Entity* body, const vesta::Spectrum& color, float size, Marker::Style style, double startTime, double lifetime);

    void removeAllMarkersOnBody(vesta::Entity* body);

private:
    QList<Marker*> m_markers;
};

#endif // _MARKER_LAYER_H_
