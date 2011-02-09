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

#include "UniverseLoader.h"
#include "InterpolatedStateTrajectory.h"
#include "InterpolatedRotation.h"
#include "TwoVectorFrame.h"
#include "compatibility/Scanner.h"
#include <vesta/Units.h>
#include <vesta/Body.h>
#include <vesta/Arc.h>
#include <vesta/Trajectory.h>
#include <vesta/Frame.h>
#include <vesta/InertialFrame.h>
#include <vesta/BodyFixedFrame.h>
#include <vesta/RotationModel.h>
#include <vesta/UniformRotationModel.h>
#include <vesta/KeplerianTrajectory.h>
#include <vesta/FixedPointTrajectory.h>
#include <vesta/WorldGeometry.h>
#include <vesta/ArrowGeometry.h>
#include <vesta/MeshGeometry.h>
#include <vesta/Units.h>
#include <QFile>
#include <QDebug>

using namespace vesta;
using namespace Eigen;


class SimpleRotationModel : public RotationModel
{
public:
    SimpleRotationModel(double inclination,
                        double ascendingNode,
                        double rotationRate,
                        double meridianAngleAtEpoch,
                        double epoch) :
        m_rotationRate(rotationRate),
        m_meridianAngleAtEpoch(meridianAngleAtEpoch),
        m_epoch(epoch),
        m_rotation(AngleAxisd(ascendingNode, Vector3d::UnitZ()) * AngleAxisd(inclination, Vector3d::UnitX()))
    {
    }


    Quaterniond
    orientation(double t) const
    {
        double meridianAngle = m_meridianAngleAtEpoch + (t - m_epoch) * m_rotationRate;
        return m_rotation * Quaterniond(AngleAxis<double>(meridianAngle, Vector3d::UnitZ()));
    }


    Vector3d
    angularVelocity(double /* t */) const
    {
        return m_rotation * (Vector3d::UnitZ() * m_rotationRate);
    }

private:
    double m_rotationRate;
    double m_meridianAngleAtEpoch;
    double m_epoch;
    Quaterniond m_rotation;
};


static bool readNextDouble(Scanner* scanner, double* value)
{
    if (scanner->readNext() == Scanner::Double || scanner->currentToken() == Scanner::Integer)
    {
        *value = scanner->doubleValue();
        return true;
    }
    else
    {
        return false;
    }
}


static bool readNextVector3(Scanner* scanner, Vector3d* value)
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    if (readNextDouble(scanner, &x) && readNextDouble(scanner, &y) && readNextDouble(scanner, &z))
    {
        *value = Vector3d(x, y, z);
        return true;
    }
    else
    {
        return false;
    }
}


static bool readNextQuaternion(Scanner* scanner, Quaterniond* value)
{
    double w = 0.0;
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    if (readNextDouble(scanner, &w) &&
        readNextDouble(scanner, &x) &&
        readNextDouble(scanner, &y) &&
        readNextDouble(scanner, &z))
    {
        *value = Quaterniond(w, x, y, z);
        return true;
    }
    else
    {
        return false;
    }
}


/** Load a list of time/state vector records from a file. The values
  * are stored in ASCII format with newline terminated hash comments
  * allowed. Dates are given as TDB Julian dates, positions are
  * in units of kilometers, and velocities are km/sec.
  */
InterpolatedStateTrajectory*
LoadXYZVTrajectory(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Unable to open trajectory file " << fileName;
        return NULL;
    }

    InterpolatedStateTrajectory::TimeStateList states;

    Scanner scanner(&file);
    bool ok = true;
    bool done = false;
    while (!done)
    {
        double jd = 0.0;
        Vector3d position = Vector3d::Zero();
        Vector3d velocity = Vector3d::Zero();
        if (!readNextDouble(&scanner, &jd))
        {
            done = true;
            if (!scanner.atEnd())
            {
                ok = false;
            }
        }
        else
        {
            if (!readNextVector3(&scanner, &position) || !readNextVector3(&scanner, &velocity))
            {
                ok = false;
                done = true;
            }
        }

        if (!done)
        {
            double tdbSec = daysToSeconds(jd - vesta::J2000);
            InterpolatedStateTrajectory::TimeState state;
            state.tsec = tdbSec;
            state.state = StateVector(position, velocity);
            states.push_back(state);
        }
    }

    if (!ok)
    {
        qDebug() << "Error in xyzv trajectory file, record " << states.size();
        return NULL;
    }
    else
    {
        return new InterpolatedStateTrajectory(states);
    }
}


