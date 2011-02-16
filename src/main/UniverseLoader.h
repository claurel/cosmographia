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
#include <vesta/Frame.h>
#include <vesta/Trajectory.h>
#include <vesta/RotationModel.h>
#include <vesta/TextureMapLoader.h>
#include <vesta/Visualizer.h>
#include <QVariant>
#include <QStringList>


class UniverseLoader
{
public:
    UniverseLoader();
    ~UniverseLoader();

    QStringList loadSolarSystem(const QVariantMap& contents,
                                UniverseCatalog* catalog);

    vesta::TextureMapLoader* textureLoader() const
    {
        return m_textureLoader.ptr();
    }

    void setTextureLoader(vesta::TextureMapLoader* textureLoader);
    void addBuiltinOrbit(const QString& name, vesta::Trajectory* trajectory);
    void removeBuiltinOrbit(const QString& name);

    void setDataSearchPath(const QString& path);
    void setTextureSearchPath(const QString& path);
    void setModelSearchPath(const QString& path);

private:
    vesta::Geometry* loadGeometry(const QVariantMap& map,
                                  const UniverseCatalog* catalog);
    vesta::Geometry* loadGlobeGeometry(const QVariantMap& map);
    vesta::Geometry* loadMeshGeometry(const QVariantMap& map);
    vesta::Geometry* loadSensorGeometry(const QVariantMap& map,
                                        const UniverseCatalog* catalog);

    vesta::Arc* loadArc(const QVariantMap& map,
                        const UniverseCatalog* catalog);
    vesta::Frame* loadFrame(const QVariantMap& map,
                            const UniverseCatalog* catalog);
    vesta::Trajectory* loadTrajectory(const QVariantMap& map);
    vesta::Trajectory* loadBuiltinTrajectory(const QVariantMap& info);
    vesta::Trajectory* loadInterpolatedStatesTrajectory(const QVariantMap& info);
    vesta::Trajectory* loadTleTrajectory(const QVariantMap& info);

    vesta::RotationModel* loadRotationModel(const QVariantMap& info);
    vesta::RotationModel* loadInterpolatedRotationModel(const QVariantMap& info);

    vesta::Visualizer* loadVisualizer(const QVariantMap& info,
                                      const UniverseCatalog* catalog);

private:
    QString dataFileName(const QString& fileName);
    QString textureFileName(const QString& fileName);
    QString modelFileName(const QString& fileName);

private:
    QMap<QString, vesta::counted_ptr<vesta::Trajectory> > m_builtinOrbits;
    vesta::counted_ptr<vesta::TextureMapLoader> m_textureLoader;
    QMap<QString, vesta::counted_ptr<vesta::Geometry> > m_modelCache;
    QString m_dataSearchPath;
    QString m_textureSearchPath;
    QString m_modelSearchPath;
    QString m_currentBodyName;
};

#endif // _UNIVERSE_LOADER_H_
