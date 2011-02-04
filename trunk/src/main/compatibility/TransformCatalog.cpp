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


TransformSscStatus
TransformSscGeometry(QVariantMap* obj)
{
    QVariantMap geometry;

    if (obj->contains("Sensor"))
    {
        geometry["type"] = "Sensor";
        MoveProperty(obj, "Target", &geometry, "target");
        MoveProperty(obj, "Range", &geometry, "range");
        MoveProperty(obj, "Shape", &geometry, "shape");
        MoveProperty(obj, "HorizontalFOV", &geometry, "horizontalFov");
        MoveProperty(obj, "VerticalFOV", &geometry, "verticalFov");
        MoveProperty(obj, "FrustumColor", &geometry, "frustumColor");
        MoveProperty(obj, "FrustumBaseColor", &geometry, "frustumBaseColor");
        MoveProperty(obj, "FrustumOpacity", &geometry, "frustumOpacity");
        MoveProperty(obj, "GridOpacity", &geometry, "gridOpacity");
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
TransformSscTrajectory(QVariantMap* obj)
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
            trajectory["type"] = "Sampled";
            MoveProperty(&properties, "Source", &trajectory, "source");
            MoveProperty(&properties, "Interpolation", &trajectory, "interpolation");
        }
    }
    else if (obj->contains("SampledOrbit"))
    {
        trajectory["type"] = "Sampled";
        trajectory["source"] = obj->value("SampledOrbit");
    }
    else if (obj->contains("EllipticalOrbit"))
    {
        trajectory["type"] = "Keplerian";
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
        rotationModel["type"] = "Sampled";
        rotationModel["source"] = obj->value("SampledOrientation");
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
        rotationModel["type"] = "FixedRotation";
    }
    else if (obj->contains("FixedAttitude"))
    {
    }
    else if (obj->contains("FixedQuaternion"))
    {
        rotationModel["type"] = "Quaternion";
        rotationModel["value"] = obj->value("Quaternion");
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
        }
        else if (properties.contains("TwoVector"))
        {
            QVariantMap frame;
            frame["type"] = "TwoVector";
            obj->insert(newName, frame);
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

        obj->remove(oldName);
    }

    return SscOk;
}


TransformSscStatus
TransformSscArc(QVariantMap* obj)
{
    TransformSscStatus status = SscOk;

    status = TransformSscTrajectory(obj);
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
