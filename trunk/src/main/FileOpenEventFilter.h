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

#ifndef _FILE_OPEN_EVENT_FILTER_H_
#define _FILE_OPEN_EVENT_FILTER_H_

#include <QObject>
#include <QString>
#include <QEvent>


/** FileOpenEventFilter is a simple event filter that catches
  * Qt FileOpen events and emits a signal in response. Qt 4.7
  * only generates FileOpen events on the Mac. A FileOpen event
  * can be triggered by a user clicking on URL with a custom
  * scheme that is registered with the application. This can
  * cause the app to be launched if it isn't running already,
  * and the event may arrive before everything is completely
  * initialized. The event filter therefore stores the last
  * received URL string so that it can be handled once the
  * application is fully initialized.
  */
class FileOpenEventFilter : public QObject
{
    Q_OBJECT

public:
    FileOpenEventFilter()
    {
    }

    ~FileOpenEventFilter()
    {
    }

    bool eventFilter(QObject *obj, QEvent *event);

    QString lastUrl() const
    {
        return m_lastUrl;
    }

signals:
    void fileOpened(const QString& file);
    void urlOpened(const QString& url);

private:
    QString m_lastUrl;
};

#endif // _FILE_OPEN_EVENT_FILTER_H_
