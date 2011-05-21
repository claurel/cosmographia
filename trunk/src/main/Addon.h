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

#ifndef _ADDON_H_
#define _ADDON_H_

#include <QString>
#include <QList>
#include <QStringList>

class AddOn
{
public:
    AddOn(const QString&);
    ~AddOn();

    QString fileName() const
    {
        return m_fileName;
    }

    QString title() const
    {
        return m_title;
    }

    QStringList objects() const
    {
        return m_objects;
    }

private:
    QString m_fileName;
    QString m_title;
    QStringList m_objects;
};

#endif // _ADDON_H_
