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

#include "UniverseLoader.h"
#include "AstorbLoader.h"
#include "ChebyshevPolyFileLoader.h"
#include "../TleTrajectory.h"
#include "../InterpolatedStateTrajectory.h"
#include "../InterpolatedRotation.h"
#include "../LinearCombinationTrajectory.h"
#include "../TwoVectorFrame.h"
#include "../WMSTiledMap.h"
#include "../MultiWMSTiledMap.h"
#include "../UnitConversion.h"
#include "../geometry/MeshInstanceGeometry.h"
#include "../geometry/TimeSwitchedGeometry.h"
#include "../geometry/FeatureLabelSetGeometry.h"
#include "../compatibility/Scanner.h"
#include "../compatibility/CmodLoader.h"
#include "../compatibility/CatalogParser.h"
#include "../compatibility/TransformCatalog.h"
#include "../compatibility/CelBodyFixedFrame.h"
#include "../vext/SimpleRotationModel.h"
#include "../vext/StripParticleGenerator.h"
#include "../vext/ArcStripParticleGenerator.h"
#include "../vext/PathRelativeTextureLoader.h"
#include "../vext/NameTemplateTiledMap.h"
#include "../vext/CompositeTrajectory.h"
#include "../astro/Rotation.h"
#include "../Viewpoint.h"
#include <vesta/Units.h>
#include <vesta/Body.h>
#include <vesta/Arc.h>
#include <vesta/Trajectory.h>
#include <vesta/Frame.h>
#include <vesta/InertialFrame.h>
#include <vesta/BodyFixedFrame.h>
#include <vesta/RotationModel.h>
#include <vesta/UniformRotationModel.h>
#include <vesta/FixedRotationModel.h>
#include <vesta/KeplerianTrajectory.h>
#include <vesta/FixedPointTrajectory.h>
#include <vesta/WorldGeometry.h>
#include <vesta/Atmosphere.h>
#include <vesta/DataChunk.h>
#include <vesta/ArrowGeometry.h>
#include <vesta/PlanetaryRings.h>
#include <vesta/SensorFrustumGeometry.h>
#include <vesta/AxesVisualizer.h>
#include <vesta/BodyDirectionVisualizer.h>
#include <vesta/LocalVisualizer.h>
#include <vesta/PlaneVisualizer.h>
#include <vesta/PlaneGeometry.h>
#include <vesta/ParticleSystemGeometry.h>
#include <vesta/Units.h>
#include <vesta/GregorianDate.h>

#ifdef SPICE_ENABLED
#include "../spice/SpiceTrajectory.h"
#include "../spice/SpiceRotationModel.h"
#endif

#include <vesta/particlesys/ParticleEmitter.h>
#include <vesta/particlesys/PointGenerator.h>
#include <vesta/particlesys/BoxGenerator.h>
#include <vesta/particlesys/DiscGenerator.h>

#include <qjson/parser.h>
#include <qjson/serializer.h>

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include <QBuffer>
#include <QDebug>

using namespace vesta;
using namespace Eigen;


static const double DefaultStartTime = daysToSeconds(-36525.0 * 2);  // 12:00:00 1 Jan 1800
static const double DefaultEndTime   = daysToSeconds( 36525.0);      // 12:00:00 1 Jan 2100

QString ValueUnitsRegexpString("^\\s*([-+]?[0-9]*\\.?[0-9]+(?:[eE][-+]?[0-9]+)?)\\s*([A-Za-z]+)?\\s*$");


QString TleKey(const QString& source, const QString& name)
{
    return source + "!" + name;
}


struct ColorPaletteEntry
{
    unsigned int rgb;
    const char* name;
};


// List of color names recognized by all (even very old) browsers
// magenta and cyan added for completeness
static ColorPaletteEntry StandardColorPalette[] =
{
    { 0x000000, "black" },
    { 0x000080, "navy" },
    { 0x0000FF, "blue" },
    { 0x008000, "green" },
    { 0x008080, "teal" },
    { 0x00FF00, "lime" },
    { 0x00FFFF, "aqua" },
    { 0x800000, "maroon" },
    { 0x800080, "purple" },
    { 0x808000, "olive" },
    { 0x808080, "gray" },
    { 0xC0C0C0, "silver" },
    { 0xFF0000, "red" },
    { 0xFF00FF, "fuchsia" },
    { 0xFFFF00, "yellow" },
    { 0xFFFFFF, "white" },
    { 0x00FFFF, "cyan" },
    { 0xFF00FF, "magenta" }
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


enum RotationConvention
{
    Standard_Rotation,
    Celestia_Rotation,
};

/** Load a list of time/quaternion records from a file. The values
  * are stored in ASCII format with newline terminated hash comments
  * allowed. Dates are given as TDB Julian dates and orientations are
  * given as quaternions with components ordered w, x, y, z (i.e. the
  * real part of the quaternion is before the imaginary parts.)
  */
InterpolatedRotation*
LoadInterpolatedRotation(const QString& fileName, RotationConvention mode)
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

            // All files *should* contain only unit quaternions, but not all of them do
            q.normalize();

            if (mode == Celestia_Rotation)
            {
                record.orientation = (xRotation(toRadians(90.0)) * q).conjugate();
            }
            else
            {
                // Normal mode
                record.orientation = q;
            }

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
    m_dataSearchPath("."),
    m_texturesInModelDirectory(true)
{
}


UniverseLoader::~UniverseLoader()
{
}


TextureMapLoader*
UniverseLoader::textureLoader() const
{
    return m_textureLoader.ptr();
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


static Spectrum colorValue(QVariant v, const Spectrum& defaultValue)
{
    Spectrum result = defaultValue;

    if (v.type() == QVariant::List)
    {
        bool ok = false;
        Vector3d vec = vec3Value(v, &ok);
        if (ok)
        {
            result = Spectrum(float(vec.x()), float(vec.y()), float(vec.z()));
        }
    }
    else if (v.type() == QVariant::String)
    {
        // Parse a color string: either a standard color name or a web-style hex value ('#ff8320')
        QString colorString = v.toString().toLower();
        bool colorOk = false;
        unsigned int rgb = 0;

        if (colorString.startsWith('#'))
        {
            if (colorString.length() == 7)
            {
                // Possibly a valid hex value
                // TODO: use a regexp to be absolutely sure
                rgb = colorString.right(6).toUInt(&colorOk, 16);
            }
        }
        else
        {
            for (unsigned int i = 0; i != sizeof(StandardColorPalette) / sizeof(StandardColorPalette[0]); ++i)
            {
                if (colorString == StandardColorPalette[i].name)
                {
                    rgb = StandardColorPalette[i].rgb;
                    colorOk = true;
                    break;
                }
            }
        }

        if (colorOk)
        {
            // We've got a valid color; unpack it from the integer rgb
            result = Spectrum(((rgb >> 16) & 0xff) / 255.0f,
                              ((rgb >> 8) & 0xff) / 255.0f,
                              (rgb & 0xff) / 255.0f);
        }
    }

    return result;
}


// Load a quaternion from a variant. The quaternion components are
// expected to be stored in a list in the order w, x, y, z
static Quaterniond quaternionValue(QVariant v, bool* ok)
{
    Quaterniond result = Quaterniond::Identity();
    bool loadOk = false;

    if (v.type() == QVariant::List)
    {
        QVariantList list = v.toList();
        if (list.length() == 4)
        {
            if (list.at(0).canConvert(QVariant::Double) &&
                list.at(1).canConvert(QVariant::Double) &&
                list.at(2).canConvert(QVariant::Double) &&
                list.at(3).canConvert(QVariant::Double))
            {
                result = Quaterniond(list.at(0).toDouble(), list.at(1).toDouble(), list.at(2).toDouble(), list.at(3).toDouble());
                loadOk = true;
            }
        }
    }

    if (ok)
    {
        *ok = loadOk;
    }

    result.normalize();

    return result;
}


// Load an angle from a variant and convert it to radians
static double angleValue(QVariant v, double defaultValue = 0.0, bool* ok = NULL)
{
    bool _ok = false;
    double value = v.toDouble(&_ok);
    if (ok)
    {
        *ok = _ok;
    }

    if (_ok)
    {
        return toRadians(value);
    }
    else
    {
        return defaultValue;
    }
}


static DistanceUnit parseDistanceUnit(const QString& unitString)
{
    if (unitString == "mm")
    {
        return Unit_Millimeter;
    }
    else if (unitString == "cm")
    {
        return Unit_Centimeter;
    }
    else if (unitString == "m")
    {
        return Unit_Meter;
    }
    else if (unitString == "km")
    {
        return Unit_Kilometer;
    }
    else if (unitString == "au")
    {
        return Unit_AU;
    }
    else
    {
        return InvalidDistanceUnit;
    }
}


static TimeUnit parseTimeUnit(const QString& unitString)
{
    if (unitString == "ms")
    {
        return Unit_Millisecond;
    }
    else if (unitString == "s")
    {
        return Unit_Second;
    }
    else if (unitString == "m")
    {
        return Unit_Minute;
    }
    else if (unitString == "h")
    {
        return Unit_Hour;
    }
    else if (unitString == "d")
    {
        return Unit_Day;
    }
    else if (unitString == "y" || unitString == "a")
    {
        return Unit_Year;
    }
    else
    {
        return InvalidTimeUnit;
    }
}


static MassUnit parseMassUnit(const QString& unitString)
{
    if (unitString == "g")
    {
        return Unit_Gram;
    }
    else if (unitString == "kg")
    {
        return Unit_Kilogram;
    }
    else if (unitString == "Mearth")
    {
        return Unit_EarthMass;
    }
    else
    {
        return InvalidMassUnit;
    }
}


// Return distance in kilometers.
static double distanceValue(QVariant v, DistanceUnit defaultUnit, double defaultValue, bool* ok = NULL)
{
    DistanceUnit unit = defaultUnit;
    double value = defaultValue;

    if (v.type() == QVariant::String)
    {
        QRegExp re(ValueUnitsRegexpString);
        if (re.indexIn(v.toString()) != -1)
        {
            QStringList parts = re.capturedTexts();
            value = parts[1].toDouble();

            QString unitString = parts[2];

            if (!unitString.isEmpty())
            {
                unit = parseDistanceUnit(unitString);
            }
        }
        else
        {
            // Error
            unit = InvalidDistanceUnit;
        }
    }
    else
    {
        bool convertOk = false;
        value = v.toDouble(&convertOk);
        if (!convertOk)
        {
            unit = InvalidDistanceUnit;
        }
    }


    if (ok)
    {
        *ok = (unit != InvalidDistanceUnit);
    }

    if (unit == InvalidDistanceUnit)
    {
        return 0.0;
    }
    else
    {
        return ConvertDistance(value, unit, Unit_Kilometer);
    }
}


// Load a duration value from a variant and convert it to seconds
static double durationValue(QVariant v, TimeUnit defaultUnit, double defaultValue, bool* ok)
{
    TimeUnit unit = defaultUnit;
    double value = defaultValue;

    if (v.type() == QVariant::String)
    {
        QRegExp re(ValueUnitsRegexpString);
        if (re.indexIn(v.toString()) != -1)
        {
            QStringList parts = re.capturedTexts();
            value = parts[1].toDouble();

            QString unitString = parts[2];

            if (!unitString.isEmpty())
            {
                unit = parseTimeUnit(unitString);
            }
        }
        else
        {
            unit = InvalidTimeUnit;
        }
    }
    else
    {
        bool convertOk = false;
        value = v.toDouble(&convertOk);
        if (!convertOk)
        {
            unit = InvalidTimeUnit;
        }
    }

    if (ok)
    {
        *ok = (unit != InvalidTimeUnit);
    }

    if (unit == InvalidTimeUnit)
    {
        return 0.0;
    }
    else
    {
        return ConvertTime(value, unit, Unit_Second);
    }
}


// Parse a date value. This can be either a double precision Julian date
// or an ISO 8601 date string with an optional time system suffix.
static double dateValue(QVariant v, bool* ok)
{
    double tsec = 0.0;

    if (v.type() == QVariant::String)
    {
        QString dateString = v.toString().trimmed();

        bool dateIsUtc = false;
        if (dateString.endsWith("UTC",Qt::CaseInsensitive)) {
            dateString.chop(3);
            dateString = dateString.trimmed();
            dateIsUtc = true;
        }
        else if (dateString.endsWith("TDB",Qt::CaseInsensitive)) {
            dateString.chop(3);
            dateString = dateString.trimmed();
        }

        // Try different methods of date parsing. The ISODate format requires the seconds field to be present
        // in the time, otherwise the time is silently ignored. This results in 2011-11-19 14:00 getting
        // treated as 2011-11-19 00:00:00
        QDateTime d;
        d = QDateTime::fromString(dateString, "yyyy-M-d hh:mm");
        if (!d.isValid())
        {
            d = QDateTime::fromString(dateString, Qt::ISODate);
        }

        if (d.isValid())
        {
            *ok = true;
            GregorianDate date;
            if (dateIsUtc) {
                date = GregorianDate(d.date().year(), d.date().month(), d.date().day(),
                                   d.time().hour(), d.time().minute(), d.time().second(), d.time().msec() * 1000,
                                   TimeScale_UTC);
            }
            else {
                date = GregorianDate(d.date().year(), d.date().month(), d.date().day(),
                                   d.time().hour(), d.time().minute(), d.time().second(), d.time().msec() * 1000,
                                   TimeScale_TDB);
            }
            tsec = date.toTDBSec();
        }
        else
        {
            *ok = false;
        }
    }
    else if (v.type() == QVariant::Double || v.type() == QVariant::Int)
    {
        *ok = true;
        double jd = v.toDouble();
        tsec = daysToSeconds(jd - vesta::J2000);
    }
    else
    {
        *ok = false;
    }

    return tsec;
}


// Return mass in kilograms.
static double massValue(QVariant v, MassUnit defaultUnit, double defaultValue, bool* ok = NULL)
{
    MassUnit unit = defaultUnit;
    double value = defaultValue;

    if (v.type() == QVariant::String)
    {
        QRegExp re(ValueUnitsRegexpString);
        if (re.indexIn(v.toString()) != -1)
        {
            QStringList parts = re.capturedTexts();
            value = parts[1].toDouble();

            QString unitString = parts[2];

            if (!unitString.isEmpty())
            {
                unit = parseMassUnit(unitString);
            }
        }
        else
        {
            // Error
            unit = InvalidMassUnit;
        }
    }
    else
    {
        bool convertOk = false;
        value = v.toDouble(&convertOk);
        if (!convertOk)
        {
            unit = InvalidMassUnit;
        }
    }

    if (ok)
    {
        *ok = (unit != InvalidMassUnit);
    }

    if (unit == InvalidMassUnit)
    {
        return 0.0;
    }
    else
    {
        return ConvertMass(value, unit, Unit_Kilogram);
    }
}


vesta::Trajectory*
UniverseLoader::loadFixedPointTrajectory(const QVariantMap& info)
{
    bool ok = false;

    Vector3d position = vec3Value(info.value("position"), &ok);
    if (!ok)
    {
        errorMessage("Invalid or missing position given for FixedPoint trajectory.");
        return NULL;
    }

    return new FixedPointTrajectory(position);
}


vesta::Trajectory*
UniverseLoader::loadFixedSphericalTrajectory(const QVariantMap& map)
{
    bool ok = false;

    QVariant latitudeVar = map.value("latitude");
    QVariant longitudeVar = map.value("longitude");
    QVariant radiusVar = map.value("radius");

    double latitude = angleValue(latitudeVar, 0.0, &ok);
    if (!ok)
    {
        errorMessage("Bad or missing latitude for FixedSpherical trajectory");
        return NULL;
    }

    double longitude = angleValue(longitudeVar, 0.0, &ok);
    if (!ok)
    {
        errorMessage("Bad or missing longitude for FixedSpherical trajectory");
        return NULL;
    }

    double radius = distanceValue(radiusVar, Unit_Kilometer, 0.0, &ok);
    if (!ok)
    {
        errorMessage("Bad or missing radius for FixedSpherical trajectory");
        return NULL;
    }

    Vector3d position(cos(latitude) * cos(longitude), cos(latitude) * sin(longitude), sin(latitude));
    return new FixedPointTrajectory(position * radius);
}


vesta::Trajectory*
loadKeplerianTrajectory(const QVariantMap& info)
{
    bool ok = false;

    QVariant semiMajorAxisVar = info.value("semiMajorAxis");
    double sma = distanceValue(semiMajorAxisVar, Unit_Kilometer, 0.0, &ok);
    if (!ok)
    {
        qDebug() << "Missing or invalid semi-major axis for Keplerian orbit.";
        return NULL;
    }

    QVariant periodVar = info.value("period");
    double period = durationValue(periodVar, Unit_Day, 1.0, &ok);
    if (!ok)
    {
        qDebug() << "Missing or invalid period for Keplerian orbit.";
        return NULL;
    }

    OrbitalElements elements;
    elements.eccentricity = doubleValue(info.value("eccentricity"));
    elements.inclination = toRadians(doubleValue(info.value("inclination")));
    elements.meanMotion = toRadians(360.0) / period;
    elements.longitudeOfAscendingNode = toRadians(doubleValue(info.value("ascendingNode")));
    elements.argumentOfPeriapsis = toRadians(doubleValue(info.value("argumentOfPeriapsis")));
    elements.meanAnomalyAtEpoch = toRadians(doubleValue(info.value("meanAnomaly")));
    elements.periapsisDistance = (1.0 - elements.eccentricity) * sma;

    QVariant epochVar = info.value("epoch");
    if (epochVar.isValid())
    {
        elements.epoch = dateValue(epochVar, &ok);
        if (!ok)
        {
            qDebug() << "Invalid epoch for Keplerian orbit.";
        }
    }

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
        errorMessage("Builtin trajectory is missing name.");
        return NULL;
    }
}


vesta::Trajectory*
UniverseLoader::loadChebyshevPolynomialsTrajectory(const QVariantMap& info)
{
    double period = 0.0;
    bool isPeriodic = false;

    if (info.contains("period"))
    {
        bool ok = false;
        period = durationValue(info.value("period"), Unit_Day, 0.0, &ok);
        if (!ok)
        {
            errorMessage("Invalid period given for Chebyshev polynomial trajectory.");
            return NULL;
        }
        else
        {
            isPeriodic = true;
        }
    }

    if (info.contains("source"))
    {
        QString name = info.value("source").toString();

        QString fileName = dataFileName(name);
        counted_ptr<Trajectory> trajectory = m_trajectoryCache.value(fileName);
        if (trajectory.isValid())
        {
            return trajectory.ptr();
        }

        ChebyshevPolyTrajectory* chebTrajectory = LoadChebyshevPolyFile(fileName);
        if (chebTrajectory && isPeriodic)
        {
            chebTrajectory->setPeriod(period);
        }

        trajectory = chebTrajectory;
        if (!trajectory.isValid())
        {
            errorMessage(QString("Chebyshev polynomial trajectory file %1 not found or invalid").arg(fileName));
            return NULL;
        }

        // Save the loaded trajectory in the cache
        m_trajectoryCache[fileName] = trajectory;

        return trajectory.ptr();
    }
    else
    {
        errorMessage("No source file specified for Chebyshev polynomials trajectory.");
        return NULL;
    }
}


vesta::Trajectory*
UniverseLoader::loadInterpolatedStatesTrajectory(const QVariantMap& info)
{
    if (info.contains("source"))
    {
        QString name = info.value("source").toString();

        QString fileName = dataFileName(name);
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
            errorMessage("Unknown sampled trajectory format.");
            return NULL;
        }
    }
    else
    {
        errorMessage("No source file specified for sampled trajectory.");
        return NULL;
    }
}


