// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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
