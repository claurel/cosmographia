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

#ifndef _CONSTELLATION_INFO_H_
#define _CONSTELLATION_INFO_H_

#include <Eigen/Core>
#include <string>
#include <vector>


class ConstellationInfo
{
public:
    ConstellationInfo(const std::string& name) : m_name(name) {};
    virtual ~ConstellationInfo() {};

    std::string name() const
    {
        return m_name;
    }

    void setName(const std::string& name)
    {
        m_name = name;
    }

    Eigen::Vector2f labelLocation() const
    {
        return m_labelLocation;
    }

    void setLabelLocation(const Eigen::Vector2f& labelLocation)
    {
        m_labelLocation = labelLocation;
    }

    static const std::vector<ConstellationInfo>& constellations();

private:
    std::string m_name;
    Eigen::Vector2f m_labelLocation;

    static std::vector<ConstellationInfo> s_constellations;
};

#endif // _SKY_LABEL_LAYER_H_