vesta::Trajectory*
UniverseLoader::loadTleTrajectory(const QVariantMap& info)
{
    QVariant nameVar = info.value("name");
    QVariant line1Var = info.value("line1");
    QVariant line2Var = info.value("line2");
    QVariant sourceVar = info.value("source");

    if (nameVar.type() != QVariant::String)
    {
        errorMessage("Bad or missing name for TLE trajectory");
        return NULL;
    }

    if (line1Var.type() != QVariant::String)
    {
        errorMessage("Bad or missing first line (line1) for TLE trajectory");
        return NULL;
    }

    if (line2Var.type() != QVariant::String)
    {
        errorMessage("Bad or missing second line (line2) for TLE trajectory");
        return NULL;
    }

    QString name = nameVar.toString();
    QString source = sourceVar.toString();
    QString line1 = line1Var.toString();
    QString line2 = line2Var.toString();

    QString key;
    if (!source.isEmpty())
    {
        key = TleKey(source, name);
        if (m_tleCache.contains(key))
        {
            // Use the cached value
            TleRecord cachedTle = m_tleCache.value(key);
            line1 = cachedTle.line1;
            line2 = cachedTle.line2;
        }
        else
        {
            // Not cached; request a new TLE set (probably from some URL) and
            // we'll update the trajectory when the data arrives.
            m_resourceRequests.insert(source);
        }
    }

    counted_ptr<TleTrajectory> tleTrajectory(TleTrajectory::Create(line1.toLatin1().data(),
                                                                   line2.toLatin1().data()));
    if (tleTrajectory.isNull())
    {
        errorMessage(QString("Invalid TLE data for '%1'").arg(name));
        return NULL;
    }

    // Only keep track of TLEs for which a source was specified; the others will
    // never need to be updated.
    if (!key.isEmpty())
    {
        m_tleTrajectories.insert(key, tleTrajectory);
    }

    return tleTrajectory.ptr();
}


vesta::Trajectory*
UniverseLoader::loadLinearCombinationTrajectory(const QVariantMap& map)
{
    QVariant trajectoriesVar = map.value("trajectories");
    QVariant weightsVar = map.value("weights");
    QVariant periodVar = map.value("period");

    if (!trajectoriesVar.isValid())
    {
        errorMessage("Trajectories list missing from LinearCombination trajectory");
        return NULL;
    }

    if (!weightsVar.isValid())
    {
        errorMessage("Weights list missing from LinearCombination trajectory");
        return NULL;
    }

    if (trajectoriesVar.type() != QVariant::List)
    {
        errorMessage("In LinearCombination trajectory, 'trajectories' must be a list");
        return NULL;
    }

    if (weightsVar.type() != QVariant::List)
    {
        errorMessage("In LinearCombination trajectory, 'weights' must be a list");
        return NULL;
    }

    QVariantList trajectories = trajectoriesVar.toList();
    QVariantList weights = weightsVar.toList();

    if (trajectories.size() != weights.size())
    {
        errorMessage("Must have one weight for each trajectory in LinearCombination trajectory");
        return NULL;
    }

    // This requirement may be relaxed eventually
    if (trajectories.size() != 2)
    {
        errorMessage("LinearCombination trajectory must contain exactly two child trajectories");
        return NULL;
    }

    QList<counted_ptr<Trajectory> > trajectoryList;
    QList<double> weightList;

    for (int i = 0; i < trajectories.size(); ++i)
    {
        QVariant trajectoryVar = trajectories.at(i);
        if (trajectoryVar.type() != QVariant::Map)
        {
            errorMessage("Invalid child trajectory in LinearCombination trajectory");
            return NULL;
        }

        Trajectory* t = loadTrajectory(trajectoryVar.toMap());
        if (!t)
        {
            return NULL;
        }

        trajectoryList << counted_ptr<Trajectory>(t);

        bool weightOk = false;
        double weight = weights.at(i).toDouble(&weightOk);
        if (!weightOk)
        {
            errorMessage("Invalid weight in LinearCombinationTrajectory");
            return NULL;
        }

        weightList << weight;
    }

    LinearCombinationTrajectory* lct =  new LinearCombinationTrajectory(trajectoryList.at(0).ptr(), weightList.at(0),
                                                                        trajectoryList.at(1).ptr(), weightList.at(1));

    if (periodVar.isValid())
    {
        bool periodOk = false;
        double period = durationValue(periodVar, Unit_Day, 1.0, &periodOk);
        if (!periodOk)
        {
            delete lct;
            return NULL;
        }

        if (period > 0.0)
        {
            lct->setPeriod(period);
        }
    }

    return lct;
}


vesta::Trajectory*
UniverseLoader::loadCompositeTrajectory(const QVariantMap& map)
{
    QVariant segmentsVar = map.value("segments");
    QVariant startTimeVar = map.value("startTime");

    bool ok = false;
    double startTime = dateValue(startTimeVar, &ok);
    if (!ok)
    {
        errorMessage("Invalid startTime specified for composite trajectory");
        return NULL;
    }

    if (segmentsVar.type() != QVariant::List)
    {
        errorMessage("Segments in composite trajectory must be an array");
        return NULL;
    }

    QVariantList segmentList = segmentsVar.toList();
    if (segmentList.isEmpty())
    {
        errorMessage("Composite trajectory must contain at least one segment");
        return NULL;
    }

    std::vector<counted_ptr<Trajectory> > segments;
    std::vector<double> durations;

    double lastEndTime = startTime;

    foreach (QVariant v, segmentList)
    {
        if (v.type() != QVariant::Map)
        {
            errorMessage("Invalid segment in segments list.");
            return NULL;
        }

        QVariantMap segmentMap = v.toMap();

        QVariant trajectoryVar = segmentMap.value("trajectory");
        QVariant endTimeVar = segmentMap.value("endTime");

        if (trajectoryVar.type() != QVariant::Map)
        {
            errorMessage("Bad or missing trajectory for composite trajectory segment");
            return NULL;
        }

        double endTime = dateValue(endTimeVar, &ok);
        if (!ok)
        {
            errorMessage("Bad or missing endTime for composite trajectory segment");
            return NULL;
        }

        if (endTime <= startTime)
        {
            errorMessage("End time of composite trajectory segment must be after start time");
            return NULL;
        }

        if (endTime <= lastEndTime)
        {
            errorMessage("End time of composite trajectory segment must be after previous segment's");
            return NULL;
        }
        double duration = endTime - lastEndTime;
        lastEndTime = endTime;

        Trajectory* trajectory = loadTrajectory(trajectoryVar.toMap());
        if (!trajectory)
        {
            return NULL;
        }

        segments.push_back(counted_ptr<vesta::Trajectory>(trajectory));
        durations.push_back(duration);
    }

    // Made it through all that without errors; now actually create the composite trajectory
    // First convert the list of counted pointers to a list of plain pointers
    std::vector<Trajectory*> segmentPtrs;
    for (unsigned int i = 0; i < segments.size(); ++i)
    {
        segmentPtrs.push_back(segments[i].ptr());
    }

    return CompositeTrajectory::Create(segmentPtrs, durations, startTime);
}


#ifdef SPICE_ENABLED
bool getNAIFCode(const QVariant& v, SpiceInt* code)
{
    bool isInteger = false;
    int integerCode = v.toInt(&isInteger);

    if (isInteger)
    {
        *code = integerCode;
        return true;
    }
    else if (v.canConvert(QVariant::String))
    {
        SpiceBoolean found = SPICEFALSE;
        bodn2c_c(v.toString().toLatin1().data(), code, &found);
        return found == SPICETRUE;
    }
    else
    {
        return false;
    }
}
#endif

vesta::Trajectory*
UniverseLoader::loadSpiceTrajectory(const QVariantMap& map)
{
#ifndef SPICE_ENABLED
    errorMessage("SPICE support unavailable in this version of Cosmographia.");
    return NULL;
#else
    QVariant targetVar = map.value("target");
    QVariant centerVar = map.value("center");
    QVariant frameVar = map.value("frame");

    QString spiceFrame = "J2000";
    if (!frameVar.isNull())
    {
        spiceFrame = frameVar.toString();
    }

    if (targetVar.isNull())
    {
        errorMessage("Target missing in SPICE trajectory.");
        return NULL;
    }

    if (centerVar.isNull())
    {
        errorMessage("Center missing in SPICE trajectory.");
        return NULL;
    }

    SpiceInt targetID = 0;
    SpiceInt centerID = 0;

    if (!getNAIFCode(targetVar, &targetID))
    {
        errorMessage("Unknown target '" + targetVar.toString() + "' for SPICE trajectory.");
        return NULL;
    }

    if (!getNAIFCode(centerVar, &centerID))
    {
        errorMessage("Unknown center '" + centerVar.toString() + "' for SPICE trajectory.");
        return NULL;
    }

    SpiceTrajectory* trajectory = new SpiceTrajectory(targetID, centerID, spiceFrame.toLatin1().data());

    return trajectory;
#endif
}



