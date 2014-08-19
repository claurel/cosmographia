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
