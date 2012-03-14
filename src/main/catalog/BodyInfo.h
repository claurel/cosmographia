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