vesta::Trajectory*
UniverseLoader::loadTrajectory(const QVariantMap& map)
{
    QVariant typeData = map.value("type");
    if (typeData.type() != QVariant::String)
    {
        errorMessage("Trajectory definition is missing type.");
    }

    QString type = typeData.toString();
    if (type == "FixedPoint")
    {
        return loadFixedPointTrajectory(map);
    }
    else if (type == "FixedSpherical")
    {
        return loadFixedSphericalTrajectory(map);
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
    else if (type == "ChebyshevPoly")
    {
        return loadChebyshevPolynomialsTrajectory(map);
    }
    else if (type == "TLE")
    {
        return loadTleTrajectory(map);
    }
    else if (type == "LinearCombination")
    {
        return loadLinearCombinationTrajectory(map);
    }
    else if (type == "Composite")
    {
        return loadCompositeTrajectory(map);
    }
    else if (type == "Spice")
    {
        return loadSpiceTrajectory(map);
    }
    else
    {
        errorMessage(QString("Unknown trajectory type '%1'").arg(type));
    }

    return NULL;
}


vesta::RotationModel*
UniverseLoader::loadFixedRotationModel(const QVariantMap& map)
{
    QVariant quatVar = map.value("quaternion");
    if (quatVar.isValid())
    {
        bool ok = false;
        Quaterniond q = quaternionValue(quatVar, &ok);
        if (!ok)
        {
            errorMessage("Invalid quaternion given for FixedRotation");
            return NULL;
        }
        else
        {
            return new FixedRotationModel(q);
        }
    }
    else
    {
        double inclination   = angleValue(map.value("inclination"));
        double ascendingNode = angleValue(map.value("ascendingNode"));
        double meridianAngle = angleValue(map.value("meridianAngle"));

        Quaterniond q = (AngleAxisd(ascendingNode, Vector3d::UnitZ()) *
                         AngleAxisd(inclination, Vector3d::UnitX()) *
                         AngleAxisd(meridianAngle, Vector3d::UnitZ()));

        return new FixedRotationModel(q);
    }

    return NULL;
}


vesta::RotationModel*
UniverseLoader::loadFixedEulerRotationModel(const QVariantMap& map)
{
    QVariant sequenceVar = map.value("sequence");
    QVariant anglesVar = map.value("angles");

    if (sequenceVar.type() != QVariant::String)
    {
        errorMessage("Bad or missing sequence for FixedEuler rotation model");
        return NULL;
    }

    if (anglesVar.type() != QVariant::List)
    {
        errorMessage("Bad or missing angles list for FixedEuler rotation model");
        return NULL;
    }

    QString sequence = sequenceVar.toString();
    QVariantList angles = anglesVar.toList();

    if (sequence.length() != angles.length())
    {
        errorMessage("Count of angles doesn't match sequence length for FixedEuler rotation model");
        return NULL;
    }

    Quaterniond q = Quaterniond::Identity();
    for (int i = 0; i < sequence.length(); ++i)
    {
        bool ok = false;
        double thetaDeg = angles.at(i).toDouble(&ok);
        if (!ok)
        {
            errorMessage("Bad angle in FixedEuler rotation model");
            return NULL;
        }

        double theta = toRadians(thetaDeg);
        Quaterniond r = Quaterniond::Identity();
        QChar axisId = sequence.at(i);
        if (axisId == '1' || axisId == 'x' || axisId == 'X')
        {
            r = AngleAxisd(theta, Vector3d::UnitX());
        }
        else if (axisId == '2' || axisId == 'y' || axisId == 'Y')
        {
            r = AngleAxisd(theta, Vector3d::UnitY());
        }
        else if (axisId == '3' || axisId == 'z' || axisId == 'Z')
        {
            r = AngleAxisd(theta, Vector3d::UnitZ());
        }
        else
        {
            errorMessage(QString("Bad axis identifier '%1' in FixedEuler sequence").arg(axisId));
            return NULL;
        }

        q = q * r;
    }

    return new FixedRotationModel(q);
}


vesta::RotationModel*
UniverseLoader::loadSpiceRotationModel(const QVariantMap &map)
{
#ifndef SPICE_ENABLED
    errorMessage("SPICE support unavailable in this version of Cosmographia.");
    return NULL;
#else
    QVariant fromFrameVar = map.value("fromFrame");
    QVariant toFrameVar = map.value("toFrame");

    if (fromFrameVar.isNull())
    {
        errorMessage("fromFrame missing in SPICE rotation model.");
        return NULL;
    }

    QString fromFrame = fromFrameVar.toString();
    QString toFrame = "J2000";
    if (!toFrameVar.isNull())
    {
        toFrame = toFrameVar.toString();
    }

    SpiceRotationModel* rotationModel = new SpiceRotationModel(fromFrame.toLatin1().data(), toFrame.toLatin1().data());

    return rotationModel;
#endif
}


vesta::RotationModel*
UniverseLoader::loadUniformRotationModel(const QVariantMap& map)
{
    bool ok = false;

    double inclination   = angleValue(map.value("inclination"));
    double ascendingNode = angleValue(map.value("ascendingNode"));
    double meridianAngle = angleValue(map.value("meridianAngle"));
    double period        = durationValue(map.value("period"), Unit_Day, 1.0, &ok);

    double epoch = 0.0;
    QVariant epochVar = map.value("epoch");
    if (epochVar.isValid())
    {
        epoch = dateValue(epochVar, &ok);
        if (!ok)
        {
            errorMessage("Invalid epoch for uniform rotation.");
            return NULL;
        }
    }

    Vector3d axis = (AngleAxisd(ascendingNode, Vector3d::UnitZ()) * AngleAxisd(inclination, Vector3d::UnitX())) * Vector3d::UnitZ();
    //Vector3d axis = (AngleAxisd(inclination, Vector3d::UnitX()) * AngleAxisd(ascendingNode, Vector3d::UnitZ())) * Vector3d::UnitZ();
    double rotationRate = 2 * PI / period;

    //return new UniformRotationModel(axis, rotationRate, meridianAngle);
    return new SimpleRotationModel(inclination, ascendingNode, rotationRate, meridianAngle, epoch);
}


vesta::RotationModel*
UniverseLoader::loadBuiltinRotationModel(const QVariantMap& info)
{
    if (info.contains("name"))
    {
        QString name = info.value("name").toString();
        return m_builtinRotations[name].ptr();
    }
    else
    {
        errorMessage("Builtin rotation model is missing name.");
        return NULL;
    }
}


vesta::RotationModel*
UniverseLoader::loadInterpolatedRotationModel(const QVariantMap& info)
{
    if (info.contains("source"))
    {
        QString name = info.value("source").toString();

        // Check the compatibility flag; Celestia uses non-standard coordinate
        // system conventions, so orientations must be converted.
        RotationConvention rotationConvention = Standard_Rotation;
        if (info.value("compatibility").toString() == "celestia")
        {
            rotationConvention = Celestia_Rotation;
        }

        QString fileName = dataFileName(name);
        if (name.toLower().endsWith(".q"))
        {
            return LoadInterpolatedRotation(fileName, rotationConvention);
        }
        else
        {
            errorMessage("Unknown interpolated rotation format.");
            return NULL;
        }
    }
    else
    {
        errorMessage("No source file specified for interpolated rotation.");
        return NULL;
    }
}


vesta::RotationModel*
UniverseLoader::loadRotationModel(const QVariantMap& map)
{
    QVariant typeVar = map.value("type");
    if (typeVar.type() != QVariant::String)
    {
        errorMessage("RotationModel definition is missing type.");
    }

    QString type = typeVar.toString();
    if (type == "Fixed")
    {
        return loadFixedRotationModel(map);
    }
    if (type == "FixedEuler")
    {
        return loadFixedEulerRotationModel(map);
    }
    else if (type == "Uniform")
    {
        return loadUniformRotationModel(map);
    }
    else if (type == "Builtin")
    {
        return loadBuiltinRotationModel(map);
    }
    else if (type == "Interpolated")
    {
        return loadInterpolatedRotationModel(map);
    }
    else if (type == "Spice")
    {
        return loadSpiceRotationModel(map);
    }
    else
    {
        errorMessage(QString("Unknown rotation model type '%1'").arg(type));
    }

    return NULL;
}


vesta::InertialFrame*
UniverseLoader::loadInertialFrame(const QString& name)
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
    else
    {
        errorMessage(QString("Unknown inertial frame: '%1'").arg(name));
        return NULL;
    }
}


