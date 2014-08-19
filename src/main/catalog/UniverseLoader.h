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

#ifndef _UNIVERSE_LOADER_H_
#define _UNIVERSE_LOADER_H_

#include "UniverseCatalog.h"
#include <vesta/Entity.h>
#include <vesta/Frame.h>
#include <vesta/Trajectory.h>
#include <vesta/RotationModel.h>
#include <vesta/Visualizer.h>
#include <vesta/TextureMapLoader.h>
#include <QVariant>
#include <QStringList>
#include <QTextStream>
#include <QHash>
#include <QSet>


class TleTrajectory;

namespace vesta
{
    class PlanetaryRings;
    class InertialFrame;
}

class Viewpoint;
class TwoVectorFrameDirection;
class PathRelativeTextureLoader;

class CatalogContents
{
public:
    CatalogContents()
    {
    }

    CatalogContents(const QStringList& bodyNames, const QStringList& spiceKernels) :
        m_bodyNames(bodyNames),
        m_spiceKernels(spiceKernels)
    {
    }

    ~CatalogContents() {}

    QStringList bodyNames() const { return m_bodyNames; }
    QStringList spiceKernels() const { return m_spiceKernels; }
    void setBodyNames(const QStringList& bodyNames) { m_bodyNames = bodyNames; }
    void setSpiceKernels(const QStringList& spiceKernels) { m_spiceKernels = spiceKernels; }
    void appendContents(const CatalogContents* contents)
    {
        m_bodyNames << contents->bodyNames();
        m_spiceKernels << contents->spiceKernels();
    }

    void appendBody(const QString& bodyName)
    {
        m_bodyNames << bodyName;
    }

    void appendSpiceKernel(const QString& spiceKernel)
    {
        m_spiceKernels << spiceKernel;
    }

private:
    QStringList m_bodyNames;
    QStringList m_spiceKernels;
};

class UniverseLoader
{
public:
    UniverseLoader();
    ~UniverseLoader();

    CatalogContents* loadCatalogItems(const QVariantMap& contents,
                                      UniverseCatalog* catalog);

    vesta::TextureMapLoader* textureLoader() const;

    void setTextureLoader(PathRelativeTextureLoader* textureLoader);
    void addBuiltinOrbit(const QString& name, vesta::Trajectory* trajectory);
    void removeBuiltinOrbit(const QString& name);
    void addBuiltinRotationModel(const QString& name, vesta::RotationModel* trajectory);
    void removeBuiltinRotationModel(const QString& name);

    void setDataSearchPath(const QString& path);
    void setTextureSearchPath(const QString& path);
    void setModelSearchPath(const QString& path);

    void updateTle(const QString& source, const QString& name, const QString& line1, const QString& line2);

    QSet<QString> resourceRequests() const;
    void clearResourceRequests();

    void setCatalogLoaded(const QString& catalogFileName);

    /** This property is normally true, but should be set to false
      * in SSC compatibility mode.
      */
    void setTexturesInModelDirectory(bool enable)
    {
        m_texturesInModelDirectory = enable;
    }

    CatalogContents* loadCatalogFile(const QString& fileName,
                                     UniverseCatalog* catalog);
    void unloadSpiceKernels(const QStringList& kernelList);

    void clearMessageLog();
    QString messageLog();

public slots:
    void processUpdates();
    void processTleSet(const QString& source, QTextStream& stream);

private:
    vesta::Geometry* loadGeometry(const QVariantMap& map,
                                  const UniverseCatalog* catalog);
    vesta::Geometry* loadGlobeGeometry(const QVariantMap& map);
    vesta::PlanetaryRings* loadRingSystemGeometry(const QVariantMap& map);
    vesta::Geometry* loadMeshGeometry(const QVariantMap& map);
    vesta::Geometry* loadSensorGeometry(const QVariantMap& map,
                                        const UniverseCatalog* catalog);
    vesta::Geometry* loadSwarmGeometry(const QVariantMap& map);
    vesta::Geometry* loadParticleSystemGeometry(const QVariantMap& map);
    vesta::Geometry* loadTimeSwitchedGeometry(const QVariantMap& map,
                                              const UniverseCatalog* catalog);

    QList<vesta::counted_ptr<vesta::Arc> > loadChronology(const QVariantList& list,
                                                          const UniverseCatalog* catalog,
                                                          double startTime);

