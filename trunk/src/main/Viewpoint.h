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

#ifndef _VIEWPOINT_H_
#define _VIEWPOINT_H_

#include <vesta/Object.h>
#include <vesta/Entity.h>
#include <vesta/Observer.h>
#include <string>


class Viewpoint : public vesta::Object
{
public:
    Viewpoint(vesta::Entity* centerBody, double distance);
    ~Viewpoint();

    vesta::Entity* centerBody() const
    {
        return m_centerBody.ptr();
    }

    void setCenterBody(vesta::Entity* centerBody)
    {
        m_centerBody = centerBody;
    }

    vesta::Entity* referenceBody() const
    {
        return m_referenceBody.ptr();
    }

    void setReferenceBody(vesta::Entity* referenceBody)
    {
        m_referenceBody = referenceBody;
    }

    double centerDistance() const
    {
        return m_centerDistance;
    }

    void setCenterDistance(double centerDistance)
    {
        m_centerDistance = centerDistance;
    }

    std::string name() const
    {
        return m_name;
    }

    void setName(const std::string& name)
    {
        m_name = name;
    }

    double azimuth() const
    {
        return m_azimuth;
    }

    void setAzimuth(double azimuth)
    {
        m_azimuth = azimuth;
    }

    double elevation() const
    {
        return m_elevation;
    }

    void setElevation(double elevation)
    {
        m_elevation = elevation;
    }

    void positionObserver(vesta::Observer* observer, double tdbSec);

    enum UpVectorDirection
    {
        CenterNorth,
        CenterSouth,
        EclipticNorth,
        EclipticSouth
    };

    UpVectorDirection upDirection() const
    {
        return m_upDirection;
    }

    void setUpDirection(UpVectorDirection upDirection)
    {
        m_upDirection = upDirection;
    }

private:
    vesta::counted_ptr<vesta::Entity> m_centerBody;
    vesta::counted_ptr<vesta::Entity> m_referenceBody;
    double m_centerDistance;
    double m_azimuth;
    double m_elevation;
    std::string m_name;
    UpVectorDirection m_upDirection;
};

#endif // _VIEWPOINT_H_
