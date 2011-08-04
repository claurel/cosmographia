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

#ifndef _UNIVERSE_CATALOG_H_
#define _UNIVERSE_CATALOG_H_

#include <vesta/Entity.h>
#include <vesta/Spectrum.h>
#include <QString>
#include <QStringList>
#include <QMap>

class Viewpoint;

class BodyInfo : public vesta::Object
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    BodyInfo() :
        classification(Other),
        labelColor(vesta::Spectrum::White()),
        labelFadeSize(0.0),
        trajectoryPlotDuration(0.0),
        trajectoryPlotSamples(100),
        trajectoryPlotColor(vesta::Spectrum::White()),
        trajectoryPlotLead(0.0),
        trajectoryPlotFade(0.0)
    {
    }

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

    Classification classification;
    vesta::Spectrum labelColor;
    double labelFadeSize;
    double trajectoryPlotDuration;
    unsigned int trajectoryPlotSamples;
    vesta::Spectrum trajectoryPlotColor;
    double trajectoryPlotLead;
    double trajectoryPlotFade;
    QString description;
    QString infoSource;
};


class UniverseCatalog
{
public:
    UniverseCatalog();
    ~UniverseCatalog();

    void removeBody(const QString& name);
    void addBody(const QString& name, vesta::Entity* body, BodyInfo* info = NULL);
    void setBodyInfo(const QString& name, BodyInfo* info);
    vesta::Entity* find(const QString& name, Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive) const;
    BodyInfo* findInfo(const QString& name) const;
    BodyInfo* findInfo(const vesta::Entity* body) const;
    bool contains(const QString& name) const;

    QStringList names() const;
    QStringList matchingNames(const QString& pattern) const;

    Viewpoint* findViewpoint(const QString& name);
    void addViewpoint(const QString& name, Viewpoint* viewpoint);
    void removeViewpoint(const QString& name);
    QStringList viewpointNames() const;

    QString getDescription(vesta::Entity* body);

private:
    QMap<QString, vesta::counted_ptr<vesta::Entity> > m_bodies;
    QMap<QString, vesta::counted_ptr<BodyInfo> > m_info;
    QMap<QString, vesta::counted_ptr<Viewpoint> > m_viewpoints;
};

#endif // _UNIVERSE_CATALOG_H_
