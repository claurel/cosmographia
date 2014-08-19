// This file is part of Cosmographia.
//
// Copyright (C) 2011-2012 Chris Laurel <claurel@gmail.com>
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

#ifndef _UNIVERSE_CATALOG_H_
#define _UNIVERSE_CATALOG_H_

#include "BodyInfo.h"
#include <vesta/Entity.h>
#include <QString>
#include <QStringList>
#include <QMap>

class Viewpoint;


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
