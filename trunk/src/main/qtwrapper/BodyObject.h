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

#ifndef _QTWRAPPER_BODY_OBJECT_H_
#define _QTWRAPPER_BODY_OBJECT_H_

#include "VisualizerObject.h"
#include <vesta/Entity.h>
#include <QObject>


/** Qt wrapper for VESTA's Entity class
  */
class BodyObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(bool isEllipsoid READ isEllipsoid)
    Q_PROPERTY(bool bodyAxes READ bodyAxes WRITE setBodyAxes)
    Q_PROPERTY(bool frameAxes READ frameAxes WRITE setFrameAxes)
    Q_PROPERTY(bool velocityArrow READ velocityArrow WRITE setVelocityArrow)
    Q_PROPERTY(bool longLatGrid READ longLatGrid WRITE setLongLatGrid)

public:
    Q_INVOKABLE bool hasVisualizer(const QString& name) const;
    Q_INVOKABLE void removeVisualizer(const QString& name);
    Q_INVOKABLE void setVisualizer(const QString& name, VisualizerObject* visualizer);
    Q_INVOKABLE double distanceTo(BodyObject* other, double t);
    Q_INVOKABLE double relativeSpeed(BodyObject* other, double t);

public:
    BodyObject(vesta::Entity* body = NULL, QObject* parent = NULL);
    ~BodyObject();

    vesta::Entity* body() const
    {
        return m_body.ptr();
    }

    QString name() const;
    bool isEllipsoid() const;

    bool bodyAxes() const;
    void setBodyAxes(bool enabled);
    bool frameAxes() const;
    void setFrameAxes(bool enabled);
    bool velocityArrow() const;
    void setVelocityArrow(bool enabled);
    bool longLatGrid() const;
    void setLongLatGrid(bool enabled);

private:
    float visualizerSize() const;

private:
    vesta::counted_ptr<vesta::Entity> m_body;
};

#endif // _QTWRAPPER_BODY_OBJECT_H_
