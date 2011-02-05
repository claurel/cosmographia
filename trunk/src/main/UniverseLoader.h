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

#ifndef _UNIVERSE_LOADER_H_
#define _UNIVERSE_LOADER_H_

#include "UniverseCatalog.h"
#include <vesta/Entity.h>
#include <vesta/Trajectory.h>
#include <vesta/TextureMapLoader.h>
#include <vesta/Frame.h>
#include <QVariant>
#include <QStringList>


class UniverseLoader
{
public:
    UniverseLoader();
    ~UniverseLoader();

    QStringList loadSolarSystem(const QVariantMap& contents,
                                UniverseCatalog* catalog);

    void setTextureLoader(vesta::TextureMapLoader* textureLoader);
    void addBuiltinOrbit(const QString& name, vesta::Trajectory* trajectory);
    void removeBuiltinOrbit(const QString& name);

private:
    vesta::Geometry* loadGeometry(const QVariantMap& map);
    vesta::Arc* loadArc(const QVariantMap& map,
                        const UniverseCatalog* catalog);
    vesta::Frame* loadFrame(const QVariantMap& map,
                            const UniverseCatalog* catalog);
    vesta::Trajectory* loadTrajectory(const QVariantMap& map);
    vesta::Trajectory* loadBuiltinTrajectory(const QVariantMap& info);

private:
    QMap<QString, vesta::counted_ptr<vesta::Trajectory> > m_builtinOrbits;
    vesta::counted_ptr<vesta::TextureMapLoader> m_textureLoader;
};

#endif // _UNIVERSE_LOADER_H_
