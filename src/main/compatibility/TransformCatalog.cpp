// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
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

// This file contains functions for transforming catalog objects from
// Celestia format. All transformations are performed on variant maps.

#include "TransformCatalog.h"
#include <QDebug>

enum SscUnitsType
{
    HeliocentricUnits,     // Distances in AU, periods in years
    PlanetocentricUnits    // Distances in km, periods in days
};

TransformSscStatus
TransformSscFrame(QVariantMap* obj, const QString& oldName, const QString& newName);

// Change the name of the property from oldKey to newKey.
// Returns true if the old key was found
static bool
TransformProperty(QVariantMap* obj, const QString& oldKey, const QString& newKey)
{
    if (obj->contains(oldKey))
    {
        obj->insert(newKey, obj->value(oldKey));
        obj->remove(oldKey);
        return true;
    }
    else
    {
        return false;
    }
}


// Move a property with the specified name from one table to another. Return true if
// the property existed in the source table.
static bool
MoveProperty(const QVariantMap* src, const QString& srcKey,
             QVariantMap* dst, const QString& dstKey)
{
    if (src->contains(srcKey))
    {
        dst->insert(dstKey, src->value(srcKey));
        return true;
    }
    else
    {
        return false;
    }
}


struct NameMapping
{
    QString from;
    QString to;
};

static NameMapping SolarSystemNames[] =
{
    { "Sol", "Sun" },
    { "Sol/Mercury", "Mercury" },
    { "Sol/Venus", "Venus" },
    { "Sol/Earth", "Earth" },
    { "Sol/Mars", "Mars" },
    { "Sol/Jupiter", "Jupiter" },
    { "Sol/Saturn", "Saturn" },
    { "Sol/Uranus", "Uranus" },
    { "Sol/Neptune", "Neptune" },
    { "Sol/Pluto", "Pluto" },
    { "Sol/Earth/Moon", "Moon" },
};

static QMap<QString, QString> SolarSystemMappings;

QString
TransformSolarSystemName(const QString& name)
{
    if (SolarSystemMappings.isEmpty())
    {
        for (unsigned int i = 0; i < sizeof(SolarSystemNames) / sizeof(SolarSystemNames[0]); ++i)
        {
            SolarSystemMappings[SolarSystemNames[i].from] = SolarSystemNames[i].to;
        }
    }

    if (SolarSystemMappings.contains(name))
    {
        return SolarSystemMappings.value(name);
    }
    else
    {
        return name;
    }
}


TransformSscStatus
TransformSscGeometry(QVariantMap* obj)
{
    QVariantMap geometry;

    if (obj->value("Sensor").type() == QVariant::Map)
    {
        QVariantMap sensor = obj->value("Sensor").toMap();

        geometry["type"] = "Sensor";
        MoveProperty(&sensor, "Target", &geometry, "target");
        MoveProperty(&sensor, "Range", &geometry, "range");
        MoveProperty(&sensor, "Shape", &geometry, "shape");
        MoveProperty(&sensor, "HorizontalFOV", &geometry, "horizontalFov");
        MoveProperty(&sensor, "VerticalFOV", &geometry, "verticalFov");
        MoveProperty(&sensor, "FrustumColor", &geometry, "frustumColor");
        MoveProperty(&sensor, "FrustumBaseColor", &geometry, "frustumBaseColor");
        MoveProperty(&sensor, "FrustumOpacity", &geometry, "frustumOpacity");
        MoveProperty(&sensor, "GridOpacity", &geometry, "gridOpacity");

        if (geometry.contains("target"))
        {
            geometry.insert("target", TransformSolarSystemName(geometry.value("target").toString()));
        }
    }
    else if (obj->contains("Mesh"))
    {
        geometry["type"] = "Mesh";
        geometry["source"] = obj->value("Mesh");
        MoveProperty(obj, "NormalizeMesh", &geometry, "normalize");
        MoveProperty(obj, "MeshScale", &geometry, "scale");
        MoveProperty(obj, "MeshCenter", &geometry, "center");
    }
    else
    {
        geometry["type"] = "Globe";
        if (obj->contains("SemiAxes"))
        {
            geometry["radii"] = obj->value("SemiAxes");
        }
        else
        {
            geometry["radius"] = obj->value("Radius");
        }
    }

    obj->insert("geometry", geometry);

    obj->remove("Mesh");
    obj->remove("Sensor");
    obj->remove("SemiAxes");
    obj->remove("Radius");
    obj->remove("NormalizeMesh");
    obj->remove("MeshScale");
    obj->remove("MeshCenter");

    return SscOk;
}