/** Load a list of time/position records from a file. The values
  * are stored in ASCII format with newline terminated hash comments
  * allowed. Dates are given as TDB Julian dates and positions are
  * in units of kilometers.
  */
InterpolatedStateTrajectory*
LoadXYZTrajectory(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Unable to open trajectory file " << fileName;
        return NULL;
    }

    InterpolatedStateTrajectory::TimePositionList positions;

    Scanner scanner(&file);
    bool ok = true;
    bool done = false;
    while (!done)
    {
        double jd = 0.0;
        Vector3d position = Vector3d::Zero();
        if (!readNextDouble(&scanner, &jd))
        {
            done = true;
            if (!scanner.atEnd())
            {
                ok = false;
            }
        }
        else
        {
            if (!readNextVector3(&scanner, &position))
            {
                ok = false;
                done = true;
            }
        }

        if (!done)
        {
            double tdbSec = daysToSeconds(jd - vesta::J2000);
            InterpolatedStateTrajectory::TimePosition record;
            record.tsec = tdbSec;
            record.position = position;
            positions.push_back(record);
        }
    }

    if (!ok)
    {
        qDebug() << "Error in xyz trajectory file, record " << positions.size();
        return NULL;
    }
    else
    {
        return new InterpolatedStateTrajectory(positions);
    }
}


/** Load a list of time/quaternion records from a file. The values
  * are stored in ASCII format with newline terminated hash comments
  * allowed. Dates are given as TDB Julian dates and orientations are
  * given as quaternions with components ordered w, x, y, z (i.e. the
  * real part of the quaternion is before the imaginary parts.)
  */
InterpolatedRotation*
LoadInterpolatedRotation(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Unable to open trajectory file " << fileName;
        return NULL;
    }

    InterpolatedRotation::TimeOrientationList orientations;

    Scanner scanner(&file);
    bool ok = true;
    bool done = false;
    while (!done)
    {
        double jd = 0.0;
        Quaterniond q = Quaterniond::Identity();
        if (!readNextDouble(&scanner, &jd))
        {
            done = true;
            if (!scanner.atEnd())
            {
                ok = false;
            }
        }
        else
        {
            if (!readNextQuaternion(&scanner, &q))
            {
                ok = false;
                done = true;
            }
        }

        if (!done)
        {
            double tdbSec = daysToSeconds(jd - vesta::J2000);
            InterpolatedRotation::TimeOrientation record;
            record.tsec = tdbSec;
            record.orientation = q;
            orientations.push_back(record);
        }
    }

    if (!ok)
    {
        qDebug() << "Error in .q orientation file, record " << orientations.size();
        return NULL;
    }
    else
    {
        return new InterpolatedRotation(orientations);
    }
}


UniverseLoader::UniverseLoader() :
    m_dataSearchPath(".")
{
}


UniverseLoader::~UniverseLoader()
{
}


static double doubleValue(QVariant v, double defaultValue = 0.0)
{
    bool ok = false;
    double value = v.toDouble(&ok);
    if (ok)
    {
        return value;
    }
    else
    {
        return defaultValue;
    }
}


