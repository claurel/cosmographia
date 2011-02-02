// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
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

#ifndef _COSMOGRAPHIA_H_
#define _COSMOGRAPHIA_H_

#include <QMainWindow>


class UniverseView;
class UniverseCatalog;

class Cosmographia : public QMainWindow
{
    Q_OBJECT

public:
    Cosmographia();
    ~Cosmographia();

public slots:
    void faster();
    void slower();
    void faster2();
    void slower2();
    void backYear();
    void forwardYear();
    void reverseTime();
    void about();
    void saveScreenShot();
    void recordVideo();

private slots:
    void setLabelMode(QAction* action);
    void setFullScreen(bool enabled);
    void loadSolarSystem();

private:
    void loadSettings();
    void saveSettings();

private:
    UniverseCatalog *m_catalog;
    UniverseView *m_view3d;

    QAction* m_fullScreenAction;
};

#endif // _COSMOGRAPHIA_H_
