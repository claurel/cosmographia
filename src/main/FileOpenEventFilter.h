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