static Vector3d vec3Value(QVariant v, bool* ok)
{
    Vector3d result = Vector3d::Zero();
    bool loadOk = false;

    if (v.type() == QVariant::List)
    {
        QVariantList list = v.toList();
        if (list.length() == 3)
        {
            if (list.at(0).canConvert(QVariant::Double) &&
                list.at(1).canConvert(QVariant::Double) &&
                list.at(2).canConvert(QVariant::Double))
            {
                result = Vector3d(list.at(0).toDouble(), list.at(1).toDouble(), list.at(2).toDouble());
                loadOk = true;
            }
        }
    }

    if (ok)
    {
        *ok = loadOk;
    }

    return result;
}


static double angleValue(QVariant v, double defaultValue = 0.0)
{
    bool ok = false;
    double value = v.toDouble(&ok);
    if (ok)
    {
        return toRadians(value);
    }
    else
    {
        return defaultValue;
    }
}


static double durationValue(QVariant v, double defaultValue = 0.0)
{
    bool ok = false;
    double value = v.toDouble(&ok);
    if (ok)
    {
        return value;
    }
    else
    {
        return defaultValue;
    }
}


vesta::Trajectory* loadFixedTrajectory(const QVariantMap& info)
{
    qDebug() << "Trajectory: " << info.value("type").toString();

    bool ok = false;
    Vector3d position = vec3Value(info.value("position"), &ok);
    if (!ok)
    {
        qDebug() << "Invalid or missing position given for FixedPoint trajectory.";
        return NULL;
    }

    return new FixedPointTrajectory(position);
}


vesta::Trajectory*
loadKeplerianTrajectory(const QVariantMap& info)
{
    double sma = doubleValue(info.value("semiMajorAxis"), 0.0);
    if (sma <= 0.0)
    {
        qDebug() << "Invalid semimajor axis given for Keplerian orbit.";
        return NULL;
    }

    double period = doubleValue(info.value("period"), 0.0);
    if (period <= 0.0)
    {
        qDebug() << "Invalid period given for Keplerian orbit.";
        return NULL;
    }

    OrbitalElements elements;
    elements.eccentricity = doubleValue(info.value("eccentricity"));
    elements.inclination = toRadians(doubleValue(info.value("inclination")));
    elements.meanMotion = toRadians(360.0) / daysToSeconds(period);
    elements.longitudeOfAscendingNode = toRadians(doubleValue(info.value("ascendingNode")));
    elements.argumentOfPeriapsis = toRadians(doubleValue(info.value("argumentOfPeriapsis")));
    elements.meanAnomalyAtEpoch = toRadians(doubleValue(info.value("meanAnomaly")));
    elements.periapsisDistance = (1.0 - elements.eccentricity) * sma;

    KeplerianTrajectory* trajectory = new KeplerianTrajectory(elements);

    return trajectory;
}


vesta::Trajectory*
UniverseLoader::loadBuiltinTrajectory(const QVariantMap& info)
{
    if (info.contains("name"))
    {
        QString name = info.value("name").toString();
        return m_builtinOrbits[name].ptr();
    }
    else
    {
        qDebug() << "Builtin trajectory is missing name.";
        return NULL;
    }
}


vesta::Trajectory*
UniverseLoader::loadInterpolatedStatesTrajectory(const QVariantMap& info)
{
    if (info.contains("source"))
    {
        QString name = info.value("source").toString();

        QString fileName = m_dataSearchPath + "/" + name;
        if (name.toLower().endsWith(".xyzv"))
        {
            return LoadXYZVTrajectory(fileName);
        }
        else if (name.toLower().endsWith(".xyz"))
        {
            return LoadXYZTrajectory(fileName);
        }
        else
        {
            qDebug() << "Unknown sampled trajectory format.";
            return NULL;
        }
    }
    else
    {
        qDebug() << "No source file specified for sampled trajectory.";
        return NULL;
    }
}


