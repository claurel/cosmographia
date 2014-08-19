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
