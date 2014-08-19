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

#ifndef _HELP_CATALOG_H_
#define _HELP_CATALOG_H_

#include <QObject>
#include <QMap>

class UniverseCatalog;

class HelpCatalog : public QObject
{
Q_OBJECT
public:
    HelpCatalog(UniverseCatalog* catalog, QObject* parent = NULL);
    ~HelpCatalog();

    int loadHelpFiles(const QString& path);

    void setHelpText(const QString& name, const QString& text);
    Q_INVOKABLE QString getHelpText(const QString& name) const;

    QString getObjectDataText(const QString& name) const;

private:
    QMap<QString, QString> m_helpResources;
    UniverseCatalog* m_universeCatalog;
};

#endif // _HELP_CATALOG_H_