vesta::Trajectory*
UniverseLoader::loadTrajectory(const QVariantMap& map)
{
    QVariant typeData = map.value("type");
    if (typeData.type() != QVariant::String)
    {
        qDebug() << "Trajectory definition is missing type.";
    }

    QString type = typeData.toString();
    if (type == "FixedPoint")
    {
        return loadFixedTrajectory(map);
    }
    else if (type == "Keplerian")
    {
        return loadKeplerianTrajectory(map);
    }
    else if (type == "Builtin")
    {
        return loadBuiltinTrajectory(map);
    }
    else if (type == "InterpolatedStates")
    {
        return loadInterpolatedStatesTrajectory(map);
    }
    else
    {
        qDebug() << "Unknown trajectory type " << type;
    }

    return NULL;
}


vesta::RotationModel*
loadFixedRotationModel(const QVariantMap& map)
{
    qDebug() << "RotationModel: " << map.value("type").toString();
    return NULL;
}


vesta::RotationModel* loadUniformRotationModel(const QVariantMap& map)
{
    qDebug() << "RotationModel: " << map.value("type").toString();

    double inclination   = angleValue(map.value("inclination"));
    double ascendingNode = angleValue(map.value("ascendingNode"));
    double meridianAngle = angleValue(map.value("meridianAngle"));
    double period        = durationValue(map.value("period"), 0.0);

    Vector3d axis = (AngleAxisd(ascendingNode, Vector3d::UnitZ()) * AngleAxisd(inclination, Vector3d::UnitX())) * Vector3d::UnitZ();
    //Vector3d axis = (AngleAxisd(inclination, Vector3d::UnitX()) * AngleAxisd(ascendingNode, Vector3d::UnitZ())) * Vector3d::UnitZ();
    double rotationRate = 2 * PI / daysToSeconds(period);

    //return new UniformRotationModel(axis, rotationRate, meridianAngle);
    return new SimpleRotationModel(inclination, ascendingNode, rotationRate, meridianAngle, 0.0);
}


vesta::RotationModel*
UniverseLoader::loadInterpolatedRotationModel(const QVariantMap& info)
{
    if (info.contains("source"))
    {
        QString name = info.value("source").toString();

        QString fileName = m_dataSearchPath + "/" + name;
        if (name.toLower().endsWith(".q"))
        {
            return LoadInterpolatedRotation(fileName);
        }
        else
        {
            qDebug() << "Unknown interpolated rotation format.";
            return NULL;
        }
    }
    else
    {
        qDebug() << "No source file specified for interpolated rotation.";
        return NULL;
    }
}


vesta::RotationModel*
UniverseLoader::loadRotationModel(const QVariantMap& map)
{
    QVariant typeVar = map.value("type");
    if (typeVar.type() != QVariant::String)
    {
        qDebug() << "RotationModel definition is missing type.";
    }

    QString type = typeVar.toString();
    if (type == "Fixed")
    {
        return loadFixedRotationModel(map);
    }
    else if (type == "Uniform")
    {
        return loadUniformRotationModel(map);
    }
    else if (type == "Interpolated")
    {
        return loadInterpolatedRotationModel(map);
    }
    else
    {
        qDebug() << "Unknown rotation model type " << type;
    }

    return NULL;
}


vesta::InertialFrame* loadInertialFrame(const QString& name)
{
    if (name == "EclipticJ2000")
    {
        return InertialFrame::eclipticJ2000();
    }
    else if (name == "EquatorJ2000")
    {
        return InertialFrame::equatorJ2000();
    }
    else if (name == "EquatorB1950")
    {
        return InertialFrame::equatorB1950();
    }
    else if (name == "ICRF")
    {
        return InertialFrame::icrf();
    }
    qDebug() << "Inertial Frame: " << name;

    return NULL;
}


vesta::Frame* loadBodyFixedFrame(const QVariantMap& map,
                                 const UniverseCatalog* catalog)
{
    QVariant bodyVar = map.value("body");
    if (bodyVar.type() != QVariant::String)
    {
        qDebug() << "BodyFixed frame is missing body name.";
        return NULL;
    }

    QString bodyName = bodyVar.toString();
    Entity* body = catalog->find(bodyName);
    if (body)
    {
        BodyFixedFrame* frame = new BodyFixedFrame(body);
        return frame;
    }
    else
    {
        qDebug() << "BodyFixed frame refers to unknown body " << bodyName;
        return NULL;
    }
}


