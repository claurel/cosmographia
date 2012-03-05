// This file is part of Cosmographia.
//
// Copyright (C) 2010-2011 Chris Laurel <claurel@gmail.com>
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

#include "Addon.h"
#include "HelpCatalog.h"
#include <vesta/Universe.h>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QVariant>


class UniverseView;
class UniverseCatalog;
class UniverseLoader;
class HelpCatalog;

class UniverseCatalogObject;

class Cosmographia : public QMainWindow
{
    Q_OBJECT

public:
    Cosmographia();
    ~Cosmographia();

    void initialize();

    Q_PROPERTY(bool autoHideToolBar READ autoHideToolBar WRITE setAutoHideToolBar NOTIFY autoHideToolBarChanged);
    Q_PROPERTY(QString videoSize READ videoSize WRITE setVideoSize NOTIFY videoSizeChanged);

    Q_INVOKABLE void loadAddOn(const QString& source);
    Q_INVOKABLE void unloadAddOn(const QString& source);
    Q_INVOKABLE QVariant getSetting(const QString& key);
    Q_INVOKABLE QString formatNumber(double value, int precision);

    bool autoHideToolBar() const
    {
        return m_autoHideToolBar;
    }

    QString videoSize() const
    {
        return m_videoSize;
    }

public slots:
    void findObject();
    void setTime();
    void faster();
    void slower();
    void faster2();
    void slower2();
    void backDay();
    void forwardDay();
    void backYear();
    void forwardYear();
    void reverseTime();
    void about();
    void saveScreenShot();
    void recordVideo();
    void plotTrajectory();
    void plotTrajectoryObserver();
    void setStarStyle(QAction* action);
    void setStereoMode(QAction* action);
    void setTimeDisplay(QAction* action);

    void processReceivedResource(QNetworkReply* reply);
    void setAutoHideToolBar(bool enabled);
    void setVideoSize(const QString& videoSize);

    void activateCosmoUrl(const QString& url);

    void minimize();

signals:
    void autoHideToolBarChanged();
    void videoSizeChanged(const QString&);
    void announcementReceived(const QString& text);

protected:
    bool event(QEvent* event);

private slots:
    void setFullScreen(bool enabled);
    void loadCatalog();
    void unloadLastCatalog();
    void copyStateUrlToClipboard();

private:
    void initializeUniverse();

    void setupMenuBar();
    void loadSettings();
    void saveSettings();

    void removeBody(vesta::Entity* body);
    void removeBody(const QString& name);

    void updateUnloadAction();
    void loadCatalogFile(const QString& fileName);
    void loadStarNamesFile(const QString& fileName, vesta::StarCatalog* starCatalog);
    void loadGallery(const QString& fileName);

    void showCatalogErrorDialog(const QString& errorMessages);
    void showAnnouncement(const QString& text, const QDateTime& modifiedTime);

private:
    vesta::counted_ptr<vesta::Universe> m_universe;

    UniverseCatalog *m_catalog;
    UniverseView *m_view3d;
    UniverseLoader *m_loader;
    HelpCatalog* m_helpCatalog;

    QAction* m_fullScreenAction;

    QNetworkAccessManager* m_networkManager;

    QList<AddOn*> m_loadedAddOns;
    QAction* m_unloadLastCatalogAction;

    UniverseCatalogObject* m_catalogWrapper;

    bool m_autoHideToolBar;
    QString m_videoSize;
};

#endif // _COSMOGRAPHIA_H_