vesta::Frame*
UniverseLoader::loadBodyFixedFrame(const QVariantMap& map,
                                   const UniverseCatalog* catalog)
{
    QVariant bodyVar = map.value("body");
    if (bodyVar.type() != QVariant::String)
    {
        errorMessage("BodyFixed frame is missing body name.");
        return NULL;
    }

    QString bodyName = bodyVar.toString();
    Entity* body = catalog->find(bodyName);
    if (body)
    {
        vesta::Frame* frame = NULL;
        if (map.value("compatibility").toString() == "celestia")
        {
            frame = new CelBodyFixedFrame(body);
        }
        else
        {
            frame = new BodyFixedFrame(body);
        }

        return frame;
    }
    else
    {
        errorMessage(QString("BodyFixed frame refers to unknown body '%1'").arg(bodyName));
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
UniverseLoader::loadConstantFrameVector(const QVariantMap& map,
                                        const UniverseCatalog* catalog)
{
    QVariant directionVar = map.value("direction");
    QVariant frameVar = map.value("frame");

    if (!directionVar.isValid())
    {
        errorMessage("Direction missing for ConstantVector");
        return NULL;
    }

    bool ok = false;
    Vector3d direction = vec3Value(directionVar, &ok);
    if (!ok)
    {
        errorMessage("Invalid vector given for ConstantVector direction");
        return NULL;
    }

    if (direction.isZero())
    {
        errorMessage("Zero vector is not permitted for ConstantVector direction");
        return NULL;
    }

    direction.normalize();

    vesta::Frame* frame = InertialFrame::equatorJ2000();
    if (frameVar.isValid())
    {
        if (frameVar.type() == QVariant::String)
        {
            // Inertial frame name
            frame = loadInertialFrame(frameVar.toString());
        }
        else if (frameVar.type() == QVariant::Map)
        {
            frame = loadFrame(frameVar.toMap(), catalog);
        }
        else
        {
            frame = NULL;
            errorMessage("Invalid frame given for ConstantVector");
        }

        if (!frame)
        {
            return NULL;
        }
    }

    return new ConstantFrameDirection(frame, direction);
}


TwoVectorFrameDirection*
UniverseLoader::loadFrameVector(const QVariantMap& map,
                                const UniverseCatalog* catalog)
{
    QVariant typeVar = map.value("type");
    if (typeVar.type() != QVariant::String)
    {
        errorMessage("Bad or missing type for TwoVector frame direction.");
        return NULL;
    }

    QString type = typeVar.toString();
    if (type == "RelativePosition")
    {
        return loadRelativePosition(map, catalog);
    }
    else if (type == "RelativeVelocity")
    {
        return loadRelativeVelocity(map, catalog);
    }
    else if (type == "ConstantVector")
    {        
        return loadConstantFrameVector(map, catalog);
    }
    else
    {
        errorMessage(QString("Unknoown TwoVector frame direction type '%1'").arg(type));
        return NULL;
    }
}


vesta::Frame*
UniverseLoader::loadTwoVectorFrame(const QVariantMap& map,
                                   const UniverseCatalog* catalog)
{
    QVariant primaryVar = map.value("primary");
    QVariant primaryAxisVar = map.value("primaryAxis");
    QVariant secondaryVar = map.value("secondary");
    QVariant secondaryAxisVar = map.value("secondaryAxis");

    if (primaryVar.type() != QVariant::Map)
    {
        errorMessage("Invalid or missing primary direction in TwoVector frame");
        return NULL;
    }

    if (secondaryVar.type() != QVariant::Map)
    {
        errorMessage("Invalid or missing secondary direction in TwoVector frame");
        return NULL;
    }

    if (primaryAxisVar.type() != QVariant::String)
    {
        errorMessage("Invalid or missing primary axis in TwoVector frame");
        return NULL;
    }

    if (secondaryAxisVar.type() != QVariant::String)
    {
        errorMessage("Invalid or missing secondary axis in TwoVector frame");
        return NULL;
    }

    TwoVectorFrame::Axis primaryAxis = TwoVectorFrame::PositiveX;
    TwoVectorFrame::Axis secondaryAxis = TwoVectorFrame::PositiveX;
    if (!parseAxisLabel(primaryAxisVar.toString(), &primaryAxis))
    {
        errorMessage(QString("Invalid label '%1' for primary axis in TwoVector frame").arg(primaryAxisVar.toString()));
        return NULL;
    }

    if (!parseAxisLabel(secondaryAxisVar.toString(), &secondaryAxis))
    {
        errorMessage(QString("Invalid label '%1' for secondary axis in TwoVector frame").arg(secondaryAxisVar.toString()));
        return NULL;
    }

    if (!TwoVectorFrame::orthogonalAxes(primaryAxis, secondaryAxis))
    {
        errorMessage("Bad two vector frame. Primary and secondary axes must be orthogonal");
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
        errorMessage("Frame definition is missing type.");
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
        Frame* frame = loadInertialFrame(type);
        if (!frame)
        {
            errorMessage(QString("Unknown frame type '%1'").arg(type));
        }
        else
        {
            return frame;
        }
    }

    return NULL;
}


vesta::Arc*
UniverseLoader::loadArc(const QVariantMap& map,
                        const UniverseCatalog* catalog,
                        double startTime)
{
    vesta::Arc* arc = new vesta::Arc();

    QVariant centerData = map.value("center");
    QVariant trajectoryData = map.value("trajectory");
    QVariant rotationModelData = map.value("rotationModel");
    QVariant trajectoryFrameData = map.value("trajectoryFrame");
    QVariant bodyFrameData = map.value("bodyFrame");

    if (centerData.type() == QVariant::String)
    {
        QString centerName = centerData.toString();
        arc->setCenter(catalog->find(centerName));
    }
    else
    {
        errorMessage("Missing center for object.");
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
            arc->setBodyFrame(frame);
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

    QVariant endTimeVar = map.value("endTime");
    double endTime = DefaultEndTime;
    if (endTimeVar.isValid())
    {
        bool ok = false;
        endTime = dateValue(endTimeVar, &ok);
        if (!ok)
        {
            errorMessage("Invalid endTime specified.");
            delete arc;
            return NULL;
        }
    }

    if (endTime <= startTime)
    {
        errorMessage("End time must be after the start time");
        delete arc;
        return NULL;
    }

    arc->setDuration(endTime - startTime);

    return arc;
}


QList<counted_ptr<vesta::Arc> >
UniverseLoader::loadChronology(const QVariantList& list,
                               const UniverseCatalog* catalog,
                               double startTime)
{
    QList<counted_ptr<vesta::Arc> > arcs;
    double nextStartTime = startTime;

    foreach (QVariant v, list)
    {
        if (v.type() != QVariant::Map)
        {
            errorMessage("Invalid arc in arcs list.");
            arcs.clear();
            break;
        }

        QVariantMap map = v.toMap();
        vesta::Arc* arc = loadArc(map, catalog, nextStartTime);
        if (!arc)
        {
            arcs.clear();
            break;
        }

        nextStartTime += arc->duration();

        arcs << counted_ptr<vesta::Arc>(arc);
    }

    return arcs;
}


static TiledMap*
loadTiledMap(const QVariantMap& map, PathRelativeTextureLoader* textureLoader)
{
    QString type = map.value("type").toString();
    if (type == "WMS")
    {
#ifdef VESTA_OGLES2
        return NULL;
#else
        QVariant layerVar = map.value("layer");
        QVariant levelCountVar = map.value("levelCount");
        QVariant tileSizeVar = map.value("tileSize");

        if (layerVar.type() != QVariant::String)
        {
            qDebug() << "Bad or missing layer name for WMS tiled texture";
            return NULL;
        }

        if (!levelCountVar.canConvert(QVariant::Int))
        {
            qDebug() << "Bad or missing level count for WMS tiled texture";
            return NULL;
        }

        if (!tileSizeVar.canConvert(QVariant::Int))
        {
            qDebug() << "Bad or missing tileSize for WMS tiled texture";
            return NULL;
        }

        QString layer = layerVar.toString();
        int levelCount = levelCountVar.toInt();
        int tileSize = tileSizeVar.toInt();

        // Enforce some limits on tile size and level count
        levelCount = std::max(1, std::min(16, levelCount));
        tileSize = std::max(128, std::min(8192, tileSize));
        
        return new WMSTiledMap(textureLoader, layer, tileSize, levelCount);
#endif
    }
    else if (type == "MultiWMS")
    {
#ifdef VESTA_OGLES2
        return NULL;
#else
        QVariant baseLayerVar = map.value("baseLayer");
        QVariant baseLevelCountVar = map.value("baseLevelCount");
        QVariant detailLayerVar = map.value("detailLayer");
        QVariant detailLevelCountVar = map.value("detailLevelCount");
        QVariant tileSizeVar = map.value("tileSize");
        QVariant topLayerVar = map.value("topLayer");

        QString baseLayer;
        QString detailLayer;
        int baseLevelCount = 0;
        int detailLevelCount = 0;

        if (baseLayerVar.type() != QVariant::String)
        {
            qDebug() << "Bad or missing base layer name for MultiWMS tiled texture";
            return NULL;
        }
        else
        {
            baseLayer = baseLayerVar.toString();
        }

        if (!baseLevelCountVar.canConvert(QVariant::Int))
        {
            qDebug() << "Bad or missing base level count for MultiWMS tiled texture";
            return NULL;
        }
        else
        {
            baseLevelCount = baseLevelCountVar.toInt();
        }

        if (detailLayerVar.isValid())
        {
            if (detailLayerVar.type() != QVariant::String)
            {
                qDebug() << "Bad detail layer name for MultiWMS tiled texture";
                return NULL;
            }
        }

        // Detail level count is only required when the detail layer name is present
        if (!detailLayer.isEmpty())
        {
            if (!detailLevelCountVar.canConvert(QVariant::Int))
            {
                qDebug() << "Bad or missing detail level count for MultiWMS tiled texture";
                return NULL;
            }
            else
            {
                detailLevelCount = detailLevelCountVar.toInt();
            }
        }

        if (!tileSizeVar.canConvert(QVariant::Int))
        {
            qDebug() << "Bad or missing tileSize for MultiWMS tiled texture";
            return NULL;
        }

        QString topLayer;
        if (topLayerVar.type() == QVariant::String)
        {
            topLayer = topLayerVar.toString();
        }

        int tileSize = tileSizeVar.toInt();

        // Enforce some limits on tile size and level count
        baseLevelCount = std::max(1, std::min(16, baseLevelCount));
        detailLevelCount = std::max(baseLevelCount, std::min(16, detailLevelCount));
        tileSize = std::max(128, std::min(8192, tileSize));

        return new MultiWMSTiledMap(textureLoader, topLayer, baseLayer, baseLevelCount, detailLayer, detailLevelCount, tileSize);
#endif
    }
    else if (type == "NameTemplate")
    {
        QVariant templateNameVar = map.value("template");
        QVariant tileSizeVar = map.value("tileSize");
        QVariant levelCountVar = map.value("levelCount");
        QVariant borderThicknessVar = map.value("tileBorderThickness");

        if (!templateNameVar.isValid())
        {
            qDebug() << "Missing template for NameTemplate tiled texture";
            return NULL;
        }

        if (!tileSizeVar.canConvert(QVariant::UInt))
        {
            qDebug() << "Bad or missing tileSize for NameTemplate tiled texture";
            return NULL;
        }

        if (!levelCountVar.canConvert(QVariant::UInt))
        {
            qDebug() << "Bad or missing level count for NameTemplate tiled texture";
            return NULL;
        }

        float borderThickness = 0.0f;
        if (borderThicknessVar.isValid())
        {
            bool ok = false;
            borderThickness = borderThicknessVar.toFloat(&ok);
            if (!ok)
            {
                qDebug() << "NameTemplate tiled texture has invalid border thickness.";
                return NULL;
            }
        }
        // Enforce some limits on tile size and level count
        unsigned int levelCount = std::max(1u, std::min(16u, levelCountVar.toUInt()));
        unsigned int tileSize = std::max(128u, std::min(8192u, tileSizeVar.toUInt()));

        // Adjust tile size to improve sharpness. Reporting a smaller tile size
        // means that transitions will occur earlier
        tileSize = (tileSize * 3) / 5;

        QString templateName = templateNameVar.toString();
        templateName = QString::fromUtf8(textureLoader->searchPath().c_str()) + QString("/") + templateName;

        NameTemplateTiledMap* tiledMap = new NameTemplateTiledMap(textureLoader, templateName.toUtf8().data(), tileSize, levelCount);
        tiledMap->setTileBorderFraction(borderThickness);
        if (templateName.toLower().endsWith(".dds") || templateName.toLower().endsWith(".dxt5nm"))
        {
            tiledMap->setTextureUsage(TextureProperties::CompressedNormalMap);
        }

        return tiledMap;
    }
    else
    {
        qDebug() << "Unknown tiled map type.";
        return NULL;
    }
}


Geometry*
UniverseLoader::loadMeshFile(const QString& fileName)
{
    Geometry* geometry = NULL;

    // Check the cache first
    if (m_geometryCache.contains(fileName))
    {
        geometry = m_geometryCache.find(fileName)->ptr();
    }
    else
    {
        // Set the texture loader path to search in the model file's directory for texture files
        // except when loading SSC files, when the texturesInModelDirectory property will be false.
        QFileInfo info(fileName);
        QString savedPath = QString::fromUtf8(m_textureLoader->searchPath().c_str());
        if (m_texturesInModelDirectory)
        {
            m_textureLoader->setSearchPath(info.absolutePath().toUtf8().data());
        }

        MeshGeometry* meshGeometry = NULL;
        if (fileName.toLower().endsWith(".cmod"))
        {
            QFile cmodFile(fileName);
            if (!cmodFile.open(QIODevice::ReadOnly))
            {
                errorMessage(QString("Error opening cmod file '%1'").arg(fileName));
            }
            else
            {
                CmodLoader loader(&cmodFile, m_textureLoader.ptr());
                meshGeometry = loader.loadMesh();
                if (loader.error())
                {
                    errorMessage(QString("Error loading cmod file %1: %2").arg(fileName, loader.errorMessage()));
                }
            }
        }
        else
        {
            meshGeometry = MeshGeometry::loadFromFile(fileName.toUtf8().data(), m_textureLoader.ptr());
        }

        if (meshGeometry)
        {
            // Optimize the mesh. The optimizations can be expensive for large meshes, but they can dramatically
            // improve rendering performance. The best solution is to use mesh files that are already optimized, but
            // the average model loaded off the web benefits from some preprocessing at load time.
            meshGeometry->mergeSubmeshes();
            meshGeometry->uniquifyVertices();
            meshGeometry->mergeMaterials();
            meshGeometry->compressIndices();
            m_geometryCache.insert(fileName, vesta::counted_ptr<Geometry>(meshGeometry));
            geometry = meshGeometry;
        }

        m_textureLoader->setSearchPath(savedPath.toUtf8().data());
    }

    return geometry;
}


PlanetaryRings*
UniverseLoader::loadRingSystemGeometry(const QVariantMap& map)
{
    QVariant innerRadiusVar = map.value("innerRadius");
    QVariant outerRadiusVar = map.value("outerRadius");
    QVariant textureVar = map.value("texture");

    if (!innerRadiusVar.isValid())
    {
        errorMessage("innerRadius missing for ring system");
        return NULL;
    }

    if (!outerRadiusVar.isValid())
    {
        errorMessage("outerRadius missing for ring system");
        return NULL;
    }

    if (!textureVar.isValid())
    {
        errorMessage("texture missing for ring system");
        return NULL;
    }

    bool ok = false;
    double innerRadius = distanceValue(innerRadiusVar, Unit_Kilometer, 1.0, &ok);
    if (!ok)
    {
        errorMessage("Bad value for inner radius of ring system");
        return NULL;
    }

    double outerRadius = distanceValue(outerRadiusVar, Unit_Kilometer, 1.0, &ok);
    if (!ok)
    {
        errorMessage("Bad value for outer radius of ring system");
        return NULL;
    }

    // The rings texture should be oriented so that its horizontal axis is
    // the radial direction. We thus wrap vertically (t), but clamp horizontally (s).
    TextureProperties ringTextureProps;
    ringTextureProps.addressS = TextureProperties::Clamp;
    ringTextureProps.addressT = TextureProperties::Wrap;

    PlanetaryRings* ringSystem = new PlanetaryRings(innerRadius, outerRadius);
    if (m_textureLoader.isValid())
    {
        QString textureName = textureVar.toString();
        TextureMap* ringTexture = m_textureLoader->loadTexture(textureName.toUtf8().data(), ringTextureProps);
        ringSystem->setTexture(ringTexture);
    }

    return ringSystem;
}


Geometry*
UniverseLoader::loadGlobeGeometry(const QVariantMap& map)
{
    Vector3d radii = Vector3d::Zero();

    QVariant radiusVar = map.value("radius");
    if (radiusVar.type() != QVariant::Invalid)
    {
        double r = distanceValue(radiusVar, Unit_Kilometer, 1.0);
        radii = Vector3d::Constant(r);
    }
    else if (map.contains("radii"))
    {
        bool ok = false;
        radii = vec3Value(map.value("radii"), &ok);
        if (!ok)
        {
            errorMessage("Invalid radii given for globe geometry.");
            return NULL;
        }
    }

    WorldGeometry* world = new WorldGeometry();
    world->setEllipsoid(radii.cast<float>() * 2.0f);

    TextureProperties props;
    props.addressS = TextureProperties::Wrap;
    props.addressT = TextureProperties::Clamp;

    QVariant baseMapVar = map.value("baseMap");
    if (baseMapVar.type() == QVariant::String)
    {
        QString baseMapName = baseMapVar.toString();
        if (m_textureLoader.isValid())
        {
            TextureMap* tex = m_textureLoader->loadTexture(baseMapName.toUtf8().data(), props);
            world->setBaseMap(tex);
        }
    }
    else if (baseMapVar.type() == QVariant::Map)
    {
        TiledMap* tiledMap = loadTiledMap(baseMapVar.toMap(), m_textureLoader.ptr());
        if (tiledMap)
        {
            world->setBaseMap(tiledMap);
        }
    }

    QVariant normalMapVar = map.value("normalMap");
    if (normalMapVar.type() == QVariant::String)
    {

        TextureProperties normalMapProps;
        normalMapProps.addressS = TextureProperties::Wrap;
        normalMapProps.addressT = TextureProperties::Clamp;
        normalMapProps.usage = TextureProperties::CompressedNormalMap;

        QString normalMapBase = normalMapVar.toString();
        if (m_textureLoader.isValid())
        {
            TextureMap* normalTex = m_textureLoader->loadTexture(normalMapBase.toUtf8().data(), normalMapProps);
            world->setNormalMap(normalTex);
        }
    }
    else if (normalMapVar.type() == QVariant::Map)
    {
        TiledMap* tiledMap = loadTiledMap(normalMapVar.toMap(), m_textureLoader.ptr());
        if (tiledMap)
        {
            world->setNormalMap(tiledMap);
        }
    }

    QVariant emissiveVar = map.value("emissive");
    if (emissiveVar.type() == QVariant::Bool)
    {
        world->setEmissive(emissiveVar.toBool());
    }

    // Specular color and power (mainly used for ocean reflections)
    QVariant specularColorVar = map.value("specularColor");
    QVariant specularPowerVar = map.value("specularPower");
    float specularPower = 0.0f;
    Spectrum specularColor = Spectrum::Black();

    if (specularPowerVar.isValid())
    {
        if (specularPowerVar.canConvert(QVariant::Double))
        {
            specularPower = specularPowerVar.toFloat();
        }
        else
        {
            errorMessage("Invalid specular power given for globe geometry.");
            delete world;
            return NULL;
        }
    }

    if (specularColorVar.isValid())
    {
        specularColor = colorValue(specularColorVar, Spectrum(1.0f, 1.0f, 1.0f));
    }

    if (!(specularColor == Spectrum::Black()) && specularPower > 0.0f)
    {
        world->setSpecularReflectance(specularColor);
        world->setSpecularPower(specularPower);
    }

    QVariant cloudMapVar = map.value("cloudMap");
    if (cloudMapVar.isValid() && m_textureLoader.isValid())
    {
        if (cloudMapVar.type() == QVariant::String)
        {
            TextureProperties cloudMapProps;
            cloudMapProps.addressS = TextureProperties::Wrap;
            cloudMapProps.addressT = TextureProperties::Clamp;

            QString cloudMapName = cloudMapVar.toString();
            TextureMap* cloudTex = m_textureLoader->loadTexture(cloudMapName.toUtf8().data(), cloudMapProps);
            world->setCloudMap(cloudTex);
        }
        else if (cloudMapVar.type() == QVariant::Map)
        {
            TiledMap* tiledMap = loadTiledMap(cloudMapVar.toMap(), m_textureLoader.ptr());
            if (tiledMap)
            {
#ifdef VESTA_OGLES2
                // TODO: Enable support in desktop version
                world->setCloudMap(tiledMap);
#endif
            }
        }

        world->setCloudAltitude(6.0f);
    }

    QVariant atmosphereVar = map.value("atmosphere");
    if (atmosphereVar.type() == QVariant::String)
    {
        QString fileName = dataFileName(atmosphereVar.toString());
        QFile atmFile(fileName);
        if (atmFile.open(QIODevice::ReadOnly))
        {
            QByteArray data = atmFile.readAll();
            DataChunk chunk(data.data(), data.size());
            Atmosphere* atm = Atmosphere::LoadAtmScat(&chunk);
            if (atm)
            {
                atm->generateTextures();
                atm->addRef();
                world->setAtmosphere(atm);
            }
        }
    }

    QVariant ringsVar = map.value("ringSystem");
    if (ringsVar.isValid())
    {
        if (ringsVar.type() == QVariant::Map)
        {
            PlanetaryRings* ringSystem = loadRingSystemGeometry(ringsVar.toMap());
            world->setRingSystem(ringSystem);
        }
        else
        {
            errorMessage("Error in definition of ringSystem");
        }
    }

    return world;
}


Geometry*
UniverseLoader::loadMeshGeometry(const QVariantMap& map)
{
    // We permit two methods of scaling the mesh:
    //    1. Specifying the size will scale the mesh to fit in a sphere of that size
    //    2. Specifying scale will apply a scaling factor
    //
    // scale overrides size when it's present. If neither size nor scale is given, a default
    // scale of 1.0 is used.
    double radius = distanceValue(map.value("size"), Unit_Kilometer, 0.0);
    double scale = doubleValue(map.value("scale"), 1.0);
    Quaternionf meshRotation(Quaternionf::Identity());
    Vector3f meshOffset(Vector3f::Zero());

    QVariant meshRotationVar = map.value("meshRotation");
    if (meshRotationVar.isValid())
    {
        bool ok = false;
        meshRotation = quaternionValue(meshRotationVar, &ok).cast<float>();
        if (!ok)
        {
            errorMessage("Invalid quaternion given for meshRotation");
            return NULL;
        }
    }

    QVariant meshOffsetVar = map.value("meshOffset");
    if (meshOffsetVar.isValid())
    {
        bool ok = false;
        meshOffset = vec3Value(meshOffsetVar, &ok).cast<float>();
        if (!ok)
        {
            errorMessage("Invalid vector given for meshOffset");
            return NULL;
        }
    }

    MeshInstanceGeometry* meshInstance = NULL;
    if (map.contains("source"))
    {
        QString sourceName = map.value("source").toString();
        MeshGeometry* mesh = dynamic_cast<MeshGeometry*>(loadMeshFile(modelFileName(sourceName)));

        if (mesh)
        {
            meshInstance = new MeshInstanceGeometry(mesh);
            if (radius > 0.0)
            {
                float maxExtent = mesh->meshBoundingBox().extents().maxCoeff();
                meshInstance->setScale(radius * 2.0f / maxExtent);
            }
            else
            {
                meshInstance->setScale(float(scale));
            }
            meshInstance->setMeshRotation(meshRotation);
            meshInstance->setMeshOffset(meshOffset);
        }
    }

    return meshInstance;
}


Geometry*
UniverseLoader::loadSensorGeometry(const QVariantMap& map, const UniverseCatalog* catalog)
{
    QVariant targetVar = map.value("target");
    QVariant rangeVar = map.value("range");
    QVariant shapeVar = map.value("shape");
    QVariant horizontalFovVar = map.value("horizontalFov");
    QVariant verticalFovVar = map.value("verticalFov");
    QVariant frustumColorVar = map.value("frustumColor");
    QVariant frustumBaseColorVar = map.value("frustumBaseColor");
    QVariant frustumOpacityVar = map.value("frustumOpacity");
    QVariant gridOpacityVar = map.value("gridOpacity");
    QVariant orientationVar = map.value("orientation");

    if (targetVar.type() != QVariant::String)
    {
        errorMessage("Bad or missing target for sensor geometry");
        return NULL;
    }

    if (!rangeVar.canConvert(QVariant::Double))
    {
        errorMessage("Bad or missing range for sensor geometry");
        return NULL;
    }

    double range = distanceValue(rangeVar, Unit_Kilometer, 1.0);
    QString shape = shapeVar.toString();
    double horizontalFov = angleValue(horizontalFovVar, 5.0);
    double verticalFov = angleValue(verticalFovVar, 5.0);
    Spectrum frustumColor = colorValue(frustumColorVar, Spectrum(1.0f, 1.0f, 1.0f));
    //Spectrum frustumBaseColor = colorValue(frustumColorVar, Spectrum(1.0f, 1.0f, 1.0f));
    double frustumOpacity = doubleValue(frustumOpacityVar, 0.3);
    //double gridOpacity = doubleValue(gridOpacityVar, 0.15);
    Quaterniond orientation = Quaterniond::Identity();

    if (orientationVar.isValid())
    {
        bool ok = false;
        orientation = quaternionValue(orientationVar, &ok);
        if (!ok)
        {
            errorMessage("Bad orientation given for sensor geometry");
            return NULL;
        }
    }

    Entity* target = catalog->find(targetVar.toString());
    if (!target)
    {
        errorMessage("Target for sensor geometry not found");
        return NULL;
    }

    SensorFrustumGeometry* sensorFrustum = new SensorFrustumGeometry();
    sensorFrustum->setTarget(target);
    sensorFrustum->setColor(frustumColor);
    sensorFrustum->setOpacity(frustumOpacity);
    sensorFrustum->setRange(range);
    sensorFrustum->setFrustumAngles(horizontalFov, verticalFov);
    sensorFrustum->setSensorOrientation(orientation);

    if (shape == "elliptical")
    {
        sensorFrustum->setFrustumShape(SensorFrustumGeometry::Elliptical);
    }
    else if (shape == "rectangular")
    {
        sensorFrustum->setFrustumShape(SensorFrustumGeometry::Rectangular);
    }

    sensorFrustum->setSource(catalog->find(m_currentBodyName));

    return sensorFrustum;
}


static vesta::ArrowGeometry*
loadAxesGeometry(const QVariantMap& map)
{
    ArrowGeometry* axes = new ArrowGeometry(1.0f, 0.005f, 0.05f, 0.01f);
    axes->setVisibleArrows(ArrowGeometry::AllAxes);
    axes->setScale(float(doubleValue(map.value("scale"), 1.0)));

    return axes;
}


Geometry*
UniverseLoader::loadSwarmGeometry(const QVariantMap& map)
{
    QVariant sourceVar       = map.value("source");
    QVariant formatVar       = map.value("format");
    QVariant particleSizeVar = map.value("particleSize");
    QVariant colorVar        = map.value("color");
    QVariant opacityVar      = map.value("opacity");

    if (!sourceVar.isValid())
    {
       errorMessage("Missing source for swarm geometry");
       return NULL;
    }

    if (!formatVar.isValid())
    {
        errorMessage("Missing format for swarm geometry");
        return NULL;
    }

    QString source = sourceVar.toString();
    QString format = formatVar.toString();

    float particleSize = 1.0f;
    if (particleSizeVar.isValid())
    {
        if (particleSizeVar.canConvert(QVariant::Double))
        {
            particleSize = particleSizeVar.toFloat();
        }
    }

    float fadeSize = 50.0f;
    QVariant fadeSizeVar = map.value("fadeSize");
    if (fadeSizeVar.isValid())
    {
        if (fadeSizeVar.canConvert(QVariant::Double))
        {
            fadeSize = fadeSizeVar.toFloat();
        }
    }

    float fullSizeDistance = 1.0e12f;
    QVariant fullSizeDistanceVar = map.value("fullSizeDistance");
    if (fullSizeDistanceVar.isValid())
    {
        if (fullSizeDistanceVar.canConvert(QVariant::Double))
        {
            fullSizeDistance = fullSizeDistanceVar.toFloat();
        }
    }

    Spectrum color = colorValue(colorVar, Spectrum::White());
    float opacity = float(doubleValue(opacityVar, 1.0));

    KeplerianSwarm* swarm = NULL;
    if (format == "astorb")
    {
        swarm = LoadAstorbFile(dataFileName(source));
    }
    else if (format == "binary")
    {
        swarm = LoadBinaryAstorbFile(dataFileName(source));
    }
    else if (format == "kepbin")
    {
        swarm = LoadBinaryKeplerianOrbitFile(dataFileName(source));
    }
    else
    {
        errorMessage("Unknown format for Keplerian swarm geometry.");
        return NULL;
    }

    if (swarm)
    {
        swarm->setColor(color);
        swarm->setOpacity(opacity);
        swarm->setPointSize(particleSize);
        swarm->setFadeSize(fadeSize);
        //swarm->setFullSizeDistance(fullSizeDistance);
    }

    return swarm;
}


static InitialStateGenerator*
loadStripParticleGenerator(const QVariantMap& map)
{
    // States is a list of floating point values giving the initial particle
    // states at line endpoints. The state values are arranged as follows:
    //
    // [ x[0], y[0], z[0], vx[0], vy[0], vz[0],
    //   x[1], y[1], z[1], vx[1], vy[1], vz[1],
    //   ...
    //   x[n], y[n], z[n], vx[n], vy[n], vz[n]
    // ]
    //
    // vx, vy, and vz are the components of the initial velocity

    QVariant statesVar = map.value("states");
    if (!statesVar.isValid())
    {
        qDebug() << "Missing states for strip particle generator";
        return NULL;
    }

    if (statesVar.type() != QVariant::List)
    {
        qDebug() << "Strip particles states must be a list of numbers";
        return NULL;
    }

    QVariantList statesList = statesVar.toList();
    if (statesList.size() < 12 || statesList.size() % 6 != 0)
    {
        qDebug() << "Bad number of values in states list for strip particle generator";
        return NULL;
    }

    unsigned int stateCount = statesList.size() / 6;
    std::vector<Vector3f> states;
    for (unsigned int i = 0; i < stateCount; ++i)
    {
        Vector3f position(statesList.at(i * 6 + 0).toFloat(),
                          statesList.at(i * 6 + 1).toFloat(),
                          statesList.at(i * 6 + 2).toFloat());
        Vector3f velocity(statesList.at(i * 6 + 3).toFloat(),
                          statesList.at(i * 6 + 4).toFloat(),
                          statesList.at(i * 6 + 5).toFloat());
        states.push_back(position);
        states.push_back(velocity);
    }

    return new StripParticleGenerator(states);
}


static InitialStateGenerator*
loadArcStripParticleGenerator(const QVariantMap& map)
{
    // Arcs is a list of floating point values, arranged as follows:
    //
    // [ latitude0, longitude0, radius0, speed0,
    //   latitude1, longitude1, radius1, speed1,
    //   ...
    //   latitudeN, longitudeN, radiusN, speedN
    // ]
    QVariant arcsVar = map.value("arcs");
    if (!arcsVar.isValid())
    {
        qDebug() << "Missing arcs for arc strip particle generator";
        return NULL;
    }

    if (arcsVar.type() != QVariant::List)
    {
        qDebug() << "Arc strip particles arcs must be a list of numbers";
        return NULL;
    }

    QVariantList arcsList = arcsVar.toList();
    if (arcsList.size() < 8 || arcsList.size() % 4 != 0)
    {
        qDebug() << "Bad number of values in arcs list for arc strip particle generator";
        return NULL;
    }

    unsigned int arcCount = arcsList.size() / 4;
    std::vector<Vector3f> positions;
    std::vector<float> speeds;
    for (unsigned int i = 0; i < arcCount; ++i)
    {
        float latitude = float(toRadians(arcsList.at(i * 4).toDouble()));
        float longitude = float(toRadians(arcsList.at(i * 4 + 1).toDouble()));
        float radius = arcsList.at(i * 4 + 2).toFloat();
        float speed = arcsList.at(i * 4 + 3).toFloat();

        Vector3f r = Vector3f(cos(latitude) * cos(longitude), cos(latitude) * sin(longitude), sin(latitude)) * radius;
        positions.push_back(r);
        speeds.push_back(speed);
    }

    return new ArcStripParticleGenerator(positions, speeds);
}


static InitialStateGenerator*
loadParticleStateGenerator(const QVariantMap& map)
{
    QVariant typeVar = map.value("type");
    if (!typeVar.isValid())
    {
        qDebug() << "Missing type for particle generator.";
        return NULL;
    }

    QString type = typeVar.toString();
    if (type == "Point")
    {
        bool ok = false;
        Vector3d position = vec3Value(map.value("position"), &ok);
        Vector3d velocity = vec3Value(map.value("velocity"), &ok);
        PointGenerator* generator = new PointGenerator(position.cast<float>(), velocity.cast<float>());

        return generator;
    }
    else if (type == "Box")
    {
        bool ok = false;
        Vector3d sides = vec3Value(map.value("sides"), &ok);
        Vector3d center = vec3Value(map.value("center"), &ok);
        Vector3d velocity = vec3Value(map.value("velocity"), &ok);
        BoxGenerator* generator = new BoxGenerator(sides.cast<float>(),
                                                   center.cast<float>(),
                                                   velocity.cast<float>());

        return generator;
    }
    else if (type == "Disc")
    {
        bool ok = false;
        float radius = map.value("radius").toFloat();
        Vector3d velocity = vec3Value(map.value("velocity"), &ok);

        DiscGenerator* generator = new DiscGenerator(radius, velocity.cast<float>());

        return generator;
    }
    else if (type == "Strip")
    {
        return loadStripParticleGenerator(map);
    }
    else if (type == "ArcStrip")
    {
        return loadArcStripParticleGenerator(map);
    }
    else
    {
        qDebug() << "Unknown particle generator type " << type;
        return NULL;
    }
}


static ParticleEmitter*
loadParticleEmitter(const QVariantMap& map)
{
    QVariant startTimeVar = map.value("startTime");
    QVariant endTimeVar = map.value("endTime");
    QVariant spawnRateVar = map.value("spawnRate");
    QVariant lifetimeVar = map.value("lifetime");
    QVariant startSizeVar = map.value("startSize");
    QVariant endSizeVar = map.value("endSize");
    QVariant colorsVar = map.value("colors");
    QVariant generatorVar = map.value("generator");
    QVariant velocityVariationVar = map.value("velocityVariation");
    QVariant forceVar = map.value("force");
    QVariant traceVar = map.value("trace");
    QVariant emissiveVar = map.value("emissive");
    QVariant phaseVar = map.value("phaseAsymmetry");

    // Get the required parameters: lifetime and spawn rate
    double lifetime = 0.0;
    double spawnRate = 0.0;
    if (!spawnRateVar.isValid())
    {
        qDebug() << "Spawn rate not specified for particle emitter.";
        return NULL;
    }

    if (!lifetimeVar.isValid())
    {
        qDebug() << "Lifetime not specified for particle emitter.";
        return NULL;
    }

    if (generatorVar.type() != QVariant::Map)
    {
        qDebug() << "Invalid or missing generator for particle emitter.";
        return NULL;
    }

    QVariantMap generatorMap = generatorVar.toMap();
    InitialStateGenerator* generator = loadParticleStateGenerator(generatorMap);
    if (generator == NULL)
    {
        return NULL;
    }


    lifetime = lifetimeVar.toDouble();
    spawnRate = spawnRateVar.toDouble();

    if (lifetime <= 0.0)
    {
        qDebug() << "Particle lifetime must be a positive value.";
        return NULL;
    }

    if (spawnRate <= 0.0)
    {
        qDebug() << "Particle spawn rate must be a positive value.";
        return NULL;
    }

    if (lifetime * spawnRate > 200000)
    {
        qDebug() << "200K particle per emitter rate exceeded. Reduce particle spawn rate.";
        return NULL;
    }

    ParticleEmitter* emitter = new ParticleEmitter();
    emitter->setGenerator(generator);
    emitter->setParticleLifetime(lifetime);
    emitter->setSpawnRate(spawnRate);

    bool ok = false;

    // Now parse the optional parameters
    float startSize = 0.0f;
    float endSize = 1.0f;
    startSize = float(distanceValue(startSizeVar, Unit_Kilometer, 0.0, &ok));
    endSize = float(distanceValue(endSizeVar, Unit_Kilometer, 1.0f, &ok));
    emitter->setSizeRange(startSize, endSize);

    if (startTimeVar.isValid())
    {
        emitter->setStartTime(dateValue(startTimeVar, &ok));
    }

    if (endTimeVar.isValid())
    {
        emitter->setEndTime(dateValue(endTimeVar, &ok));
    }

    if (velocityVariationVar.isValid())
    {
        emitter->setVelocityVariation(velocityVariationVar.toFloat());
    }

    if (traceVar.isValid())
    {
        emitter->setTraceLength(traceVar.toFloat());
    }

    if (forceVar.isValid())
    {
        Vector3d force = vec3Value(forceVar, &ok);
        if (ok)
        {
            emitter->setForce(force.cast<float>());
        }
    }

    // Load the color ramp. This is an array of values arranged
    // with interleaved color and opacity values, e.g.
    //   [ "#00ff00", 0.0, "#ffff80", 1.0 ]
    //
    // Up to five values are used; anything beyond that is ignored
    Spectrum colors[5];
    float opacities[5];
    colors[0] = Spectrum::White();
    opacities[0] = 1.0;
    unsigned int colorCount = 1;

    if (colorsVar.type() == QVariant::List)
    {
        QVariantList colorsList = colorsVar.toList();
        colorCount = (unsigned int) colorsList.size() / 2;
        for (unsigned int i = 0; i < colorCount; ++i)
        {
            colors[i] = colorValue(colorsList[i * 2], Spectrum::White());
            opacities[i] = float(doubleValue(colorsList[i * 2 + 1], 1.0));
        }
    }

    emitter->setColorCount(colorCount);
    for (unsigned int i = 0; i < colorCount; ++i)
    {
        emitter->setColor(i, colors[i], opacities[i]);
    }

    if (emissiveVar.type() == QVariant::Bool)
    {
        emitter->setEmissive(emissiveVar.toBool());
    }

    if (phaseVar.isValid())
    {
        float phase = phaseVar.toFloat(&ok);
        if (ok)
        {
            if (phase <= -1.0f || phase >= 1.0f)
            {
                qDebug() << "Value for phaseAsymmetry must be between -1 and 1";
            }
            else
            {
                emitter->setPhaseAsymmetry(phase);
            }
        }
    }

    return emitter;
}


Geometry*
UniverseLoader::loadParticleSystemGeometry(const QVariantMap& map)
{
    QVariant emittersVar = map.value("emitters");
    if (!emittersVar.isValid())
    {
        errorMessage("Emitters are missing from particle system");
        return NULL;
    }

    if (emittersVar.type() != QVariant::List)
    {
        errorMessage("Emitters in particle system must be an array");
        return NULL;
    }

    QVariantList emitters = emittersVar.toList();

    ParticleSystemGeometry* particles = new ParticleSystemGeometry();
    foreach (QVariant emitterVar, emitters)
    {
        if (emitterVar.type() == QVariant::Map)
        {
            QVariantMap emitterMap = emitterVar.toMap();
            QVariant textureVar = emitterMap.value("texture");

            TextureMap* texture = NULL;
            TextureProperties particleTextureProps;
            particleTextureProps.addressS = TextureProperties::Clamp;
            particleTextureProps.addressT = TextureProperties::Clamp;
            if (m_textureLoader.isValid())
            {
                QString textureName = textureVar.toString();
                texture = m_textureLoader->loadTexture(textureName.toUtf8().data(), particleTextureProps);
            }

            ParticleEmitter* emitter = loadParticleEmitter(emitterMap);
            if (emitter)
            {
                particles->addEmitter(emitter, texture);
            }
        }
        else
        {
            errorMessage("Bad emitter in particle system");
        }
    }

    return particles;
}


Geometry*
UniverseLoader::loadTimeSwitchedGeometry(const QVariantMap& map, const UniverseCatalog* catalog)
{
    QVariant sequenceVar = map.value("sequence");
    if (!sequenceVar.isValid())
    {
        errorMessage("Sequence is missing from time switched geometry");
        return NULL;
    }

    if (sequenceVar.type() != QVariant::List)
    {
        errorMessage("Sequence in time switched geometry must be an array");
        return NULL;
    }

    QVariantList sequence = sequenceVar.toList();

    TimeSwitchedGeometry* timeSwitched = new TimeSwitchedGeometry();
    foreach (QVariant stepVar, sequence)
    {
        if (stepVar.type() == QVariant::Map)
        {
            QVariantMap stepMap = stepVar.toMap();
            QVariant geometryVar = stepMap.value("geometry");
            QVariant startTimeVar = stepMap.value("startTime");
            double startTime = 0.0;

            if (!startTimeVar.isValid())
            {
                errorMessage("Step in time switched geometry is missing startTime");
                delete timeSwitched;
                return NULL;
            }
            else
            {
                bool ok = false;
                startTime = dateValue(startTimeVar, &ok);
                if (!ok)
                {
                    errorMessage("Invalid startTime specified in time switched geometry");
                    delete timeSwitched;
                    return NULL;
                }
            }

            Geometry* geometry = NULL;
            if (geometryVar.type() == QVariant::Map)
            {
                geometry = loadGeometry(geometryVar.toMap(), catalog);
                if (geometry == NULL)
                {
                    delete timeSwitched;
                    return NULL;
                }
            }

            timeSwitched->addGeometry(startTime, geometry);
        }
        else
        {
            errorMessage("Bad emitter in particle system");
        }
    }

    return timeSwitched;
}


Geometry*
UniverseLoader::loadGeometry(const QVariantMap& map, const UniverseCatalog* catalog)
{
    Geometry* geometry = NULL;

    QVariant typeValue = map.value("type");
    if (typeValue.type() != QVariant::String)
    {
        qDebug() << "Bad or missing type for geometry.";
        return NULL;
    }

    QString type = typeValue.toString();

    if (type == "Globe")
    {
        geometry = loadGlobeGeometry(map);
    }
    else if (type == "Mesh")
    {
        geometry = loadMeshGeometry(map);
    }
    else if (type == "Axes")
    {
        geometry = loadAxesGeometry(map);
    }
    else if (type == "Sensor")
    {
        geometry = loadSensorGeometry(map, catalog);
    }
    else if (type == "KeplerianSwarm")
    {
        geometry = loadSwarmGeometry(map);
    }
    else if (type == "ParticleSystem")
    {
        geometry = loadParticleSystemGeometry(map);
    }
    else if (type == "Rings")
    {
        geometry = loadRingSystemGeometry(map);
    }
    else if (type == "TimeSwitched")
    {
        geometry = loadTimeSwitchedGeometry(map, catalog);
    }
    else
    {
        errorMessage(QString("Unknown type '%1' for geometry.").arg(type));
    }

    return geometry;
}


Visualizer*
loadBodyAxesVisualizer(const QVariantMap& map)
{
    bool ok = false;
    double size = map.value("size", 1.0).toDouble(&ok);
    if (!ok)
    {
        qDebug() << "Bad size given for BodyAxes visualizer";
        return NULL;
    }
    else
    {
        return new AxesVisualizer(AxesVisualizer::BodyAxes, size);
    }
}


Visualizer*
loadFrameAxesVisualizer(const QVariantMap& map)
{
    bool ok = false;
    double size = map.value("size", 1.0).toDouble(&ok);
    if (!ok)
    {
        qDebug() << "Bad size given for FrameAxes visualizer";
        return NULL;
    }
    else
    {
        AxesVisualizer* axes = new AxesVisualizer(AxesVisualizer::FrameAxes, size);
        axes->arrows()->setOpacity(0.3f);
        return axes;
    }
}


Visualizer*
loadBodyDirectionVisualizer(const QVariantMap& map,
                            const UniverseCatalog* catalog)
{
    bool ok = false;
    double size = map.value("size", 1.0).toDouble(&ok);
    if (!ok)
    {
        qDebug() << "Bad size given for FrameAxes visualizer";
        return NULL;
    }

    QVariant targetVar = map.value("target");
    QVariant colorVar = map.value("color");
    Spectrum color = colorValue(colorVar, Spectrum::White());

    if (targetVar.type() != QVariant::String)
    {
        qDebug() << "Bad or missing target for BodyDirection visualizer";
        return NULL;
    }

    Entity* target = catalog->find(targetVar.toString());
    if (!target)
    {
        qDebug() << "Target body " << targetVar.toString() << " for BodyDirection visualizer not found";
        return NULL;
    }

    BodyDirectionVisualizer* direction = new BodyDirectionVisualizer(size, target);
    direction->setColor(color);

    return direction;
}


Visualizer*
UniverseLoader::loadPlaneVisualizer(const QVariantMap& style,
                                    const UniverseCatalog* catalog)
{
    bool ok = false;
    double size = style.value("size", 1.0).toDouble(&ok);
    if (!ok)
    {
        errorMessage("Bad size given for Plane visualizer");
        return NULL;
    }

    QVariant colorVar = style.value("color");
    QVariant gridSubdivisionVar = style.value("gridSubdivision");
    QVariant frameVar = style.value("frame");
    QVariant opacityVar = style.value("opacity");

    vesta::Frame* frame = InertialFrame::equatorJ2000();
    if (frameVar.isValid())
    {
        if (frameVar.type() == QVariant::String)
        {
            // Inertial frame name
            frame = loadInertialFrame(frameVar.toString());
        }
        else if (frameVar.type() == QVariant::Map)
        {
            frame = loadFrame(frameVar.toMap(), catalog);
        }
        else
        {
            frame = NULL;
            errorMessage("Invalid frame given for ConstantVector");
        }

        if (!frame)
        {
            return NULL;
        }
    }

    unsigned int gridSubdivision = 10;
    if (gridSubdivisionVar.isValid())
    {
        gridSubdivision = gridSubdivisionVar.toUInt(&ok);
        if (!ok)
        {
            errorMessage("gridSubdivision for plane visualizer must be a non-negative integer");
            return NULL;
        }
    }

    double gridSpacing = 0.0;
    if (gridSubdivision > 0)
    {
        gridSpacing = (size * 2.0) / gridSubdivision;
    }

    Spectrum color = colorValue(colorVar, Spectrum::White());
    float opacity = float(doubleValue(opacityVar, 0.2));

    PlaneVisualizer* visualizer = new PlaneVisualizer(size);
    visualizer->plane()->setGridLineSpacing(gridSpacing);
    visualizer->plane()->setColor(color);
    visualizer->plane()->setOpacity(opacity);
    visualizer->setFrame(frame);

    return visualizer;
}


Visualizer*
UniverseLoader::loadVisualizer(const QVariantMap& map,
                               const UniverseCatalog* catalog)
{
    QVariant styleVar = map.value("style");
    if (styleVar.type() != QVariant::Map)
    {
        errorMessage("Missing visualizer style.");
        return NULL;
    }

    QVariantMap style = styleVar.toMap();
    QVariant typeVar = style.value("type");
    if (typeVar.type() != QVariant::String)
    {
        errorMessage("Bad or missing type for visualizer style.");
        return NULL;
    }

    QString type = typeVar.toString();
    if (type == "BodyAxes")
    {
        return loadBodyAxesVisualizer(style);
    }
    else if (type == "FrameAxes")
    {
        return loadFrameAxesVisualizer(style);
    }
    else if (type == "BodyDirection")
    {
        return loadBodyDirectionVisualizer(style, catalog);
    }
    else if (type == "Plane")
    {
        return loadPlaneVisualizer(style, catalog);
    }
    else
    {
        errorMessage(QString("Unknown visualizer type '%1'").arg(type));
        return NULL;
    }
}

#if 0
static Vector3d
planetographicToRectangular(const AlignedEllipsoid& e, const PlanetographicCoord3& c)
{
    // Compute the planetographic normal
    Vector3d n(cos(c.latitude()) * cos(c.longitude()),
               cos(c.latitude()) * sin(c.longitude()),
               sin(c.latitude()));

    Vector3d k = e.semiAxes().cwise().square().cwise() * n;
    double s = sqrt(k.dot(n));

    Vector3d surfacePoint = k / s;
    return surfacePoint + c.height() * n;
}


static Vector3d
planetocentricToRectangular(const AlignedEllipsoid& e, const PlanetographicCoord3& c)
{
    Vector3d d(cos(c.latitude()) * cos(c.longitude()),
               cos(c.latitude()) * sin(c.longitude()),
               sin(c.latitude()));

    return e.semiAxes().cwise() * d + c.height() * d;
}
#endif


Visualizer*
UniverseLoader::loadFeatureLabels(const QVariantMap& map,
                                  const Entity* body)
{
    QVariant featuresVar = map.value("features");

    if (featuresVar.type() != QVariant::List)
    {
        errorMessage("Features list in FeatureLabels item is missing or invalid.");
        return NULL;
    }

    if (!body->geometry() || !body->geometry()->isEllipsoidal())
    {
        return NULL;
    }

    // Get the spin axis at the J2000.0 epoch; use this to determine whether the body
    // is a retrograde rotator.
    Vector3d spinAxisEcl = InertialFrame::eclipticJ2000()->orientation().conjugate() * body->orientation(0.0) * Vector3d::UnitZ();
    bool isRetrogradeRotator = spinAxisEcl.z() < 0.0;

    AlignedEllipsoid ellipsoid = body->geometry()->ellipsoid();

    counted_ptr<FeatureLabelSetGeometry> featureLabelSet(new FeatureLabelSetGeometry());
    featureLabelSet->setOccluder(ellipsoid);

    QVariantList featuresList = featuresVar.toList();
    foreach (QVariant featureVar, featuresList)
    {
        if (featureVar.type() != QVariant::Map)
        {
            errorMessage("Bad feature in FeatureLabels list");
            return NULL;
        }

        QVariantMap feature = featureVar.toMap();

        QVariant nameVar = feature.value("name");
        if (nameVar.type() != QVariant::String)
        {
            errorMessage("Bad or missing name for feature");
            return NULL;
        }

        std::string name(nameVar.toString().toUtf8().data());

        bool ok = false;
        double longitude = feature.value("longitude").toDouble(&ok);
        if (!ok)
        {
            errorMessage("Bad or missing longitude for feature");
            return NULL;
        }

        double latitude = feature.value("latitude").toDouble(&ok);
        if (!ok)
        {
            errorMessage("Bad or missing latitude for feature");
            return NULL;
        }

        double diameter = distanceValue(feature.value("diameter"), Unit_Kilometer, 0.0, &ok);
        if (!ok)
        {
            errorMessage("Bad or missing diameter for feature");
            return NULL;
        }

        // Reverse coordinates for retrograde rotators: the IAU coordinate systems for planets
        // and moons use ecliptic north, while Cosmographia uses rotational north.
        if (isRetrogradeRotator)
        {
            longitude = -longitude;
            latitude = -latitude;
        }

        longitude = toRadians(longitude);
        latitude = toRadians(latitude);

        PlanetographicCoord3 position(latitude, longitude, 0.0);
        Vector3d rectPosition = ellipsoid.planetographicToRectangular(position);

        featureLabelSet->addFeature(name, rectPosition.cast<float>(), float(diameter / 2.0), Spectrum(1.0f, 1.0f, 0.85f));
    }

    return new LocalVisualizer(featureLabelSet.ptr());
}


Viewpoint*
UniverseLoader::loadViewpoint(const QVariantMap& map,
                              UniverseCatalog* catalog)
{
    QVariant nameVar = map.value("name");
    QVariant centerVar = map.value("center");
    QVariant referenceVar = map.value("reference");
    QVariant altitudeVar = map.value("altitude");
    QVariant azimuthVar = map.value("azimuth");
    QVariant elevationVar = map.value("elevation");
    QVariant upVar = map.value("up");

    if (!nameVar.isValid())
    {
        errorMessage("Viewpoint is missing name");
        return NULL;
    }

    if (!centerVar.isValid())
    {
        errorMessage("Viewpoint is missing center body");
        return NULL;
    }

    if (!referenceVar.isValid())
    {
        errorMessage("Viewpoint is missing reference body");
        return NULL;
    }

    if (!altitudeVar.isValid() || !altitudeVar.canConvert(QVariant::Double))
    {
        errorMessage("Bad or missing altitude for viewpoint.");
        return NULL;
    }

    Viewpoint::UpVectorDirection up = Viewpoint::CenterNorth;
    if (upVar.isValid())
    {
        QString ups = upVar.toString();
        if (ups == "CenterNorth")
        {
            up = Viewpoint::CenterNorth;
        }
        else if (ups == "CenterSouth")
        {
            up = Viewpoint::CenterSouth;
        }
        else if (ups == "EclipticNorth")
        {
            up = Viewpoint::EclipticNorth;
        }
        else if (ups == "EclipticSouth")
        {
            up = Viewpoint::EclipticSouth;
        }
    }

    double azimuth = 0.0;
    if (azimuthVar.isValid())
    {
        if (!altitudeVar.canConvert(QVariant::Double))
        {
           errorMessage("Bad azimuth given for viewpoint");
           return NULL;
        }
        else
        {
            azimuth = azimuthVar.toDouble();
        }
    }

    double elevation = 0.0;
    if (elevationVar.isValid())
    {
        if (!elevationVar.canConvert(QVariant::Double))
        {
           errorMessage("Bad elevation given for viewpoint.");
           return NULL;
        }
        else
        {
            elevation = elevationVar.toDouble();
        }
    }

    Entity* center = catalog->find(centerVar.toString());
    Entity* referenceBody = catalog->find(referenceVar.toString());

    if (!center)
    {
        errorMessage(QString("Unknown center body '%1' for viewpoint").arg(centerVar.toString()));
        return NULL;
    }

    if (!referenceBody)
    {
        errorMessage(QString("Unknown reference body '%1' for viewpoint").arg(referenceVar.toString()));
        return NULL;
    }

    // Convert altitude to distance when the center object is an ellipsoid
    double distance = altitudeVar.toDouble();
    if (center->geometry() && center->geometry()->isEllipsoidal())
    {
        distance += center->geometry()->ellipsoid().semiMajorAxisLength();
    }

    Viewpoint* viewpoint = new Viewpoint(center, distance);
    viewpoint->setReferenceBody(referenceBody);
    viewpoint->setAzimuth(azimuth);
    viewpoint->setElevation(elevation);
    viewpoint->setName(nameVar.toString().toUtf8().constData());
    viewpoint->setUpDirection(up);

    return viewpoint;
}


void
loadTrajectoryPlotInfo(BodyInfo* info,
                       const QVariantMap& plot)
{
    QVariant colorVar = plot.value("color");
    QVariant durationVar = plot.value("duration");
    QVariant sampleCountVar = plot.value("sampleCount");
    QVariant fadeVar = plot.value("fade");
    QVariant leadVar = plot.value("lead");

    if (sampleCountVar.canConvert(QVariant::Int))
    {
        int count = sampleCountVar.toInt();
        info->trajectoryPlotSamples = (unsigned int) std::max(100, std::min(50000, count));
    }

    bool ok = false;

    double duration = durationValue(durationVar, Unit_Day, 0.0, &ok);
    if (duration != 0.0)
    {
        info->trajectoryPlotDuration = duration;
    }

    if (leadVar.isValid())
    {
        info->trajectoryPlotLead = durationValue(leadVar, Unit_Day, 0.0, &ok);
    }

    if (fadeVar.canConvert(QVariant::Double))
    {
        info->trajectoryPlotFade = std::max(0.0, std::min(1.0, fadeVar.toDouble()));
    }

    if (colorVar.isValid())
    {
        info->trajectoryPlotColor = colorValue(colorVar, Spectrum::White());
    }
}


void
loadLabelInfo(BodyInfo* info, const QVariantMap& map)
{
    QVariant colorVar = map.value("color");
    QVariant labelFadeSizeVar = map.value("fadeSize");
    QVariant showTextVar = map.value("showText");

    if (colorVar.isValid())
    {
        info->labelColor = colorValue(colorVar, Spectrum::White());
    }

    if (labelFadeSizeVar.isValid())
    {
        info->labelFadeSize = doubleValue(labelFadeSizeVar, 0.0);
    }

    if (showTextVar.isValid())
    {
        info->labelTextVisible = showTextVar.toBool();
    }
}


/** Load additional information about a body.
  */
BodyInfo*
UniverseLoader::loadBodyInfo(const QVariantMap& item)
{
    BodyInfo* info = new BodyInfo();

    QVariant classVar = item.value("class");
    if (classVar.type() == QVariant::String)
    {
        info->classification = BodyInfo::parseClassification(classVar.toString());
    }

    QVariant descVar = item.value("description");
    if (descVar.type() == QVariant::String)
    {
        info->description = descVar.toString();
    }

    QVariant infoSourceVar = item.value("infoSource");
    if (infoSourceVar.type() == QVariant::String)
    {
        info->infoSource = infoSourceVar.toString();
        if (!info->infoSource.startsWith("help:"))
        {
            info->infoSource = dataFileName(info->infoSource);
        }
    }

    QVariant massVar = item.value("mass");
    if (massVar.isValid())
    {
        bool ok = false;
        info->massKg = massValue(massVar, Unit_Kilogram, 0.0, &ok);
        if (!ok)
        {
            errorMessage("Bad value given for mass");
        }
    }

    QVariant densityVar = item.value("density");
    if (densityVar.isValid())
    {
        bool ok = false;
        info->density = densityVar.toFloat(&ok);
        if (!ok)
        {
            errorMessage("Bad value given for density");
        }
    }

    QVariant labelVar = item.value("label");
    if (labelVar.type() == QVariant::Map)
    {
        loadLabelInfo(info, labelVar.toMap());
    }

    // The default trajectory color is the label color
    info->trajectoryPlotColor = info->labelColor;

    QVariant trajectoryPlotVar = item.value("trajectoryPlot");
    if (trajectoryPlotVar.type() == QVariant::Map)
    {
        QVariantMap trajectoryPlot = trajectoryPlotVar.toMap();
        loadTrajectoryPlotInfo(info, trajectoryPlot);
    }

    return info;
}


CatalogContents*
UniverseLoader::loadCatalogItems(const QVariantMap& contents,
                                 UniverseCatalog* catalog)
{
    return loadCatalogItems(contents, catalog, 0);
}


CatalogContents*
UniverseLoader::loadCatalogFile(const QString& fileName,
                                UniverseCatalog* catalog)
{
    if (fileName.toLower().endsWith(".ssc"))
    {
        QStringList spiceKernels;
        QStringList bodyNames = loadSSC(fileName, catalog, 0);
        return new CatalogContents(bodyNames, spiceKernels);
    }
    else
    {
        return loadCatalogFile(fileName, catalog, 0);
    }
}


// Load a Celestia SSC (Solar System Catalog) file
QStringList
UniverseLoader::loadSSC(const QString& fileName,
                        UniverseCatalog* catalog,
                        unsigned int requireDepth)
{
    QStringList bodyNames;

    QString path = dataFileName(fileName);

    QFileInfo info(path);
    path = info.canonicalFilePath();

    QFile catalogFile(path);
    if (!catalogFile.open(QIODevice::ReadOnly))
    {
        errorMessage(QString("Cannot open SSC file %1").arg(path));
        return bodyNames;
    }

    // Save search paths
    QString searchPath = info.absolutePath();
    QString saveDataSearchPath = m_dataSearchPath;
    QString saveTextureSearchPath = m_textureSearchPath;
    QString saveModelSearchPath = m_modelSearchPath;

    // SSC files expect media and trajectory data files in subdirectories:
    //   trajectories and rotation models - ./data
    //   textures - ./textures/medres
    //   mesh files - ./models
    // Where '.' is the directory containing the ssc file
    setDataSearchPath(searchPath + "/data");
    setModelSearchPath(searchPath + "/models");
    setTextureSearchPath(searchPath + "/textures/medres");

    if (m_textureLoader.isValid())
    {
        m_textureLoader->setSearchPath(std::string(searchPath.toUtf8().data()) + "/textures/medres");
    }
    setTexturesInModelDirectory(false);

    QVariantList items;

    CatalogParser parser(&catalogFile);
    QVariant obj = parser.nextSscObject();
    while (obj.type() == QVariant::Map)
    {
        QJson::Serializer serializer;
#if DEBUG_SSC_CONVERSION
        qDebug() << serializer.serialize(obj);
#endif

        QVariantMap map = obj.toMap();
        TransformSscObject(&map);
#if DEBUG_SSC_CONVERSION
        qDebug() << "Converted: " << serializer.serialize(map);
#endif

        QString fullName = map.value("_parent").toString() + "/" + map.value("name").toString();
        map.insert("name", fullName);
        items << map;

        obj = parser.nextSscObject();
    }

    QVariantMap contents;
    contents.insert("name", fileName);
    contents.insert("version", "1.0");
    contents.insert("items", items);

    CatalogContents* catalogContents = loadCatalogItems(contents, catalog, requireDepth + 1);
    bodyNames = catalogContents->bodyNames();
    delete catalogContents;

    // Restore search paths
    setDataSearchPath(saveDataSearchPath);
    setModelSearchPath(saveModelSearchPath);
    if (m_textureLoader.isValid())
    {
        m_textureLoader->setSearchPath(saveTextureSearchPath.toUtf8().data());
    }

    // Reset the textures in model directory bit
    setTexturesInModelDirectory(true);

    return bodyNames;
}


CatalogContents*
UniverseLoader::loadCatalogFile(const QString& fileName,
                                UniverseCatalog* catalog,
                                unsigned int requireDepth)
{
    QString path = dataFileName(fileName);
    CatalogContents* contents = new CatalogContents();

    QFileInfo info(path);
    path = info.canonicalFilePath();

    if (m_loadedCatalogFiles.contains(path))
    {
        // File is already loaded
        return contents;
    }

    if (requireDepth > 10)
    {
        errorMessage("'require' is nested too deeply (recursive requires?)");
        return contents;
    }

    QFile catalogFile(path);
    if (!catalogFile.open(QIODevice::ReadOnly))
    {
        errorMessage(QString("Cannot open required file %1").arg(path));
        return contents;
    }

    // Strip single-line C++ style comments from the JSON text. This is a
    // temporary solution, as the regex used here doesn't properly distinguish
    // and ignore comment characters in the middle of a string.
    QString catalogText(catalogFile.readAll());
    QRegExp stripComments("//[^\"]*[\n\r]");
    stripComments.setMinimal(true);
    QByteArray catalogBytes = catalogText.replace(stripComments, " ").toUtf8();
    QBuffer buffer(&catalogBytes);

    QJson::Parser parser;

    bool parseOk = false;
    QVariant result = parser.parse(&buffer, &parseOk);
    if (!parseOk)
    {
        errorMessage(QString("Error in %1, line %2: %3").arg(path).arg(parser.errorLine()).arg(parser.errorString()));
        return contents;
    }

    QVariantMap contentsMap = result.toMap();
    if (contentsMap.empty())
    {
        errorMessage("Solar system catalog is empty.");
        return contents;
    }

    // Save search paths
    QString searchPath = info.absolutePath();
    QString saveDataSearchPath = m_dataSearchPath;
    QString saveTextureSearchPath = m_textureSearchPath;
    QString saveModelSearchPath = m_modelSearchPath;
    setDataSearchPath(searchPath);
    setModelSearchPath(searchPath);

    delete contents;
    contents = loadCatalogItems(contentsMap, catalog, requireDepth + 1);

    // Restore search paths
    setDataSearchPath(saveDataSearchPath);
    setModelSearchPath(saveModelSearchPath);

    return contents;
}


CatalogContents*
UniverseLoader::loadCatalogItems(const QVariantMap& contentsMap,
                                 UniverseCatalog* catalog,
                                 unsigned int requireDepth)
{
#ifdef DEBUG
    qDebug() << "Loading catalog " << contents["name"].toString();
#endif
    m_currentBodyName = "";

    CatalogContents* contents = new CatalogContents();

    // Validate the file version (must be 1.0 right now)
    QVariant versionVar = contentsMap.value("version");
    if (!versionVar.isValid())
    {
        errorMessage("Version missing from catalog file");
        return contents;
    }
    else if (versionVar.toString() != "1.0")
    {
        qDebug() << versionVar;
        errorMessage(QString("Unsupported catalog file version %1 (only version 1.0 allowed)").arg(versionVar.toString()));
        return contents;
    }

    if (contentsMap.contains("require"))
    {
        QVariant requireVar = contentsMap.value("require");
        if (requireVar.type() == QVariant::List)
        {
            QVariantList requireList = requireVar.toList();
            foreach (QVariant v, requireList)
            {
                if (v.type() == QVariant::String)
                {
                    QString fileName = v.toString();
                    if (fileName.toLower().endsWith(".ssc"))
                    {
                        QStringList bodyNames = contents->bodyNames();
                        bodyNames << loadSSC(fileName, catalog, requireDepth);
                        contents->setBodyNames(bodyNames);
                    }
                    else
                    {
                        CatalogContents* catalogContents = loadCatalogFile(fileName, catalog, requireDepth);
                        contents->appendContents(catalogContents);
                        delete catalogContents;
                    }
                }
            }
        }
        else
        {
            errorMessage("Require property must be a list of filenames");
        }
    }

    if (!contentsMap.contains("items"))
    {
        return contents;
    }

    if (contentsMap["items"].type() != QVariant::List)
    {
        errorMessage("items is not a list.");
        return contents;
    }
    QVariantList items = contentsMap["items"].toList();

    if (contentsMap.contains("spiceKernels"))
    {
        QVariant kernelsVar = contentsMap["spiceKernels"];
        if (kernelsVar.type() != QVariant::List)
        {
            errorMessage("spiceKernels is not a list.");
            return contents;
        }

        QVariantList kernelList = kernelsVar.toList();
        QStringList resolvedKernelFileList = resolveSpiceKernelList(kernelList);
        foreach (QString kernelFile, resolvedKernelFileList)
        {
            contents->appendSpiceKernel(kernelFile);
        }

        loadSpiceKernels(resolvedKernelFileList);
    }

    foreach (QVariant itemVar, items)
    {
        m_currentBodyName = "";

        if (itemVar.type() != QVariant::Map)
        {
            errorMessage("Invalid item in bodies list.");
        }
        else
        {
            QVariantMap item = itemVar.toMap();

            QString type = item.value("type").toString();
            if (type == "body" || type.isEmpty())
            {
                QString bodyName = item.value("name").toString();
                m_currentBodyName = bodyName;

                bool newBody = false;
                bool valid = true;

                vesta::Body* body = dynamic_cast<Body*>(catalog->find(bodyName));
                if (body == NULL)
                {
                    newBody = true;

                    // No body with this name exists, so create it
                    body = new vesta::Body();
                    body->setName(bodyName.toUtf8().data());

                    // Add the body to the catalog now so that it may be referenced by
                    // frames.
                    catalog->addBody(bodyName, body);
                }

                // The following values will be assigned to the body *if* it
                // can be successfully loaded.
                counted_ptr<vesta::Geometry> geometry;
                double startTime = DefaultStartTime;
                QList<counted_ptr<vesta::Arc> > arcs;

                if (item.contains("geometry"))
                {
                    QVariant geometryValue = item.value("geometry");
                    if (geometryValue.type() == QVariant::Map)
                    {
                        geometry = loadGeometry(geometryValue.toMap(), catalog);
                    }
                    else
                    {
                        errorMessage("Invalid geometry for body.");
                        valid = false;
                    }
                }

                QVariant startTimeVar = item.value("startTime");
                if (startTimeVar.isValid())
                {
                    bool ok = false;
                    startTime = dateValue(startTimeVar, &ok);
                    if (!ok)
                    {
                        errorMessage("Invalid startTime specified");
                        valid = false;
                    }
                }

                // A list of arcs may be provided
                QVariant arcsVar = item.value("arcs");
                if (arcsVar.isValid())
                {
                    if (arcsVar.type() != QVariant::List)
                    {
                        errorMessage("Arcs must be an array");
                    }
                    else
                    {
                        arcs = loadChronology(arcsVar.toList(), catalog, startTime);
                    }

#if 0
                    // Debugging code to output positions for one arc in the frame of the previous one.
                    // Useful for 'stitching' arcs together.
                    double t = startTime;
                    for (unsigned int i = 1; i < arcs.size(); ++i)
                    {
                        vesta::Arc* prevArc = arcs.at(i - 1).ptr();
                        vesta::Arc* arc = arcs.at(i).ptr();

                        t += prevArc->duration();
                        Vector3d upos = prevArc->center()->position(t) + prevArc->trajectoryFrame()->orientation(t) * prevArc->trajectory()->position(t);
                        Vector3d pos = arc->trajectoryFrame()->orientation(t).conjugate() * (upos - arc->center()->position(t));
                        qDebug() << bodyName
                                 << QString(": %1, %2, %3").arg(pos.x(), 0, 'g', 16).arg(pos.y(), 0, 'g', 16).arg(pos.z(), 0, 'g', 16);
                    }
#endif
                }
                else
                {
                    // No list provided; just read the properties for a single arc
                    vesta::Arc* arc = loadArc(item, catalog, startTime);
                    if (arc)
                    {
                        arcs << counted_ptr<vesta::Arc>(arc);
                    }
                }

                // At least one arc is required
                if (arcs.isEmpty())
                {
                    valid = false;
                }

                // If we successfully loaded a new body, add it to the list if it's new
                // or replace it otherwise.
                //
                // If we failed then leave any existing body with the same name alone.
                if (valid)
                {
                    BodyInfo* info = loadBodyInfo(item);
                    catalog->setBodyInfo(bodyName, info);

                    // Set all information about a body to the default state
                    body->setLightSource(NULL);
                    body->setGeometry(NULL);
                    body->setVisible(true);
                    body->chronology()->clearArcs();

                    // Visible property
                    body->setVisible(item.value("visible", true).toBool());
                    body->setGeometry(geometry.ptr());
                    body->chronology()->setBeginning(startTime);
                    foreach (counted_ptr<vesta::Arc> arc, arcs)
                    {
                        body->chronology()->addArc(arc.ptr());
                    }

                    contents->appendBody(bodyName);
                }
                else
                {
                    errorMessage(QString("Skipping body '%1' because of errors.").arg(bodyName));
                    if (newBody)
                    {
                        catalog->removeBody(bodyName);
                    }
                }
            }
            else if (type == "Visualizer")
            {
                QVariant tagVar = item.value("tag");
                QVariant bodyVar = item.value("body");

                if (tagVar.type() != QVariant::String)
                {
                    errorMessage("Bad or missing tag for visualizer");
                }
                else if (bodyVar.type() != QVariant::String)
                {
                    errorMessage("Bad or missing body name for visualizer");
                }
                else
                {
                    QString tag = tagVar.toString();
                    QString bodyName = bodyVar.toString();

                    Entity* body = catalog->find(bodyName);
                    if (body == NULL)
                    {
                        errorMessage(QString("Can't find body '%1' for visualizer.").arg(bodyName));
                    }
                    else
                    {
                        Visualizer* visualizer = loadVisualizer(item, catalog);
                        if (visualizer)
                        {
                            body->setVisualizer(tag.toUtf8().data(), visualizer);
                        }
                    }
                }
            }
            else if (type == "FeatureLabels")
            {
                QVariant bodyVar = item.value("body");

                if (bodyVar.type() != QVariant::String)
                {
                    errorMessage("Bad or missing body name for visualizer");
                }
                else
                {
                    QString bodyName = bodyVar.toString();

                    Entity* body = catalog->find(bodyName);
                    if (body == NULL)
                    {
                        errorMessage(QString("Can't find body '%1' for feature labels.").arg(bodyName));
                    }
                    else
                    {
                        Visualizer* visualizer = loadFeatureLabels(item, body);
                        if (visualizer)
                        {
                            body->setVisualizer("surface features", visualizer);
                        }
                    }
                }
            }
            else if (type == "Viewpoint")
            {
                Viewpoint* viewpoint = loadViewpoint(item, catalog);
                if (viewpoint)
                {
                    catalog->addViewpoint(QString::fromUtf8(viewpoint->name().c_str()), viewpoint);
                }
            }
        }
    }

    return contents;
}


void
UniverseLoader::loadSpiceKernels(const QStringList& kernelList)
{
#ifdef SPICE_ENABLED
    foreach (QString kernel, kernelList)
    {
        furnsh_c(kernel.toLatin1().data());
    }
#endif
}


QStringList
UniverseLoader::resolveSpiceKernelList(const QVariantList& kernelList)
{
    QStringList resolvedKernelFilePaths;
    foreach (QVariant kernel, kernelList)
    {
        if (kernel.type() == QVariant::String)
        {
            QString kernelFileName = kernel.toString();
            resolvedKernelFilePaths << dataFileName(kernelFileName);
        }
        else
        {
            errorMessage("Spice kernel list contains non-string value.");
        }
    }

    return resolvedKernelFilePaths;
}


void
UniverseLoader::unloadSpiceKernels(const QStringList& kernelList)
{
#ifdef SPICE_ENABLED
    for (int i = kernelList.length() - 1; i >= 0; --i)
    {
        QString kernel = kernelList.at(i);
        unload_c(kernel.toLatin1().data());
    }
#endif
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
UniverseLoader::addBuiltinRotationModel(const QString& name, vesta::RotationModel* rotationModel)
{
    m_builtinRotations[name] = rotationModel;
}


void
UniverseLoader::removeBuiltinRotationModel(const QString& name)
{
    m_builtinRotations.remove(name);
}


void
UniverseLoader::setTextureLoader(PathRelativeTextureLoader *textureLoader)
{
    m_textureLoader = textureLoader;
}


void
UniverseLoader::setDataSearchPath(const QString& path)
{
    m_dataSearchPath = path;
}


void
UniverseLoader::setTextureSearchPath(const QString& path)
{
    m_textureSearchPath = path;
}


void
UniverseLoader::setModelSearchPath(const QString& path)
{
    m_modelSearchPath = path;
}


QString
UniverseLoader::dataFileName(const QString& fileName)
{
    return m_dataSearchPath + "/" + fileName;
}


QString
UniverseLoader::modelFileName(const QString& fileName)
{
    return m_modelSearchPath + "/" + fileName;
}


void
UniverseLoader::cleanGeometryCache()
{
    // Remove items from the geometry cache that are only referenced
    // in the cache.
    foreach (QString resourcePath, m_geometryCache.keys())
    {
        Geometry* geometry = m_geometryCache.find(resourcePath)->ptr();
        if (geometry && geometry->refCount() == 1)
        {
            m_geometryCache.remove(resourcePath);
        }
    }
}


/** Process all pending object updates, e.g. new TLE sets received from
  * the network.
  */
void
UniverseLoader::processUpdates()
{
    foreach (TleRecord tleData, m_tleUpdates)
    {
        QString key = TleKey(tleData.source, tleData.name);

        // Add it to the TLE cache
        m_tleCache.insert(key, tleData);

        // Update all TLE trajectories that refer to this TLE
        foreach (counted_ptr<TleTrajectory> trajectory, m_tleTrajectories.values(key))
        {
            // Create a temporary TLE trajectory from the data and use it to udpate the trajectory in
            // the cache.
            counted_ptr<TleTrajectory> tempTle(TleTrajectory::Create(tleData.line1.toLatin1().data(),
                                                                     tleData.line2.toLatin1().data()));
            if (tempTle.isValid())
            {
                trajectory->copy(tempTle.ptr());
            }
            else
            {
                qDebug() << "Bad TLE received: " << tleData.name << " from " << tleData.source;
            }
        }
    }

    m_tleUpdates.clear();
}


/** Process a new TLE data set.
  */
void
UniverseLoader::processTleSet(const QString &source, QTextStream& stream)
{
    QTextStream::Status status = QTextStream::Ok;
    while (status == QTextStream::Ok)
    {
        QString name = stream.readLine();
        QString tleLine1 = stream.readLine();
        QString tleLine2 = stream.readLine();

        name = name.trimmed();
        status = stream.status();
        if (status == QTextStream::Ok)
        {
            if (name.isEmpty())
            {
                break;
            }
            else
            {
                updateTle(source, name, tleLine1, tleLine2);
            }
        }
    }
}


void
UniverseLoader::updateTle(const QString &source, const QString &name, const QString &line1, const QString &line2)
{
    TleRecord tle;
    tle.source = source;
    tle.name = name;
    tle.line1 = line1;
    tle.line2 = line2;

    m_tleUpdates << tle;
}


/** Get the set of all resources requested (since the last time clearResourceRequests was called.)
  */
QSet<QString>
UniverseLoader::resourceRequests() const
{
    return m_resourceRequests;
}


/** Clear all resource requests.
  */
void
UniverseLoader::clearResourceRequests()
{
    m_resourceRequests.clear();
}


void
UniverseLoader::clearMessageLog()
{
    m_messageLog.clear();
}


QString
UniverseLoader::messageLog()
{
    return m_messageLog;
}


void
UniverseLoader::errorMessage(const QString& message)
{
    if (!m_currentBodyName.isEmpty())
    {
        m_messageLog += QString("Item '%1': ").arg(m_currentBodyName);
    }
    m_messageLog += message;
    m_messageLog += '\n';
}


void
UniverseLoader::warningMessage(const QString& message)
{
    if (!m_currentBodyName.isEmpty())
    {
        m_messageLog += QString("Item '%1': ").arg(m_currentBodyName);
    }
    m_messageLog += message;
    m_messageLog += '\n';
}