    vesta::Arc* loadArc(const QVariantMap& map,
                        const UniverseCatalog* catalog,
                        double startTime);
    vesta::Frame* loadFrame(const QVariantMap& map,
                            const UniverseCatalog* catalog);
    vesta::InertialFrame* loadInertialFrame(const QString& name);
    vesta::Frame* loadBodyFixedFrame(const QVariantMap& map, const UniverseCatalog* catalog);
    vesta::Frame* loadTwoVectorFrame(const QVariantMap& map, const UniverseCatalog* catalog);
    TwoVectorFrameDirection* loadFrameVector(const QVariantMap& map, const UniverseCatalog* catalog);
    TwoVectorFrameDirection* loadConstantFrameVector(const QVariantMap& map, const UniverseCatalog* catalog);

    vesta::Trajectory* loadTrajectory(const QVariantMap& map);
    vesta::Trajectory* loadBuiltinTrajectory(const QVariantMap& info);
    vesta::Trajectory* loadInterpolatedStatesTrajectory(const QVariantMap& info);
    vesta::Trajectory* loadChebyshevPolynomialsTrajectory(const QVariantMap& info);
    vesta::Trajectory* loadTleTrajectory(const QVariantMap& info);
    vesta::Trajectory* loadFixedPointTrajectory(const QVariantMap& info);
    vesta::Trajectory* loadFixedSphericalTrajectory(const QVariantMap& info);
    vesta::Trajectory* loadLinearCombinationTrajectory(const QVariantMap& info);
    vesta::Trajectory* loadCompositeTrajectory(const QVariantMap& info);
    vesta::Trajectory* loadSpiceTrajectory(const QVariantMap& info);

    vesta::RotationModel* loadRotationModel(const QVariantMap& info);
    vesta::RotationModel* loadBuiltinRotationModel(const QVariantMap& info);
    vesta::RotationModel* loadUniformRotationModel(const QVariantMap& info);
    vesta::RotationModel* loadInterpolatedRotationModel(const QVariantMap& info);
    vesta::RotationModel* loadFixedRotationModel(const QVariantMap& map);
    vesta::RotationModel* loadFixedEulerRotationModel(const QVariantMap& map);
    vesta::RotationModel* loadSpiceRotationModel(const QVariantMap& info);

    vesta::Visualizer* loadVisualizer(const QVariantMap& info,
                                      const UniverseCatalog* catalog);
    vesta::Visualizer* loadPlaneVisualizer(const QVariantMap& info,
                                           const UniverseCatalog* catalog);

    vesta::Visualizer* loadFeatureLabels(const QVariantMap& info,
                                         const vesta::Entity* body);

    Viewpoint* loadViewpoint(const QVariantMap& info,
                             UniverseCatalog* catalog);

    BodyInfo* loadBodyInfo(const QVariantMap& item);

    CatalogContents* loadCatalogItems(const QVariantMap& contents,
                                      UniverseCatalog* catalog,
                                      unsigned int requireDepth);

private:
    QString dataFileName(const QString& fileName);
    QString modelFileName(const QString& fileName);

    void cleanGeometryCache();
    vesta::Geometry* loadMeshFile(const QString& fileName);

    CatalogContents* loadCatalogFile(const QString& fileName,
                                     UniverseCatalog* catalog,
                                     unsigned int requireDepth);
    QStringList loadSSC(const QString& fileName,
                        UniverseCatalog* catalog,
                        unsigned int requireDepth);
    void loadSpiceKernels(const QStringList &kernelList);
    QStringList resolveSpiceKernelList(const QVariantList &kernelList);

    void errorMessage(const QString& message);
    void warningMessage(const QString& message);

private:
    QMap<QString, vesta::counted_ptr<vesta::Trajectory> > m_builtinOrbits;
    QMap<QString, vesta::counted_ptr<vesta::RotationModel> > m_builtinRotations;
    vesta::counted_ptr<PathRelativeTextureLoader> m_textureLoader;
    QMap<QString, vesta::counted_ptr<vesta::Geometry> > m_modelCache;
    QString m_dataSearchPath;
    QString m_textureSearchPath;
    QString m_modelSearchPath;
    QString m_currentBodyName;

    struct TleRecord
    {
        QString source;
        QString name;
        QString line1;
        QString line2;
    };

    QHash<QString,TleRecord> m_tleCache;
    QMultiHash<QString, vesta::counted_ptr<TleTrajectory> > m_tleTrajectories;
    QList<TleRecord> m_tleUpdates;
    QSet<QString> m_resourceRequests;

    QHash<QString, vesta::counted_ptr<vesta::Geometry> > m_geometryCache;

    QHash<QString, vesta::counted_ptr<vesta::Trajectory> > m_trajectoryCache;

    QSet<QString> m_loadedCatalogFiles;
    QString m_messageLog;

    bool m_texturesInModelDirectory;
};

#endif // _UNIVERSE_LOADER_H_
