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

#ifndef _UNIVERSE_BODY_INFO_H_
#define _UNIVERSE_BODY_INFO_H_

#include <vesta/Entity.h>
#include <vesta/Spectrum.h>
#include <QString>


class BodyInfo : public vesta::Object
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    BodyInfo();

    enum Classification
    {
        ReferencePoint,
        Planet,
        Satellite,
        Asteroid,
        DwarfPlanet,
        Spacecraft,
        Star,
        Other
    };

    static Classification parseClassification(const QString& classificationName);
    static Classification guessClassification(const vesta::Entity* body);

public:
    Classification classification;
    vesta::Spectrum labelColor;
    bool labelTextVisible;
    double labelFadeSize;
    double trajectoryPlotDuration;
    unsigned int trajectoryPlotSamples;
    vesta::Spectrum trajectoryPlotColor;
    double trajectoryPlotLead;
    double trajectoryPlotFade;
    QString description;
    QString infoSource;
    double massKg;
    float density;
};

#endif // _UNIVERSE_BODY_INFO_H_