static bool parseAxisLabel(const QString& label, TwoVectorFrame::Axis* axis)
{
    bool validLabel = true;

    QString lcLabel = label.toLower();
    if (lcLabel == "x" || lcLabel == "+x")
    {
        *axis = TwoVectorFrame::PositiveX;
    }
    else if (lcLabel == "y" || lcLabel == "+y")
    {
        *axis = TwoVectorFrame::PositiveY;
    }
    else if (lcLabel == "z" || lcLabel == "+z")
    {
        *axis = TwoVectorFrame::PositiveZ;
    }
    else if (lcLabel == "-x")
    {
        *axis = TwoVectorFrame::NegativeX;
    }
    else if (lcLabel == "-y")
    {
        *axis = TwoVectorFrame::NegativeY;
    }
    else if (lcLabel == "-z")
    {
        *axis = TwoVectorFrame::NegativeZ;
    }
    else
    {
        validLabel = false;
    }

    return validLabel;
}


TwoVectorFrameDirection*
loadRelativePosition(const QVariantMap& map,
                     const UniverseCatalog* catalog)
{
    QVariant observerVar = map.value("observer");
    QVariant targetVar = map.value("target");

    if (observerVar.type() != QVariant::String)
    {
        qDebug() << "Bad or missing observer for RelativePosition direction";
        return NULL;
    }

    if (targetVar.type() != QVariant::String)
    {
        qDebug() << "Bad or missing target for RelativePosition direction";
        return NULL;
    }

    Entity* observer = catalog->find(observerVar.toString());
    if (!observer)
    {
        qDebug() << "Observer body " << observerVar.toString() << " for RelativePosition direction not found";
        return NULL;
    }

    Entity* target = catalog->find(targetVar.toString());
    if (!target)
    {
        qDebug() << "Target body " << targetVar.toString() << " for RelativePosition direction not found";
        return NULL;
    }

    return new RelativePositionVector(observer, target);
}


TwoVectorFrameDirection*
loadRelativeVelocity(const QVariantMap& map,
                     const UniverseCatalog* catalog)
{
    QVariant observerVar = map.value("observer");
    QVariant targetVar = map.value("target");

    if (observerVar.type() != QVariant::String)
    {
        qDebug() << "Bad or missing observer for RelativeVelocity direction";
        return NULL;
    }

    if (targetVar.type() != QVariant::String)
    {
        qDebug() << "Bad or missing target for RelativeVelocity direction";
        return NULL;
    }

    Entity* observer = catalog->find(observerVar.toString());
    if (!observer)
    {
        qDebug() << "Observer body " << observerVar.toString() << " for RelativeVelocity direction not found";
        return NULL;
    }

    Entity* target = catalog->find(targetVar.toString());
    if (!target)
    {
        qDebug() << "Target body " << targetVar.toString() << " for RelativeVelocity direction not found";
        return NULL;
    }

    return new RelativeVelocityVector(observer, target);
}


TwoVectorFrameDirection*
loadFrameVector(const QVariantMap& map,
                const UniverseCatalog* catalog)
{
    QVariant typeVar = map.value("type");
    if (typeVar.type() != QVariant::String)
    {
        qDebug() << "Bad or missing type for TwoVector frame direction.";
        return NULL;
    }

    QVariant type = typeVar.toString();
    if (type == "RelativePosition")
    {
        return loadRelativePosition(map, catalog);
    }
    else if (type == "RelativeVelocity")
    {
        return loadRelativeVelocity(map, catalog);
    }
    else
    {
        qDebug() << "Unknoown TwoVector frame direction type " << type;
        return NULL;
    }
}


