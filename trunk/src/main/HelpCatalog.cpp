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

#include "HelpCatalog.h"
#include "catalog/UniverseCatalog.h"
#include <QDir>
#include <QDebug>

using namespace vesta;


/** Create a new HelpCatalog. The catalog does not take ownership
  * of the universe catalog pointer.
  */
HelpCatalog::HelpCatalog(UniverseCatalog* universeCatalog, QObject* parent) :
    QObject(parent),
    m_universeCatalog(universeCatalog)
{
}


HelpCatalog::~HelpCatalog()
{
}


/** Load all HTML files in the specified path. Returns the number
  * of files successfully loaded.
  */
int
HelpCatalog::loadHelpFiles(const QString& path)
{
    QDir dir(path);

    QStringList filters;
    filters << "*.html" << "*.htm";

    int filesLoaded = 0;

    QFileInfoList helpFiles = dir.entryInfoList(filters);
    foreach (QFileInfo fileInfo, helpFiles)
    {
        QFile htmlFile(fileInfo.filePath());
        if (htmlFile.open(QIODevice::ReadOnly))
        {
            QString resourceName = fileInfo.baseName();
            m_helpResources[resourceName] = QString::fromUtf8(htmlFile.readAll());
        }
    }

    return filesLoaded;
}


/** Return the named help resource. Returns an empty string if the
  * resource isn't available.
  */
QString
HelpCatalog::getHelpText(const QString& name) const
{
    QString help = m_helpResources.value(name.toLower());
    if (help.isEmpty())
    {
        // No help available; see if the named object has a custom info resource and use
        // that. If not, create a default info page.
        Entity* body = m_universeCatalog->find(name, Qt::CaseInsensitive);
        if (body != NULL)
        {
            BodyInfo* info = m_universeCatalog->findInfo(body);
            if (info)
            {
                if (info->infoSource.startsWith("help:"))
                {
                    help = m_helpResources.value(info->infoSource.mid(5));
                }
                else if (!info->infoSource.isEmpty())
                {
                    QFile infoFile(info->infoSource);
                    if (infoFile.open(QFile::ReadOnly))
                    {
                        help = QString::fromLatin1(infoFile.readAll());
                    }
                }
            }
        }

        // Nothing worked. Create a default help page if an object with
        // the specified name is present in the catalog.
        if (help.isEmpty())
        {
            QString description = m_universeCatalog->getDescription(body);
            help = QString("<h1>%1</h1>%2").arg(QString::fromUtf8(body->name().c_str()), description);
        }
    }

    return help;
}
