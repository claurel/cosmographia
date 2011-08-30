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

#include "FileOpenEventFilter.h"
#include <QFileOpenEvent>
#include <QUrl>

bool
FileOpenEventFilter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::FileOpen)
    {
        QFileOpenEvent* fileEvent = static_cast<QFileOpenEvent*>(event);
        if (!fileEvent->url().isEmpty())
        {
            m_lastUrl = fileEvent->url().toString();
            emit urlOpened(m_lastUrl);
        }
        else if (!fileEvent->file().isEmpty())
        {
            emit fileOpened(fileEvent->file());
        }

        return false;
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}
