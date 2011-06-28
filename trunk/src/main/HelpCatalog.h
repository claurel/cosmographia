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

#ifndef _HELP_CATALOG_H_
#define _HELP_CATALOG_H_

#include <QObject>
#include <QMap>


class HelpCatalog : public QObject
{
Q_OBJECT
public:
    HelpCatalog(QObject* parent = NULL);
    ~HelpCatalog();

    int loadHelpFiles(const QString& path);

    Q_INVOKABLE QString getHelpText(const QString& name) const;

private:
    QMap<QString, QString> m_helpResources;
};

#endif // _HELP_CATALOG_H_
