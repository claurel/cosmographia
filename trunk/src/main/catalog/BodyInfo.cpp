// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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

#include "BodyInfo.h"
#include "vesta/Geometry.h"
#include "vesta/Arc.h"

using namespace vesta;


BodyInfo::BodyInfo() :
    classification(Other),
    labelColor(vesta::Spectrum::White()),
    labelTextVisible(true),
    labelFadeSize(0.0),
    trajectoryPlotDuration(0.0),
    trajectoryPlotSamples(100),
    trajectoryPlotColor(vesta::Spectrum::White()),
    trajectoryPlotLead(0.0),
    trajectoryPlotFade(0.0),
    massKg(0.0),
    density(0.0f)
{
}


struct ClassificationName
{
    BodyInfo::Classification classification;
    const char* name;
};


static ClassificationName classificationNames[] =
{
    { BodyInfo::Planet, "planet" },
    { BodyInfo::DwarfPlanet, "dwarf planet" },
    { BodyInfo::Satellite, "satellite" },
    { BodyInfo::Spacecraft, "spacecraft" },
    { BodyInfo::Asteroid, "asteroid" },
    { BodyInfo::ReferencePoint, "reference point" },
    { BodyInfo::Star, "star" },
    { BodyInfo::Other, "other" }
};


BodyInfo::Classification
BodyInfo::parseClassification(const QString& classificationName)
{
    for (unsigned int i = 0; i < sizeof(classificationNames) / sizeof(classificationNames[0]); ++i)
    {
        if (classificationName == classificationNames[i].name)
        {
            return classificationNames[i].classification;
        }
    }
    return BodyInfo::Other;
}


BodyInfo::Classification
BodyInfo::guessClassification(const vesta::Entity* body)
{
    Geometry* geometry = body->geometry();
    if (geometry == NULL)
    {
        return BodyInfo::ReferencePoint;
    }

    float radius = geometry->boundingSphereRadius();
    if (radius < 1.0f)
    {
        return BodyInfo::Spacecraft;
    }

    // Special case for the sun
    if (body->name() == "Sun")
    {
        return BodyInfo::Star;
    }

    Entity* center = body->chronology()->firstArc()->center();
    if (center == NULL || center->name() == "Sun")
    {
        if (radius > 1500.0f)
        {
            return BodyInfo::Planet;
        }
        else if (radius > 400.0f)
        {
            return BodyInfo::DwarfPlanet;
        }
        else
        {
            return BodyInfo::Asteroid;
        }
    }
    else
    {
        return BodyInfo::Satellite;
    }
}
