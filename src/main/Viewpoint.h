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