vesta::Frame* loadTwoVectorFrame(const QVariantMap& map,
                                 const UniverseCatalog* catalog)
{
    QVariant primaryVar = map.value("primary");
    QVariant primaryAxisVar = map.value("primaryAxis");
    QVariant secondaryVar = map.value("secondary");
    QVariant secondaryAxisVar = map.value("secondaryAxis");

    if (primaryVar.type() != QVariant::Map)
    {
        qDebug() << "Invalid or missing primary direction in TwoVector frame";
        return NULL;
    }

    if (secondaryVar.type() != QVariant::Map)
    {
        qDebug() << "Invalid or missing secondary direction in TwoVector frame";
        return NULL;
    }

    if (primaryAxisVar.type() != QVariant::String)
    {
        qDebug() << "Invalid or missing primary axis in TwoVector frame";
        return NULL;
    }

    if (secondaryAxisVar.type() != QVariant::String)
    {
        qDebug() << "Invalid or missing secondary axis in TwoVector frame";
        return NULL;
    }

    TwoVectorFrame::Axis primaryAxis = TwoVectorFrame::PositiveX;
    TwoVectorFrame::Axis secondaryAxis = TwoVectorFrame::PositiveX;
    if (!parseAxisLabel(primaryAxisVar.toString(), &primaryAxis))
    {
        qDebug() << "Invalid label " << primaryAxisVar.toString() << " for primary axis in TwoVector frame";
        return NULL;
    }

    if (!parseAxisLabel(secondaryAxisVar.toString(), &secondaryAxis))
    {
        qDebug() << "Invalid label " << secondaryAxisVar.toString() << " for secondary axis in TwoVector frame";
        return NULL;
    }

    if (!TwoVectorFrame::orthogonalAxes(primaryAxis, secondaryAxis))
    {
        qDebug() << "Bad two vector frame. Primary and secondary axes must be orthogonal";
        return NULL;
    }

    TwoVectorFrameDirection* primaryDir = loadFrameVector(primaryVar.toMap(), catalog);
    TwoVectorFrameDirection* secondaryDir = loadFrameVector(secondaryVar.toMap(), catalog);

    if (primaryDir && secondaryDir)
    {
        return new TwoVectorFrame(primaryDir, primaryAxis, secondaryDir, secondaryAxis);
    }
    else
    {
        return NULL;
    }
}


vesta::Frame*
UniverseLoader::loadFrame(const QVariantMap& map,
                          const UniverseCatalog* catalog)
{
    QVariant typeVar = map.value("type");
    if (typeVar.type() != QVariant::String)
    {
        qDebug() << "Frame definition is missing type.";
    }

    QString type = typeVar.toString();
    if (type == "BodyFixed")
    {
        return loadBodyFixedFrame(map, catalog);
    }
    else if (type == "TwoVector")
    {
        return loadTwoVectorFrame(map, catalog);
    }
    else
    {
        qDebug() << "Unknown frame type " << type;
    }

    return NULL;
}