TransformSscStatus
TransformSscTrajectory(QVariantMap* obj, SscUnitsType units)
{
    QVariantMap trajectory;
    if (obj->contains("CustomOrbit"))
    {
        trajectory["type"] = "Builtin";
        trajectory["name"] = obj->value("CustomOrbit");
    }
    else if (obj->contains("SpiceOrbit"))
    {
        QVariant value = obj->value("SpiceOrbit");
        if (value.type() == QVariant::Map)
        {
            QVariantMap properties = value.toMap();
            trajectory["type"] = "Spice";
            trajectory["source"] = 0;
        }
    }
    else if (obj->contains("ScriptedOrbit"))
    {
        QVariant value = obj->value("ScriptedOrbit");
        if (value.type() == QVariant::Map)
        {
            QVariantMap properties = value.toMap();
            trajectory["type"] = "Scripted";
            trajectory["source"] = 0;
        }
    }
    else if (obj->contains("SampledTrajectory"))
    {
        QVariant value = obj->value("SampledTrajectory");
        if (value.type() == QVariant::Map)
        {
            QVariantMap properties = value.toMap();
            trajectory["type"] = "InterpolatedStates";
            MoveProperty(&properties, "Source", &trajectory, "source");
            MoveProperty(&properties, "Interpolation", &trajectory, "interpolation");
        }
    }
    else if (obj->contains("SampledOrbit"))
    {
        trajectory["type"] = "InterpolatedStates";
        trajectory["source"] = obj->value("SampledOrbit");
    }
    else if (obj->contains("EllipticalOrbit"))
    {
        QVariant value = obj->value("EllipticalOrbit");
        QVariantMap properties = value.toMap();
        trajectory["type"] = "Keplerian";

        // SemiMajorAxis and Period properties require some special handling. Both
        // properties are required to present in an SSC EllipticalOrbit. And the
        // values of both are interpreted differently depending on whether the
        // body is orbiting the Sun or a planet (or moon etc.):
        //    Period has units of years for heliocentric objects, days otherwise
        //    SemiMajorAxis has units of AU for heliocentric objects, kilometers otherwise
        QVariant sma = properties.value("SemiMajorAxis");
        QVariant period = properties.value("Period");
        if (!sma.canConvert(QVariant::Double))
        {
            qDebug() << "Bad or missing SemiMajorAxis for EllipticalOrbit in SSC file";
            return SscInvalidTrajectory;
        }

        if (!period.canConvert(QVariant::Double))
        {
            qDebug() << "Bad or missing Period for EllipticalOrbit in SSC file";
            return SscInvalidTrajectory;
        }

        if (units == HeliocentricUnits)
        {
            sma = QString("%1au").arg(sma.toString());
            period = QString("%1y").arg(period.toString());
        }
        else if (units == PlanetocentricUnits)
        {
            sma = QString("%1km").arg(sma.toString());
            period = QString("%1d").arg(period.toString());
        }

        trajectory.insert("semiMajorAxis", sma);
        trajectory.insert("period", period);

        MoveProperty(&properties, "Eccentricity", &trajectory, "eccentricity");
        MoveProperty(&properties, "Inclination", &trajectory, "inclination");
        MoveProperty(&properties, "AscendingNode", &trajectory, "ascendingNode");
        MoveProperty(&properties, "ArgOfPericenter", &trajectory, "argumentOfPeriapsis");
        MoveProperty(&properties, "MeanAnomaly", &trajectory, "meanAnomaly");
    }
    else if (obj->contains("FixedPosition"))
    {
        QVariant value = obj->value("FixedPosition");
        if (value.type() == QVariant::List)
        {
            trajectory["type"] = "FixedPoint";
            trajectory["position"] = value;
        }
        else if (value.type() == QVariant::Map)
        {
        }
    }
    else
    {
        return SscMissingTrajectory;
    }

    obj->insert("trajectory", trajectory);
    obj->remove("CustomOrbit");
    obj->remove("SpiceOrbit");
    obj->remove("ScriptedOrbit");
    obj->remove("SampledTrajectory");
    obj->remove("SampledOrbit");
    obj->remove("EllipticalOrbit");
    obj->remove("FixedPosition");

    return SscOk;
}


