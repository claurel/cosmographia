// This file is part of Cosmographia.
//
// Copyright (C) 2010-2011 Chris Laurel <claurel@gmail.com>
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

#ifndef _COSMOGRAPHIA_H_
#define _COSMOGRAPHIA_H_

#include "Addon.h"
#include "HelpCatalog.h"
#include "UnitConversion.h"
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
    Q_PROPERTY(QString measurementSystem READ measurementSystem WRITE setMeasurementSystem NOTIFY measurementSystemChanged);

    Q_INVOKABLE void loadAddOn(const QString& source);
    Q_INVOKABLE void unloadAddOn(const QString& source);
    Q_INVOKABLE QVariant getSetting(const QString& key);
    Q_INVOKABLE QString formatNumber(double value, int precision);
    Q_INVOKABLE QString formatDistance(double value, int precision);
    Q_INVOKABLE QString formatSpeed(double value, int precision);

    bool autoHideToolBar() const
    {
        return m_autoHideToolBar;
    }

    QString videoSize() const
    {
        return m_videoSize;
    }

    QString measurementSystem() const;
    void setMeasurementSystem(const QString& ms);

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
    void measurementSystemChanged(const QString&);
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