vesta::Arc*
UniverseLoader::loadArc(const QVariantMap& map,
                        const UniverseCatalog* catalog)
{
    vesta::Arc* arc = new vesta::Arc();

    QVariant centerData = map.value("center");
    QVariant trajectoryData = map.value("trajectory");
    QVariant rotationModelData = map.value("rotationModel");
    QVariant trajectoryFrameData = map.value("trajectoryFrame");
    QVariant bodyFrameData = map.value("bodyFrame");

    arc->setDuration(daysToSeconds(100 * 365.25));

    if (centerData.type() == QVariant::String)
    {
        QString centerName = centerData.toString();
        arc->setCenter(catalog->find(centerName));
    }
    else
    {
        qDebug() << "Missing center for object.";
        delete arc;
        return NULL;
    }

    if (trajectoryData.type() == QVariant::Map)
    {
        Trajectory* trajectory = loadTrajectory(trajectoryData.toMap());
        if (trajectory)
        {
            arc->setTrajectory(trajectory);
        }
    }

    if (rotationModelData.type() == QVariant::Map)
    {
        RotationModel* rotationModel = loadRotationModel(rotationModelData.toMap());
        if (rotationModel)
        {
            arc->setRotationModel(rotationModel);
        }
    }

    if (trajectoryFrameData.type() == QVariant::String)
    {
        // Inertial frame name
        InertialFrame* frame = loadInertialFrame(trajectoryFrameData.toString());
        if (frame)
        {
            arc->setTrajectoryFrame(frame);
        }
    }
    else if (trajectoryFrameData.type() == QVariant::Map)
    {
        Frame* frame = loadFrame(trajectoryFrameData.toMap(), catalog);
        if (frame)
        {
            arc->setTrajectoryFrame(frame);
        }
    }

    if (bodyFrameData.type() == QVariant::String)
    {
        // Inertial frame name
        InertialFrame* frame = loadInertialFrame(bodyFrameData.toString());
        if (frame)
        {
            arc->setTrajectoryFrame(frame);
        }
    }
    else if (bodyFrameData.type() == QVariant::Map)
    {
        Frame* frame = loadFrame(bodyFrameData.toMap(), catalog);
        if (frame)
        {
            arc->setBodyFrame(frame);
        }
    }

    return arc;
}


static MeshGeometry*
loadMeshFile(const QString& fileName, TextureMapLoader* textureLoader)
{
    MeshGeometry* meshGeometry = MeshGeometry::loadFromFile(fileName.toUtf8().data(), textureLoader);
    if (!meshGeometry)
    {
        //QMessageBox::warning(NULL, "Missing mesh file", QString("Error opening mesh file %1.").arg(fileName));
    }
    else
    {
        // Optimize the mesh. The optimizations can be expensive for large meshes, but they can dramatically
        // improve rendering performance. The best solution is to use mesh files that are already optimized, but
        // the average model loaded off the web benefits from some preprocessing at load time.
        meshGeometry->mergeSubmeshes();
        meshGeometry->uniquifyVertices();
    }

    return meshGeometry;
}


static WorldGeometry*
loadGlobeGeometry(const QVariantMap& map,
                  TextureMapLoader* textureLoader)
{
    Vector3d radii = Vector3d::Zero();

    if (map.value("radius").canConvert(QVariant::Double))
    {
        double r = map.value("radius").toDouble();
        radii = Vector3d::Constant(r);
    }
    else if (map.contains("radii"))
    {
        bool ok = false;
        radii = vec3Value(map.value("radii"), &ok);
        if (!ok)
        {
            qDebug() << "Invalid radii given for globe geometry.";
            return NULL;
        }
    }

    WorldGeometry* world = new WorldGeometry();
    world->setEllipsoid(radii.cast<float>() * 2.0f);

    TextureProperties props;
    props.addressS = TextureProperties::Wrap;
    props.addressT = TextureProperties::Clamp;

    if (map.contains("baseMap"))
    {
        QString baseMapName = map.value("baseMap").toString();
        if (textureLoader)
        {
            TextureMap* tex = textureLoader->loadTexture(baseMapName.toUtf8().data(), props);
            world->setBaseMap(tex);
        }
    }

    if (map.contains("normalMap"))
    {
        TextureProperties normalMapProps;
        normalMapProps.addressS = TextureProperties::Wrap;
        normalMapProps.addressT = TextureProperties::Clamp;
        normalMapProps.usage = TextureProperties::CompressedNormalMap;

        QString normalMapBase = map.value("normalMap").toString();
        if (textureLoader)
        {
            TextureMap* normalTex = textureLoader->loadTexture(normalMapBase.toUtf8().data(), normalMapProps);
            world->setNormalMap(normalTex);
        }
    }

    return world;
}