TransformSscStatus
TransformSscRotationModel(QVariantMap* obj)
{
    QVariantMap rotationModel;
    if (obj->contains("CustomRotation"))
    {
    }
    else if (obj->contains("SpiceRotation"))
    {
    }
    else if (obj->contains("ScriptedRotation"))
    {
    }
    else if (obj->contains("SampledOrientation"))
    {
        rotationModel["type"] = "Interpolated";
        rotationModel["source"] = obj->value("SampledOrientation");
        rotationModel["compatibility"] = "celestia";
    }
    else if (obj->contains("PrecessingRotation"))
    {
    }
    else if (obj->contains("UniformRotation"))
    {
        QVariant value = obj->value("UniformRotation");
        if (value.type() == QVariant::Map)
        {
            QVariantMap properties = value.toMap();
            rotationModel["type"] = "Uniform";
            MoveProperty(&properties, "Period", &rotationModel, "period");
            MoveProperty(&properties, "Inclination", &rotationModel, "inclination");
            MoveProperty(&properties, "AscendingNode", &rotationModel, "ascendingNode");
            MoveProperty(&properties, "MeridianAngle", &rotationModel, "meridianAngle");
            MoveProperty(&properties, "Epoch", &rotationModel, "epoch");
        }
    }
    else if (obj->contains("FixedRotation"))
    {
        QVariant value = obj->value("FixedRotation");
        if (value.type() == QVariant::Map)
        {
            QVariantMap properties = value.toMap();
            rotationModel["type"] = "Fixed";
            MoveProperty(&properties, "Inclination", &rotationModel, "inclination");
            MoveProperty(&properties, "AscendingNode", &rotationModel, "ascendingNode");
            MoveProperty(&properties, "MeridianAngle", &rotationModel, "meridianAngle");
        }
    }
    else if (obj->contains("FixedAttitude"))
    {
    }
    else if (obj->contains("FixedQuaternion"))
    {
        rotationModel["type"] = "Fixed";
        rotationModel["quaternion"] = obj->value("Quaternion");
    }

    if (!rotationModel.empty())
    {
        obj->insert("rotationModel", rotationModel);
    }
    obj->remove("CustomRotation");
    obj->remove("SpiceRotation");
    obj->remove("ScriptedRotation");
    obj->remove("SampledOrientation");
    obj->remove("PrecessingRotation");
    obj->remove("UniformRotation");
    obj->remove("FixedRotation");
    obj->remove("FixedAttitude");

    return SscOk;
}


QVariantMap
transformTwoVectorDirection(const QVariantMap& direction)
{
    QVariantMap newDirection;

    if (direction.contains("RelativePosition"))
    {
        QVariant relativePosVar = direction.value("RelativePosition");
        if (relativePosVar.type() == QVariant::Map)
        {
            QVariantMap relativePos = relativePosVar.toMap();
            newDirection.insert("type", "RelativePosition");
            if (relativePos.contains("Target"))
            {
                newDirection.insert("target", TransformSolarSystemName(relativePos.value("Target").toString()));
            }
            if (relativePos.contains("Observer"))
            {
                newDirection.insert("observer", TransformSolarSystemName(relativePos.value("Observer").toString()));
            }
        }
    }
    else if (direction.contains("RelativeVelocity"))
    {
        QVariant relativeVelVar = direction.value("RelativeVelocity");
        if (relativeVelVar.type() == QVariant::Map)
        {
            QVariantMap relativeVel = relativeVelVar.toMap();
            newDirection.insert("type", "RelativeVelocity");
            if (relativeVel.contains("Target"))
            {
                newDirection.insert("target", TransformSolarSystemName(relativeVel.value("Target").toString()));
            }
            if (relativeVel.contains("Observer"))
            {
                newDirection.insert("observer", TransformSolarSystemName(relativeVel.value("Observer").toString()));
            }
        }
    }
    else if (direction.contains("ConstantVector"))
    {
        QVariant constantVecVar = direction.value("ConstantVector");
        if (constantVecVar.type() == QVariant::Map)
        {
            QVariantMap constantVec = constantVecVar.toMap();
            newDirection.insert("type", "ConstantVector");
            MoveProperty(&constantVec, "Vector", &newDirection, "direction");
            MoveProperty(&constantVec, "Frame", &newDirection, "frame");
            TransformSscStatus status = TransformSscFrame(&newDirection, "frame", "frame");
            if (status != SscOk)
            {
                qDebug() << "Error transforming ConstantVector in TwoVector frame";
            }
        }
    }

    return newDirection;
}


