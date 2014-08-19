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

#ifndef _ADDON_H_
#define _ADDON_H_

#include <QString>
#include <QList>
#include <QStringList>

class AddOn
{
public:
    AddOn();
    ~AddOn();

    QString source() const
    {
        return m_source;
    }

    void setSource(const QString& source);

    QString title() const
    {
        return m_title;
    }

    void setTitle(const QString& title);

    QStringList objects() const
    {
        return m_objects;
    }

    void addObject(const QString& objectName);

    QStringList spiceKernels() const
    {
        return m_spiceKernels;
    }

    void setSpiceKernels(const QStringList& spiceKernels)
    {
        m_spiceKernels = spiceKernels;
    }

private:
    QString m_source;
    QString m_title;
    QStringList m_objects;
    QStringList m_spiceKernels;
};

#endif // _ADDON_H_