static MeshGeometry*
loadMeshGeometry(const QVariantMap& map,
                 TextureMapLoader* textureLoader)
{
    MeshGeometry* mesh = NULL;

    double radius = doubleValue(map.value("size"), 1.0);

    if (map.contains("source"))
    {
        QString sourceName = map.value("source").toString();
        mesh = loadMeshFile(sourceName, textureLoader);
        if (mesh)
        {
            mesh->setMeshScale(radius / mesh->boundingSphereRadius());
        }
    }

    return mesh;
}


static vesta::ArrowGeometry*
loadAxesGeometry(const QVariantMap& map)
{
    ArrowGeometry* axes = new ArrowGeometry(1.0f, 0.005f, 0.05f, 0.01f);
    axes->setVisibleArrows(ArrowGeometry::AllAxes);
    axes->setScale(float(doubleValue(map.value("scale"), 1.0)));

    return axes;
}


vesta::Geometry*
UniverseLoader::loadGeometry(const QVariantMap& map)
{
    Geometry* geometry = NULL;

    QVariant typeValue = map.value("type");
    if (typeValue.type() != QVariant::String)
    {
        qDebug() << "Bad or missing type for geometry.";
        return NULL;
    }

    QString type = typeValue.toString();

    qDebug() << "geometry type: " << type;

    if (type == "Globe")
    {
        geometry = loadGlobeGeometry(map, m_textureLoader.ptr());
    }
    else if (type == "Mesh")
    {
        geometry = loadMeshGeometry(map, m_textureLoader.ptr());
    }
    else if (type == "Axes")
    {
        geometry = loadAxesGeometry(map);
    }
    else
    {
        qDebug() << "Unknown type " << type << " for geometry.";
    }

    return geometry;
}


QStringList
UniverseLoader::loadSolarSystem(const QVariantMap& contents,
                                UniverseCatalog* catalog)
{
    qDebug() << contents["name"];

    QStringList bodyNames;

    if (!contents.contains("bodies"))
    {
        qDebug() << "No bodies defined.";
        return bodyNames;
    }

    if (contents["bodies"].type() != QVariant::List)
    {
        qDebug() << "Bodies is not a list.";
        return bodyNames;
    }
    QVariantList bodies = contents["bodies"].toList();

    foreach (QVariant body, bodies)
    {
        if (body.type() != QVariant::Map)
        {
            qDebug() << "Invalid item in bodies list.";
        }
        else
        {
            QVariantMap bodyInfo = body.toMap();
            vesta::Body* body = new vesta::Body();

            if (bodyInfo.contains("geometry"))
            {
                QVariant geometryValue = bodyInfo.value("geometry");
                if (geometryValue.type() == QVariant::Map)
                {
                    vesta::Geometry* geometry = loadGeometry(geometryValue.toMap());
                    if (geometry)
                    {
                        body->setGeometry(geometry);
                    }
                }
                else
                {
                    qDebug() << "Invalid geometry for body.";
                }
            }

            vesta::Arc* arc = loadArc(bodyInfo, catalog);

            if (!arc)
            {
                delete body;
            }
            else
            {
                body->chronology()->addArc(arc);
            }

            // Visible property
            body->setVisible(bodyInfo.value("visible", true).toBool());
            QString bodyName = bodyInfo["name"].toString();
            body->setName(bodyName.toUtf8().data());

            catalog->addBody(bodyName, body);
            bodyNames << bodyName;
        }
    }

    return bodyNames;
}


void
UniverseLoader::addBuiltinOrbit(const QString& name, vesta::Trajectory* trajectory)
{
    m_builtinOrbits[name] = trajectory;
}


void
UniverseLoader::removeBuiltinOrbit(const QString& name)
{
    m_builtinOrbits.remove(name);
}


void
UniverseLoader::setTextureLoader(vesta::TextureMapLoader *textureLoader)
{
    m_textureLoader = textureLoader;
}


void
UniverseLoader::setDataSearchPath(const QString& path)
{
    m_dataSearchPath = path;
}