TransformSscStatus
TransformSscFrame(QVariantMap* obj, const QString& oldName, const QString& newName)
{
    if (obj->contains(oldName))
    {
        QVariant value = obj->value(oldName);
        if (value.type() != QVariant::Map)
        {
            return SscBadFrame;
        }

        QVariantMap properties = value.toMap();

        if (properties.contains("BodyFixed"))
        {
            QVariant frameValue = properties.value("BodyFixed");
            if (frameValue.type() != QVariant::Map)
            {
                return SscBadFrame;
            }

            QVariantMap sscFrame = frameValue.toMap();
            QVariantMap frame;
            frame["type"] = "BodyFixed";
            MoveProperty(&sscFrame, "Center", &frame, "body");

            obj->insert(newName, frame);
        }
        else if (properties.contains("MeanEquator"))
        {
            // Not supported
        }
        else if (properties.contains("TwoVector"))
        {
            QVariant twoVectorVar = properties.value("TwoVector");
            if (twoVectorVar.type() == QVariant::Map)
            {
                QVariantMap oldFrame = twoVectorVar.toMap();

                QVariantMap frame;
                frame["type"] = "TwoVector";

                QVariant primaryVar = oldFrame.value("Primary");
                QVariant secondaryVar = oldFrame.value("Secondary");
                QVariant centerVar = oldFrame.value("Center");

                if (primaryVar.type() == QVariant::Map && secondaryVar.type() == QVariant::Map)
                {
                    QVariantMap primary = primaryVar.toMap();
                    QVariantMap secondary = secondaryVar.toMap();

                    QVariantMap newPrimary = transformTwoVectorDirection(primary);
                    QVariantMap newSecondary = transformTwoVectorDirection(secondary);

                    // Observer defaults to center when it's not specified
                    if (centerVar.type() == QVariant::String)
                    {
                        QString center = TransformSolarSystemName(centerVar.toString());
                        if (!newPrimary.contains("observer"))
                        {
                            newPrimary.insert("observer", center);
                        }
                        if (!newSecondary.contains("observer"))
                        {
                            newSecondary.insert("observer", center);
                        }
                    }

                    // In VESTA, axes are properties of two vector frames, not of the directions
                    frame["primaryAxis"] = primary.value("Axis");
                    frame["secondaryAxis"] = secondary.value("Axis");
                    frame["primary"] = newPrimary;
                    frame["secondary"] = newSecondary;
                }
                obj->insert(newName, frame);
            }
        }
        else if (properties.contains("Topocentric"))
        {
        }
        else if (properties.contains("EclipticJ2000"))
        {
            obj->insert(newName, "EclipticJ2000");
        }
        else if (properties.contains("EquatorJ2000"))
        {
            obj->insert(newName, "EquatorJ2000");
        }

        if (newName != oldName)
        {
            obj->remove(oldName);
        }
    }

    return SscOk;
}


TransformSscStatus
TransformSscArc(QVariantMap* obj)
{
    TransformSscStatus status = SscOk;

    // TODO: handle overrides in trajectory frame
    QString centerName = TransformSolarSystemName(obj->value("_parent").toString());
    obj->insert("center", centerName);

    status = TransformSscTrajectory(obj, centerName == "Sun" ? HeliocentricUnits : PlanetocentricUnits);
    if (status != SscOk)
    {
        return status;
    }

    status = TransformSscRotationModel(obj);
    if (status != SscOk)
    {
        return status;
    }

    status = TransformSscFrame(obj, "OrbitFrame", "trajectoryFrame");
    if (status != SscOk)
    {
        return status;
    }

    status = TransformSscFrame(obj, "BodyFrame", "bodyFrame");
    if (status != SscOk)
    {
        return status;
    }

    return SscOk;
}


TransformSscStatus
TransformSscBody(QVariantMap* obj)
{
    TransformSscStatus status = SscOk;

    status = TransformSscGeometry(obj);
    if (status != SscOk)
    {
        return status;
    }

    status = TransformSscArc(obj);
    if (status != SscOk)
    {
        return status;
    }

    TransformProperty(obj, "_name", "name");
    TransformProperty(obj, "Class", "class");
    TransformProperty(obj, "Visible", "visible");

    if (obj->contains("OrbitColor") ||
        obj->contains("TrajectoryPlotDuration") ||
        obj->contains("TrajectoryPlotLead") ||
        obj->contains("TrajectoryPlotFade"))
    {
        QVariantMap trajectoryPlot;
        MoveProperty(obj, "OrbitColor", &trajectoryPlot, "color");
        MoveProperty(obj, "TrajectoryPlotDuration", &trajectoryPlot, "duration");
        MoveProperty(obj, "TrajectoryPlotLead", &trajectoryPlot, "lead");
        MoveProperty(obj, "TrajectoryPlotFade", &trajectoryPlot, "fade");
        obj->insert("trajectoryPlot", trajectoryPlot);
    }

    return SscOk;
}


TransformSscStatus
TransformSscSurface(QVariantMap* /* obj */)
{
    return SscOk;
}


TransformSscStatus
TransformSscLocation(QVariantMap* /* obj */)
{
    return SscOk;
}


TransformSscStatus
TransformSscObject(QVariantMap* obj)
{
    QString type = obj->value("_type").toString();
    if (type == "Body")
    {
        return TransformSscBody(obj);
    }
    else if (type == "Location")
    {
        return TransformSscLocation(obj);
    }
    else if (type == "AltSurface")
    {
        return TransformSscSurface(obj);
    }
    else
    {
        return SscBadType;
    }
}
