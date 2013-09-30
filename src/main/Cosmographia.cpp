// This file is part of Cosmographia.
//
// Copyright (C) 2010-2011 Chris Laurel <claurel@gmail.com>
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

#include <QtGui>

#include "UniverseView.h"
#include "GalleryView.h"
#include "catalog/UniverseCatalog.h"
#include "catalog/UniverseLoader.h"
#include "qtwrapper/UniverseCatalogObject.h"
#include "Cosmographia.h"
#if FFMPEG_SUPPORT
#include "QVideoEncoder.h"
#elif QTKIT_SUPPORT
#include "../video/VideoEncoder.h"
#endif
#include "JPLEphemeris.h"
#include "NetworkTextureLoader.h"
#include "LinearCombinationTrajectory.h"
#include "astro/IAULunarRotationModel.h"
#include "astro/MarsSat.h"
#include "astro/L1.h"
#include "astro/TASS17.h"
#include "astro/Gust86.h"
#include "DateUtility.h"
#include "NumberFormat.h"
#include "SkyLabelLayer.h"
#include <vesta/GregorianDate.h>
#include <vesta/Body.h>
#include <vesta/Arc.h>
#include <vesta/Trajectory.h>
#include <vesta/Frame.h>
#include <vesta/InertialFrame.h>
#include <vesta/RotationModel.h>
#include <vesta/KeplerianTrajectory.h>
#include <vesta/FixedPointTrajectory.h>
#include <vesta/WorldGeometry.h>
#include <vesta/Units.h>
#include <vesta/StarsLayer.h>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <algorithm>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QComboBox>
#include <QLabel>
#include <QCompleter>
#include <QDialog>
#include <QMessageBox>
#include <QApplication>
#include <QFileDialog>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QBoxLayout>
#include <QStackedLayout>
#include <QDialogButtonBox>
#include <QDesktopServices>
#include <QNetworkDiskCache>
#include <QNetworkRequest>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>

using namespace vesta;
using namespace Eigen;


// Set this to 1 to enable the Solar System gallery view
#define ENABLE_GALLERY 1


Cosmographia::Cosmographia() :
    QMainWindow(NULL),
    m_catalog(NULL),
    m_view3d(NULL),
    m_loader(NULL),
    m_helpCatalog(NULL),
    m_fullScreenAction(NULL),
    m_networkManager(NULL),
    m_catalogWrapper(NULL),
    m_autoHideToolBar(false),
    m_videoSize("wvga")
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAutoFillBackground(false);
    setBackgroundRole(QPalette::Window);
    QPalette newPalette = palette();
    newPalette.setColor(QPalette::Window, Qt::black);
    setPalette(newPalette);

    initializeUniverse();

    m_catalog = new UniverseCatalog();
    m_view3d = new UniverseView(this, m_universe.ptr(), m_catalog);
    m_loader = new UniverseLoader();

    loadStarNamesFile("starnames.json", m_universe->starCatalog());

    m_helpCatalog = new HelpCatalog(m_catalog);
    m_helpCatalog->loadHelpFiles("./help");

    m_catalogWrapper = new UniverseCatalogObject(m_catalog);

    // Initialize QML types
    qmlRegisterUncreatableType<Cosmographia>("Cosmographia", 1, 0, "Cosmographia", "Use global cosmoApp");
    qmlRegisterUncreatableType<UniverseView>("Cosmographia", 1, 0, "UniverseView", "Use global universeView");
    qmlRegisterUncreatableType<HelpCatalog>("Cosmographia", 1, 0, "HelpCatalog", "Use global helpCatalog");
    qmlRegisterUncreatableType<UniverseCatalogObject>("Cosmographia", 1, 0, "UniverseCatalog", "Use global universeCatalog");
    qmlRegisterType<BodyObject>("Cosmographia", 1, 0, "Body");
    qmlRegisterType<VisualizerObject>("Cosmographia", 1, 0, "Visualizer");
    m_view3d->rootContext()->setContextProperty("cosmoApp", this);
    m_view3d->rootContext()->setContextProperty("universeView", m_view3d);
    m_view3d->rootContext()->setContextProperty("universeCatalog", m_catalogWrapper);
    m_view3d->rootContext()->setContextProperty("helpCatalog", m_helpCatalog);

    setCentralWidget(m_view3d);

    setWindowTitle(tr("Cosmographia"));

    m_fullScreenAction = new QAction("Full Screen", this);
    m_fullScreenAction->setShortcut(QKeySequence("Ctrl+Shift+F"));
    m_fullScreenAction->setCheckable(true);

    loadSettings();

    // Set up the UI *after* settings are loaded so that the
    // controls are sync'ed
    m_view3d->initializeDeclarativeUi("qml/main.qml");

    setupMenuBar();

    setCursor(QCursor(Qt::CrossCursor));
}


Cosmographia::~Cosmographia()
{
    saveSettings();
    delete m_catalogWrapper;
}


void
Cosmographia::setupMenuBar()
{
    /*** File Menu ***/
    QMenu* fileMenu = new QMenu("&File", this);
    QAction* saveScreenShotAction = new QAction("&Save Screenshot", this);
    QAction* copyScreenShotAction = new QAction("&Copy Screenshot to Clipboard", this);
    copyScreenShotAction->setShortcut(QKeySequence("Shift+Ctrl+C"));
    QAction* recordVideoAction = new QAction("&Record Video", this);
    recordVideoAction->setShortcut(QKeySequence("Ctrl+R"));
#if !FFMPEG_SUPPORT && !QTKIT_SUPPORT
    recordVideoAction->setEnabled(false);
#endif
    fileMenu->addAction(saveScreenShotAction);
    fileMenu->addAction(recordVideoAction);
    fileMenu->addSeparator();
    QAction* loadCatalogAction = fileMenu->addAction("&Open Catalog...");
    loadCatalogAction->setShortcut(QKeySequence("Ctrl+O"));
    m_unloadLastCatalogAction = fileMenu->addAction("&Unload Last Catalog");
    m_unloadLastCatalogAction->setDisabled(true);
    m_unloadLastCatalogAction->setShortcut(QKeySequence("Ctrl+W"));
    fileMenu->addSeparator();
    QAction* quitAction = fileMenu->addAction("&Quit");
    menuBar()->addMenu(fileMenu);

    connect(saveScreenShotAction, SIGNAL(triggered()), this, SLOT(saveScreenShot()));
    connect(copyScreenShotAction, SIGNAL(triggered()), m_view3d, SLOT(copyNextFrameToClipboard()));
    connect(recordVideoAction, SIGNAL(triggered()), this, SLOT(recordVideo()));
    connect(loadCatalogAction, SIGNAL(triggered()), this, SLOT(loadCatalog()));
    connect(m_unloadLastCatalogAction, SIGNAL(triggered()), this, SLOT(unloadLastCatalog()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    QAction* copyStateUrlAction = new QAction("Copy Viewpoint &URL", this);
    copyStateUrlAction->setShortcut(QKeySequence("Ctrl+U"));
    connect(copyStateUrlAction, SIGNAL(triggered()), this, SLOT(copyStateUrlToClipboard()));

    /*** Time Menu ***/
    QMenu* timeMenu = new QMenu("&Time", this);
    QAction* setTimeAction = new QAction("Set &Time...", this);
    setTimeAction->setShortcut(QKeySequence("Ctrl+T"));
    timeMenu->addAction(setTimeAction);
    QAction* nowAction = new QAction("&Current time", this);
    timeMenu->addAction(nowAction);
    menuBar()->addMenu(timeMenu);

    QMenu* timeDisplayMenu = new QMenu("&Time Display", this);
    QActionGroup* timeDisplayGroup = new QActionGroup(this);
    QAction* utcAction = new QAction("UTC", timeDisplayGroup);
    utcAction->setCheckable(true);
    utcAction->setChecked(true);
    utcAction->setData(int(UniverseView::TimeDisplay_UTC));
    timeDisplayMenu->addAction(utcAction);
    QAction* localAction = new QAction("Local", timeDisplayGroup);
    localAction->setCheckable(true);
    localAction->setData(int(UniverseView::TimeDisplay_Local));
    timeDisplayMenu->addAction(localAction);
    QAction* multipleTimeAction = new QAction("Multiple", timeDisplayGroup);
    multipleTimeAction->setCheckable(true);
    multipleTimeAction->setData(int(UniverseView::TimeDisplay_Multiple));
    timeDisplayMenu->addAction(multipleTimeAction);
    timeMenu->addMenu(timeDisplayMenu);
    connect(timeDisplayGroup, SIGNAL(selected(QAction*)), this, SLOT(setTimeDisplay(QAction*)));

    timeMenu->addSeparator();
    QAction* pauseAction = new QAction("&Pause", this);
    pauseAction->setCheckable(true);
    pauseAction->setShortcut(Qt::Key_Space);
    timeMenu->addAction(pauseAction);
    QAction* fasterAction = new QAction("&Faster", this);
    fasterAction->setShortcut(QKeySequence("Ctrl+L"));
    timeMenu->addAction(fasterAction);
    QAction* slowerAction = new QAction("&Slower", this);
    slowerAction->setShortcut(QKeySequence("Ctrl+K"));
    timeMenu->addAction(slowerAction);
    QAction* faster2Action = new QAction("2x Faster", this);
    faster2Action->setShortcut(QKeySequence("Ctrl+Shift+L"));
    timeMenu->addAction(faster2Action);
    QAction* slower2Action = new QAction("2x Slower", this);
    slower2Action->setShortcut(QKeySequence("Ctrl+Shift+K"));
    timeMenu->addAction(slower2Action);
    QAction* backDayAction = new QAction("Back One Day", this);
    backDayAction->setShortcut(QKeySequence("Ctrl+["));
    timeMenu->addAction(backDayAction);
    QAction* forwardDayAction = new QAction("Forward One Day", this);
    forwardDayAction->setShortcut(QKeySequence("Ctrl+]"));
    timeMenu->addAction(forwardDayAction);
    QAction* backYearAction = new QAction("Back One Year", this);
    backYearAction->setShortcut(QKeySequence("Ctrl+Shift+["));
    timeMenu->addAction(backYearAction);
    QAction* forwardYearAction = new QAction("Forward One Year", this);
    forwardYearAction->setShortcut(QKeySequence("Ctrl+Shift+]"));
    timeMenu->addAction(forwardYearAction);
    QAction* reverseAction = new QAction("&Reverse", this);
    reverseAction->setShortcut(QKeySequence("Ctrl+J"));
    timeMenu->addAction(reverseAction);

    connect(setTimeAction, SIGNAL(triggered()),     this,     SLOT(setTime()));
    connect(pauseAction,   SIGNAL(triggered(bool)), m_view3d, SLOT(setPaused(bool)));
    connect(fasterAction,  SIGNAL(triggered()),     this,     SLOT(faster()));
    connect(slowerAction,  SIGNAL(triggered()),     this,     SLOT(slower()));
    connect(faster2Action,  SIGNAL(triggered()),     this,     SLOT(faster2()));
    connect(slower2Action,  SIGNAL(triggered()),     this,     SLOT(slower2()));
    connect(backDayAction,  SIGNAL(triggered()),     this,     SLOT(backDay()));
    connect(forwardDayAction,  SIGNAL(triggered()),     this,     SLOT(forwardDay()));
    connect(backYearAction,  SIGNAL(triggered()),     this,     SLOT(backYear()));
    connect(forwardYearAction,  SIGNAL(triggered()),     this,     SLOT(forwardYear()));
    connect(reverseAction, SIGNAL(triggered()),     this,     SLOT(reverseTime()));
    connect(nowAction,     SIGNAL(triggered()),     m_view3d, SLOT(setCurrentTime()));

    /*** Camera Menu ***/
    QMenu* cameraMenu = new QMenu("&Camera", this);

    QAction* findAction = new QAction("&Find Object...", cameraMenu);
    findAction->setShortcut(QKeySequence("Ctrl+F"));
    cameraMenu->addAction(findAction);
    QAction* centerAction = new QAction("Set &Center", cameraMenu);
    centerAction->setShortcut(QKeySequence("Ctrl+C"));
    cameraMenu->addAction(centerAction);
    QAction* gotoAction = new QAction("&Goto Selected Object", cameraMenu);
    gotoAction->setShortcut(QKeySequence("Ctrl+G"));
    cameraMenu->addAction(gotoAction);

    QActionGroup* cameraFrameGroup = new QActionGroup(cameraMenu);
    QAction* inertialAction = new QAction("&Inertial Frame", cameraFrameGroup);
    inertialAction->setShortcut(QKeySequence("Ctrl+I"));
    inertialAction->setCheckable(true);
    inertialAction->setChecked(true);
    cameraMenu->addAction(inertialAction);
    QAction* bodyFixedAction = new QAction("&Body Fixed Frame", cameraFrameGroup);
    bodyFixedAction->setShortcut(QKeySequence("Ctrl+B"));
    bodyFixedAction->setCheckable(true);
    cameraMenu->addAction(bodyFixedAction);
    QAction* synodicAction = new QAction("&Synodic Frame", cameraFrameGroup);
    synodicAction->setShortcut(QKeySequence("Ctrl+Y"));
    synodicAction->setCheckable(true);
    cameraMenu->addAction(synodicAction);
    QAction* lockedAction = new QAction("&Locked Frame", cameraFrameGroup);
    lockedAction->setShortcut(QKeySequence("Ctrl+Shift+Y"));
    lockedAction->setCheckable(true);
    cameraMenu->addAction(lockedAction);

    menuBar()->addMenu(cameraMenu);

    connect(findAction,      SIGNAL(triggered()),     m_view3d, SLOT(findObject()));
    connect(centerAction,    SIGNAL(triggered()),     m_view3d, SLOT(setObserverCenter()));
    connect(gotoAction,      SIGNAL(triggered()),     m_view3d, SLOT(gotoSelectedObject()));
    connect(inertialAction,  SIGNAL(triggered(bool)), m_view3d, SLOT(inertialObserver(bool)));
    connect(bodyFixedAction, SIGNAL(triggered(bool)), m_view3d, SLOT(bodyFixedObserver(bool)));
    connect(synodicAction,   SIGNAL(triggered(bool)), m_view3d, SLOT(synodicObserver(bool)));
    connect(lockedAction,    SIGNAL(triggered(bool)), m_view3d, SLOT(lockedObserver(bool)));

    /*** Visual aids menu ***/
    QMenu* visualAidsMenu = new QMenu("&Visualization", this);

    QAction* eqGridAction = new QAction("E&quatorial Grid", visualAidsMenu);
    eqGridAction->setCheckable(true);
    visualAidsMenu->addAction(eqGridAction);
    QAction* eclipticAction = new QAction("&Ecliptic", visualAidsMenu);
    eclipticAction->setCheckable(true);
    visualAidsMenu->addAction(eclipticAction);
    visualAidsMenu->addSeparator();
    QAction* labelsAction = new QAction("&Labels", visualAidsMenu);
    labelsAction->setCheckable(true);
    labelsAction->setChecked(true);
    visualAidsMenu->addAction(labelsAction);
    QAction* figuresAction = new QAction("Constellation &Figures", visualAidsMenu);
    figuresAction->setCheckable(true);
    visualAidsMenu->addAction(figuresAction);
    QAction* constellationNamesAction = new QAction("Constellation &Names", visualAidsMenu);
    constellationNamesAction->setCheckable(true);
    visualAidsMenu->addAction(constellationNamesAction);
    visualAidsMenu->addSeparator();

    QAction* planetOrbitsAction = new QAction("Planet &Orbits", visualAidsMenu);
    planetOrbitsAction->setShortcut(QKeySequence("Ctrl+P"));
    planetOrbitsAction->setCheckable(true);
    planetOrbitsAction->setChecked(m_view3d->planetOrbitsVisibility());
    visualAidsMenu->addAction(planetOrbitsAction);
    QAction* plotTrajectoryAction = new QAction("&Plot Trajectory", visualAidsMenu);
    plotTrajectoryAction->setShortcut(QKeySequence("Shift+Ctrl+P"));
    visualAidsMenu->addAction(plotTrajectoryAction);
    QAction* plotTrajectoryObserverAction = new QAction("&Plot Trajectory in Observer Frame", visualAidsMenu);
    plotTrajectoryObserverAction->setShortcut(QKeySequence("Shift+Alt+Ctrl+P"));
    visualAidsMenu->addAction(plotTrajectoryObserverAction);

    visualAidsMenu->addSeparator();
    QAction* infoTextAction = new QAction("Info text", visualAidsMenu);
    infoTextAction->setCheckable(true);
    infoTextAction->setChecked(true);
    visualAidsMenu->addAction(infoTextAction);

    menuBar()->addMenu(visualAidsMenu);

    connect(eqGridAction,   SIGNAL(triggered(bool)), m_view3d, SLOT(setEquatorialGridVisibility(bool)));
    connect(eclipticAction, SIGNAL(triggered(bool)), m_view3d, SLOT(setEclipticVisibility(bool)));
    connect(labelsAction,   SIGNAL(triggered(bool)), m_view3d, SLOT(setLabelVisibility(bool)));
    connect(figuresAction,  SIGNAL(triggered(bool)), m_view3d, SLOT(setConstellationFigureVisibility(bool)));
    connect(constellationNamesAction,  SIGNAL(triggered(bool)), m_view3d, SLOT(setConstellationNameVisibility(bool)));

    connect(planetOrbitsAction, SIGNAL(triggered(bool)), m_view3d, SLOT(setPlanetOrbitsVisibility(bool)));
    connect(plotTrajectoryAction, SIGNAL(triggered()), this, SLOT(plotTrajectory()));
    connect(plotTrajectoryObserverAction, SIGNAL(triggered()), this, SLOT(plotTrajectoryObserver()));
    connect(infoTextAction, SIGNAL(triggered(bool)), m_view3d, SLOT(setInfoText(bool)));

    /*** Star style menu ***/
    QMenu* starStyleMenu = new QMenu("Star Style");
    QActionGroup* starStyleGroup = new QActionGroup(starStyleMenu);
    QAction* pointStarsAction = new QAction("&Points", starStyleGroup);
    pointStarsAction->setCheckable(true);
    pointStarsAction->setData(0);
    starStyleMenu->addAction(pointStarsAction);
    QAction* gaussianStarsAction = new QAction("&Gaussian", starStyleGroup);
    gaussianStarsAction->setCheckable(true);
    gaussianStarsAction->setChecked(true);
    gaussianStarsAction->setData(1);
    starStyleMenu->addAction(gaussianStarsAction);
    QAction* diffractionSpikeStarsAction = new QAction("Gaussian with &diffraction spikes", starStyleGroup);
    diffractionSpikeStarsAction->setCheckable(true);
    diffractionSpikeStarsAction->setData(2);
    starStyleMenu->addAction(diffractionSpikeStarsAction);

    /*** Stereo mode menu ***/
    QMenu* stereoModeMenu = new QMenu("Stereo Mode");
    QActionGroup* stereoModeGroup = new QActionGroup(stereoModeMenu);
    QAction* monoAction = new QAction("Stereo Disabled", stereoModeGroup);
    monoAction->setCheckable(true);
    monoAction->setChecked(true);
    monoAction->setData((int) UniverseView::Mono);
    monoAction->setShortcut(QKeySequence("Shift+Ctrl+M"));
    stereoModeMenu->addAction(monoAction);
    QAction* anaglyphRedCyanAction = new QAction("Anaglyph (Red-Cyan)", stereoModeGroup);
    anaglyphRedCyanAction->setCheckable(true);
    anaglyphRedCyanAction->setData((int) UniverseView::AnaglyphRedCyan);
    anaglyphRedCyanAction->setShortcut(QKeySequence("Shift+Ctrl+A"));
    stereoModeMenu->addAction(anaglyphRedCyanAction);
    QAction* sideBySideAction = new QAction("Side-by-side", stereoModeGroup);
    sideBySideAction->setCheckable(true);
    sideBySideAction->setData((int) UniverseView::SideBySide);
    sideBySideAction->setShortcut(QKeySequence("Shift+Ctrl+S"));
    stereoModeMenu->addAction(sideBySideAction);

    /*** Graphics menu ***/
    QMenu* graphicsMenu = new QMenu("&Graphics", this);
    QAction* shadowsAction = new QAction("&Shadows", graphicsMenu);
    shadowsAction->setCheckable(true);
    shadowsAction->setChecked(m_view3d->shadows());
    graphicsMenu->addAction(shadowsAction);
    QAction* eclipsesAction = new QAction("&Eclipse Shadows", graphicsMenu);
    eclipsesAction->setCheckable(true);
    eclipsesAction->setChecked(m_view3d->eclipseShadows());
    graphicsMenu->addAction(eclipsesAction);
    QAction* atmospheresAction = new QAction("&Atmosphere", graphicsMenu);
    atmospheresAction->setCheckable(true);
    atmospheresAction->setChecked(m_view3d->atmospheresVisible());
    atmospheresAction->setShortcut(QKeySequence("Ctrl+A"));
    graphicsMenu->addAction(atmospheresAction);
    QAction* cloudLayerAction = new QAction("&Cloud Layers", graphicsMenu);
    cloudLayerAction->setCheckable(true);
    cloudLayerAction->setChecked(m_view3d->cloudsVisible());
    graphicsMenu->addAction(cloudLayerAction);
    QAction* ambientLightAction = new QAction("Extra &Light", graphicsMenu);
    ambientLightAction->setCheckable(true);
    ambientLightAction->setChecked(m_view3d->ambientLight() > 0.0);
    graphicsMenu->addAction(ambientLightAction);
    QAction* sunGlareAction = new QAction("Sun &Glare", graphicsMenu);
    sunGlareAction->setCheckable(true);
    sunGlareAction->setChecked(m_view3d->sunGlare());
    graphicsMenu->addAction(sunGlareAction);
    QAction* reflectionsAction = new QAction("&Reflections", graphicsMenu);
    reflectionsAction->setCheckable(true);
    reflectionsAction->setChecked(m_view3d->reflections());
    graphicsMenu->addAction(reflectionsAction);
    QAction* milkyWayAction = new QAction("&Milky Way", graphicsMenu);
    milkyWayAction->setCheckable(true);
    milkyWayAction->setChecked(m_view3d->milkyWayVisible());
    //milkyWayAction->setShortcut(QKeySequence("Ctrl+M"));
    graphicsMenu->addAction(milkyWayAction);
    graphicsMenu->addMenu(starStyleMenu);
    graphicsMenu->addSeparator();
    graphicsMenu->addAction(m_fullScreenAction);
    connect(m_fullScreenAction, SIGNAL(toggled(bool)), this, SLOT(setFullScreen(bool)));
    graphicsMenu->addMenu(stereoModeMenu);

    menuBar()->addMenu(graphicsMenu);

    connect(shadowsAction,          SIGNAL(triggered(bool)), m_view3d, SLOT(setShadows(bool)));
    connect(eclipsesAction,         SIGNAL(triggered(bool)), m_view3d, SLOT(setEclipseShadows(bool)));
    connect(atmospheresAction,      SIGNAL(triggered(bool)), m_view3d, SLOT(setAtmospheresVisible(bool)));
    connect(cloudLayerAction,       SIGNAL(triggered(bool)), m_view3d, SLOT(setCloudsVisible(bool)));
    connect(ambientLightAction,     SIGNAL(triggered(bool)), m_view3d, SLOT(setAmbientLight(bool)));
    connect(sunGlareAction,         SIGNAL(triggered(bool)), m_view3d, SLOT(setSunGlare(bool)));
    connect(reflectionsAction,      SIGNAL(triggered(bool)), m_view3d, SLOT(setReflections(bool)));
    connect(milkyWayAction,         SIGNAL(triggered(bool)), m_view3d, SLOT(setMilkyWayVisible(bool)));
    connect(starStyleGroup,         SIGNAL(selected(QAction*)), this, SLOT(setStarStyle(QAction*)));
    connect(stereoModeGroup,        SIGNAL(selected(QAction*)), this, SLOT(setStereoMode(QAction*)));

    /*** Help menu ***/
    QMenu* helpMenu = new QMenu("Help", this);

    QAction* aboutAction = new QAction("About QtCosmographia", helpMenu);
    helpMenu->addAction(aboutAction);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    menuBar()->addMenu(helpMenu);

    QAction* galleryAction = new QAction("Worlds Gallery", this);
    galleryAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(galleryAction, SIGNAL(triggered()), m_view3d, SLOT(toggleGallery()));

#ifdef Q_OS_MAC
    QAction* minimizeAction = new QAction("Minimize", this);
    minimizeAction->setShortcut(QKeySequence("Ctrl+M"));
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(minimize()));
    addAction(minimizeAction);
#endif

#if NOMENUBAR
    // Cosmographia may be set up to work without a menu bar in full screen mode.
    // In that case, we want to add some actions to the main window so that keyboard
    // shortcuts are still available.
    setMenuBar(NULL);
    addAction(quitAction);
    addAction(loadCatalogAction);
    addAction(m_unloadLastCatalogAction);
    addAction(m_fullScreenAction);
    addAction(copyScreenShotAction);
    addAction(copyStateUrlAction);
    addAction(recordVideoAction);
    addAction(pauseAction);
    addAction(fasterAction);
    addAction(slowerAction);
    addAction(faster2Action);
    addAction(slower2Action);
    addAction(reverseAction);
    addAction(forwardDayAction);
    addAction(backDayAction);
    addAction(forwardYearAction);
    addAction(backYearAction);
    addAction(findAction);
    addAction(gotoAction);
    addAction(centerAction);
    addAction(inertialAction);
    addAction(bodyFixedAction);
    addAction(planetOrbitsAction);
    addAction(plotTrajectoryAction);
    addAction(galleryAction);
#else
    // These should go into the edit menu, but there currently
    // isn't one.
    addAction(copyScreenShotAction);
    addAction(copyStateUrlAction);
    addAction(galleryAction);
#endif // NOMENUBAR
}


void
Cosmographia::initializeUniverse()
{
    m_universe = new Universe();
    m_universe->addRef();

    double duration = daysToSeconds(365.25);

    vesta::Arc* arc = NULL;

    // Create the solar system barycenter
    Entity* ssb = new Entity();
    arc = new vesta::Arc();
    arc->setDuration(duration);
    ssb->chronology()->addArc(arc);
    m_universe->addEntity(ssb);

    // Create the Sun
    Body* sun = new Body();
    sun->setName("Sun");

    arc = new vesta::Arc();
    arc->setCenter(ssb);
    arc->setDuration(daysToSeconds(365.25 * 200.0));
    sun->chronology()->setBeginning(0.0);
    sun->chronology()->addArc(arc);

    m_universe->addEntity(sun);

    StarCatalog* stars = NULL;
    QFile starFile("tycho2.stars");
    if (starFile.open(QFile::ReadOnly))
    {
        stars = new StarCatalog();
        QDataStream in(&starFile);
        in.setVersion(QDataStream::Qt_4_4);
        in.setByteOrder(QDataStream::BigEndian);

        bool ok = true;
        while (ok)
        {
            quint32 id = 0;
            float ra = 0.0f;
            float dec = 0.0f;
            float vmag = 0.0f;
            float bv = 0.0f;
            in >> id >> ra >> dec >> vmag >> bv;

            // Constrain maximum B-V color index; conversion to RGB color is not
            // valid for large values. TODO: Fix this in VESTA
            if (bv > 2.5f)
            {
                bv = 2.5f;
            }

            if (in.status() == QDataStream::Ok)
            {
                stars->addStar(id, (float) toRadians(ra), (float) toRadians(dec), vmag, bv);
            }
            else
            {
                ok = false;
            }
        }

        stars->buildCatalogIndex();
        m_universe->setStarCatalog(stars);
    }
}


// Convert a JPL ephemeris orbit from SSB-centered to Sun-centered
static Trajectory*
createSunRelativeTrajectory(const JPLEphemeris* eph, JPLEphemeris::JplObjectId id)
{
    LinearCombinationTrajectory* orbit = new LinearCombinationTrajectory(eph->trajectory(id), 1.0,
                                                                         eph->trajectory(JPLEphemeris::Sun), -1.0);
    orbit->setPeriod(eph->trajectory(id)->period());
    return orbit;
}


static QString cacheFilePath(const QString& fileName)
{
#if 0
    return QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + "/catalog";
#else
    return QStandardPaths::locate(QStandardPaths::CacheLocation, fileName, QStandardPaths::LocateFile);
#endif
}

static QString cacheDirectoryPath(const QString& subdirName)
{
#if 0
    return QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + "/catalog";
#else
    return QStandardPaths::locate(QStandardPaths::CacheLocation, subdirName, QStandardPaths::LocateDirectory);
#endif
}

static QString pictureFilePath(const QString& fileName)
{
#if 0
    return QDesktopServices::storageLocation(QDesktopServices::PicturesLocation) + "/" + fileName;
#else
    return QStandardPaths::locate(QStandardPaths::PicturesLocation, fileName, QStandardPaths::LocateFile);
#endif
}

static QString documentFilePath(const QString& fileName)
{
#if 0
    return QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/" + fileName;
#else
    return QStandardPaths::locate(QStandardPaths::DocumentsLocation, fileName, QStandardPaths::LocateFile);
#endif
}


/** Perform once-per-run initialization, such as loading planetary ephemerides.
  */
void
Cosmographia::initialize()
{
    // Set up builtin orbits
    JPLEphemeris* eph = JPLEphemeris::load("de406_1800-2100.dat");
    if (eph)
    {
        m_loader->addBuiltinOrbit("Sun",     eph->trajectory(JPLEphemeris::Sun));
        m_loader->addBuiltinOrbit("Moon",    eph->trajectory(JPLEphemeris::Moon));

        // The code below will create planet trajectories relative to the SSB
        /*
        m_loader->addBuiltinOrbit("Mercury", eph->trajectory(JPLEphemeris::Mercury));
        m_loader->addBuiltinOrbit("Venus",   eph->trajectory(JPLEphemeris::Venus));
        m_loader->addBuiltinOrbit("EMB",     eph->trajectory(JPLEphemeris::EarthMoonBarycenter));
        m_loader->addBuiltinOrbit("Mars",    eph->trajectory(JPLEphemeris::Mars));
        m_loader->addBuiltinOrbit("Jupiter", eph->trajectory(JPLEphemeris::Jupiter));
        m_loader->addBuiltinOrbit("Saturn",  eph->trajectory(JPLEphemeris::Saturn));
        m_loader->addBuiltinOrbit("Uranus",  eph->trajectory(JPLEphemeris::Uranus));
        m_loader->addBuiltinOrbit("Neptune", eph->trajectory(JPLEphemeris::Neptune));
        m_loader->addBuiltinOrbit("Pluto",   eph->trajectory(JPLEphemeris::Pluto));
        */

        Trajectory* embTrajectory = createSunRelativeTrajectory(eph, JPLEphemeris::EarthMoonBarycenter);
        m_loader->addBuiltinOrbit("EMB", embTrajectory);

        m_loader->addBuiltinOrbit("Mercury", createSunRelativeTrajectory(eph, JPLEphemeris::Mercury));
        m_loader->addBuiltinOrbit("Venus",   createSunRelativeTrajectory(eph, JPLEphemeris::Venus));
        m_loader->addBuiltinOrbit("Mars",    createSunRelativeTrajectory(eph, JPLEphemeris::Mars));
        m_loader->addBuiltinOrbit("Jupiter", createSunRelativeTrajectory(eph, JPLEphemeris::Jupiter));
        m_loader->addBuiltinOrbit("Saturn",  createSunRelativeTrajectory(eph, JPLEphemeris::Saturn));
        m_loader->addBuiltinOrbit("Uranus",  createSunRelativeTrajectory(eph, JPLEphemeris::Uranus));
        m_loader->addBuiltinOrbit("Neptune", createSunRelativeTrajectory(eph, JPLEphemeris::Neptune));
        m_loader->addBuiltinOrbit("Pluto",   createSunRelativeTrajectory(eph, JPLEphemeris::Pluto));

        // m = the ratio of the Moon's to the mass of the Earth-Moon system
        double m = 1.0 / (1.0 + eph->earthMoonMassRatio());
        LinearCombinationTrajectory* earthTrajectory =
                new LinearCombinationTrajectory(embTrajectory, 1.0,
                                                eph->trajectory(JPLEphemeris::Moon), -m);
        earthTrajectory->setPeriod(embTrajectory->period());
        m_loader->addBuiltinOrbit("Earth", earthTrajectory);

        // JPL HORIZONS results for position of Moon with respect to Earth at 1 Jan 2000 12:00
        // position: -2.916083884571964E+05 -2.667168292374240E+05 -7.610248132320160E+04
        // velocity:  6.435313736079528E-01 -6.660876955662288E-01 -3.013257066079174E-01
        //std::cout << "Moon @ J2000:  " << eph->trajectory(JPLEphemeris::Moon)->position(0.0).transpose().format(16) << std::endl;

        // JPL HORIZONS results for position of Earth with respect to Sun at 1 Jan 2000 12:00
        // position: -2.649903422886233E+07  1.327574176646856E+08  5.755671744790662E+07
        // velocity: -2.979426004836674E+01 -5.018052460415045E+00 -2.175393728607054E+00
        //std::cout << "Earth @ J2000: " << earthTrajectory->position(0.0).transpose().format(16) << std::endl;
    }

    // Martian satellites
    m_loader->addBuiltinOrbit("Phobos", MarsSatOrbit::Create(MarsSatOrbit::Phobos));
    m_loader->addBuiltinOrbit("Deimos", MarsSatOrbit::Create(MarsSatOrbit::Deimos));

    // Galilean satellites
    m_loader->addBuiltinOrbit("Io", L1Orbit::Create(L1Orbit::Io));
    m_loader->addBuiltinOrbit("Europa", L1Orbit::Create(L1Orbit::Europa));
    m_loader->addBuiltinOrbit("Ganymede", L1Orbit::Create(L1Orbit::Ganymede));
    m_loader->addBuiltinOrbit("Callisto", L1Orbit::Create(L1Orbit::Callisto));

    // Saturnian satellites
    m_loader->addBuiltinOrbit("Mimas",     TASS17Orbit::Create(TASS17Orbit::Mimas));
    m_loader->addBuiltinOrbit("Enceladus", TASS17Orbit::Create(TASS17Orbit::Enceladus));
    m_loader->addBuiltinOrbit("Tethys",    TASS17Orbit::Create(TASS17Orbit::Tethys));
    m_loader->addBuiltinOrbit("Dione",     TASS17Orbit::Create(TASS17Orbit::Dione));
    m_loader->addBuiltinOrbit("Rhea",      TASS17Orbit::Create(TASS17Orbit::Rhea));
    m_loader->addBuiltinOrbit("Titan",     TASS17Orbit::Create(TASS17Orbit::Titan));
    m_loader->addBuiltinOrbit("Hyperion",  TASS17Orbit::Create(TASS17Orbit::Hyperion));
    m_loader->addBuiltinOrbit("Iapetus",   TASS17Orbit::Create(TASS17Orbit::Iapetus));

    // Uranian satellites
    m_loader->addBuiltinOrbit("Miranda",   Gust86Orbit::Create(Gust86Orbit::Miranda));
    m_loader->addBuiltinOrbit("Ariel",     Gust86Orbit::Create(Gust86Orbit::Ariel));
    m_loader->addBuiltinOrbit("Umbriel",   Gust86Orbit::Create(Gust86Orbit::Umbriel));
    m_loader->addBuiltinOrbit("Titania",   Gust86Orbit::Create(Gust86Orbit::Titania));
    m_loader->addBuiltinOrbit("Oberon",    Gust86Orbit::Create(Gust86Orbit::Oberon));

    // Set up builtin rotation models
    m_loader->addBuiltinRotationModel("IAU Moon", new IAULunarRotationModel());

    // Set up the network manager. Eventually, the texture tile loader and resource loader should share
    // the same QNetworkAccessManager. However, there is a noticeable lag when loading a TLE orbit
    // over the network, and it disappears when the cache is disabled. Although reading over the network
    // is asynchronous, loading the cache the directory for the first time blocks for about a second.
    // Using a second network manager with its own cache directory with many fewer entries solves the
    // problem. The lag could return if the resource loader has thousands of files in its cache, but
    // since it currently is used just for TLEs, it's not a problem now.
    m_networkManager = new QNetworkAccessManager();
    QNetworkDiskCache* cache = new QNetworkDiskCache();    
    //cache->setCacheDirectory(QDesktopServices::storageLocation(QDesktopServices::CacheLocation) + "/catalog");
    cache->setCacheDirectory(cacheDirectoryPath("catalog"));
    m_networkManager->setCache(cache);
    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processReceivedResource(QNetworkReply*)));

    // Set up the texture loader
    m_loader->setTextureLoader(dynamic_cast<PathRelativeTextureLoader*>(m_view3d->textureLoader()));

    loadCatalogFile("solarsys.json");
    loadCatalogFile("start-viewpoints.json");

    loadGallery("gallery/gallery.json");

    {
        QNetworkRequest request(QUrl("http://www.cosmographia.info/announcements/current.html"));
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
        request.setOriginatingObject(this);
        m_networkManager->get(request);
    }

    // Clear the list of loaded add-ons so that the basic catalogs can't be unloaded
    foreach (AddOn* addOn, m_loadedAddOns)
    {
        delete addOn;
    }
    m_loadedAddOns.clear();
    updateUnloadAction();

    QStringList viewpointNames = m_catalog->viewpointNames();
    if (!viewpointNames.isEmpty())
    {
        qsrand((uint) QTime::currentTime().msecsTo(QTime(0, 0, 0)));
        QString viewpointName = "Default Start";
        if (!m_catalog->findViewpoint(viewpointName))
        {
            viewpointName = viewpointNames.at(abs(qrand()) % viewpointNames.size());
        }

        m_view3d->setViewpoint(m_catalog->findViewpoint(viewpointName));
    }

    // Parse the command line
    QStringList argList = QCoreApplication::arguments();
    QStringList catalogLoadList;
    QVariantMap argMap;
    if (!argList.isEmpty())
    {
        // Remove the program name
        argList.removeAt(0);

        int argIndex = 0;
        while (argIndex < argList.size())
        {
            QString arg = argList.at(argIndex);

            // Treat all command line arguments starting with a dash as
            // switches with arguments.
            if (arg.at(0) == '-')
            {
                if (argIndex < argList.size() - 1)
                {
                    argMap[arg] = argList[argIndex + 1];
                }
                argIndex += 2;
            }
            else
            {
                // Everything else on the command line is a file that will be loaded
                // at startup time.
                catalogLoadList << arg;
                ++argIndex;
            }
        }
    }

    // Load catalog files
    if (!catalogLoadList.empty())
    {
        QString saveDir = QDir::currentPath();
        QDir::setCurrent(QCoreApplication::applicationDirPath());
        foreach (QString fileName, catalogLoadList)
        {
            QFileInfo fileInfo(fileName);
            loadCatalogFile(fileInfo.absoluteFilePath());
        }
        QDir::setCurrent(saveDir);
    }

    // Handle URLs passed on the command line
    if (argMap.contains("-u"))
    {
        m_view3d->setStateFromUrl(QUrl(argMap.value("-u").toString()));
    }

    // Reapply the earth map month setting, because it requires the Solar System
    // catalog to be loaded
    m_view3d->setEarthMapMonth(m_view3d->earthMapMonth());

    // Required because it must be set after the Solar System catalog is loaded
    m_view3d->setPlanetOrbitsVisibility(m_view3d->planetOrbitsVisibility());
}


// This method is rendered obsolete by the new QML-based user interface
void
Cosmographia::findObject()
{
    QDialog findDialog(this);
    findDialog.setWindowTitle(tr("Find Object"));
    QComboBox* nameEntry = new QComboBox(&findDialog);
    nameEntry->setEditable(true);

    QVBoxLayout* vbox = new QVBoxLayout(&findDialog);
    findDialog.setLayout(vbox);

    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->addWidget(new QLabel("Object name: ", &findDialog));
    hbox->addWidget(nameEntry);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &findDialog);
    vbox->addItem(hbox);
    vbox->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()), &findDialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), &findDialog, SLOT(reject()));

    // TODO: If we need to support extremely large numbers of objects, we should
    // use a abstract item model instead of a completer.
    QCompleter* completer = new QCompleter(m_catalog->names(), nameEntry);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    nameEntry->setCompleter(completer);

    findDialog.move((width() - findDialog.width()) / 2, 0);
    if (findDialog.exec() == QDialog::Accepted)
    {
        QString name = completer->currentCompletion();
        Entity* body = m_catalog->find(name);
        if (body)
        {
            m_view3d->setSelectedBody(body);
        }
    }
}


void
Cosmographia::setTime()
{
    QDialog timeDialog;
    timeDialog.setWindowTitle(tr("Set Time and Date"));
    QDateTimeEdit* timeEdit = new QDateTimeEdit(&timeDialog);
    timeEdit->setDateTimeRange(QDateTime(QDate(1800, 1, 1)), QDateTime(QDate(2100, 1, 1)));
    timeEdit->setDisplayFormat("yyyy-MMM-dd hh:mm:ss");

    QVBoxLayout* vbox = new QVBoxLayout(&timeDialog);
    timeDialog.setLayout(vbox);

    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->addWidget(new QLabel("Enter date: ", &timeDialog));
    hbox->addWidget(timeEdit);

    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &timeDialog);
    vbox->addItem(hbox);
    vbox->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()), &timeDialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), &timeDialog, SLOT(reject()));

    double tsec = m_view3d->simulationTime();
    GregorianDate simDate = GregorianDate::UTCDateFromTDBSec(tsec);
    timeEdit->setDateTime(VestaDateToQtDate(simDate));

    if (timeDialog.exec() == QDialog::Accepted)
    {
        QDateTime newDate = timeEdit->dateTime();
        m_view3d->setSimulationTime(QtDateToVestaDate(newDate).toTDBSec());
    }
}


void
Cosmographia::faster()
{
    double t = m_view3d->timeScale() * 10.0;
    if (t < -1.0e7)
        t = -1.0e7;
    else if (t > 1.0e7)
        t = 1.0e7;
    m_view3d->setTimeScale(t);
}


void
Cosmographia::slower()
{
    double t = m_view3d->timeScale() * 0.1;
    if (t > 0.0)
        t = std::max(1.0e-3, t);
    else if (t < 0.0)
        t = std::min(-1.0e-3, t);
    m_view3d->setTimeScale(t);
}


void
Cosmographia::faster2()
{
    double t = m_view3d->timeScale() * 2.0;
    if (t < -1.0e7)
        t = -1.0e7;
    else if (t > 1.0e7)
        t = 1.0e7;
    m_view3d->setTimeScale(t);
}


void
Cosmographia::slower2()
{
    double t = m_view3d->timeScale() * 0.5;
    if (t > 0.0)
        t = std::max(1.0e-3, t);
    else if (t < 0.0)
        t = std::min(-1.0e-3, t);
    m_view3d->setTimeScale(t);
}


void
Cosmographia::backDay()
{
    m_view3d->setSimulationTime(m_view3d->simulationTime() - daysToSeconds(1.0));
}


void
Cosmographia::forwardDay()
{
    m_view3d->setSimulationTime(m_view3d->simulationTime() + daysToSeconds(1.0));
}


void
Cosmographia::backYear()
{
    vesta::GregorianDate d = vesta::GregorianDate::UTCDateFromTDBSec(m_view3d->simulationTime());
    m_view3d->setSimulationTime(vesta::GregorianDate(d.year() - 1, d.month(), d.day(), d.hour(), d.minute(), d.second()).toTDBSec());
}


void
Cosmographia::forwardYear()
{
    vesta::GregorianDate d = vesta::GregorianDate::UTCDateFromTDBSec(m_view3d->simulationTime());
    m_view3d->setSimulationTime(vesta::GregorianDate(d.year() + 1, d.month(), d.day(), d.hour(), d.minute(), d.second()).toTDBSec());
}


void
Cosmographia::reverseTime()
{
    m_view3d->setTimeScale(-m_view3d->timeScale());
}


void
Cosmographia::plotTrajectory()
{
    Entity* body = m_view3d->selectedBody();
    if (body)
    {
        QString name = QString::fromUtf8(body->name().c_str());
        BodyInfo* info = m_catalog->findInfo(name);

        if (m_view3d->hasTrajectoryPlots(body))
        {
            m_view3d->clearTrajectoryPlots(body);
        }
        else
        {
            m_view3d->plotTrajectory(body, info);
        }
    }
}


void
Cosmographia::plotTrajectoryObserver()
{
    Entity* body = m_view3d->selectedBody();
    if (body)
    {
        QString name = QString::fromUtf8(body->name().c_str());
        BodyInfo* info = m_catalog->findInfo(name);

        m_view3d->plotTrajectoryObserver(info);
    }
}


void
Cosmographia::setFullScreen(bool enabled)
{
    if (enabled)
    {
        showFullScreen();
    }
    else
    {
        showNormal();
    }
}

void
Cosmographia::about()
{
    QMessageBox::about(this,
                       "Cosmographia",
                       "Copyright (C) 2011 by Chris Laurel<br><br>"
                       "Cosmographia includes code from the following libraries: <br>"
                       "VESTA engine for 3D rendering. Copyright (C) Astos Solutions Gmbh<br>"
                       );
}


void
Cosmographia::saveScreenShot()
{
    QImage screenShot = m_view3d->grabFrameBuffer(false);

    //QString defaultFileName = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation) + "/image.png";
    QString defaultFileName = pictureFilePath("image.png");
    QString saveFileName = QFileDialog::getSaveFileName(this, "Save Image As...", defaultFileName, "*.png *.jpg *.webm *.mov *.ogg");
    if (!saveFileName.isEmpty())
    {
        screenShot.save(saveFileName);
    }
}


void
Cosmographia::loadSettings()
{
    QSettings settings;

    double ambientLight = settings.value("ambientLight", 0.15).toDouble();
    m_view3d->setAmbientLight(ambientLight);
    double limitingMagnitude = settings.value("limitingMagnitude", 8.0).toDouble();
    m_view3d->setLimitingMagnitude(limitingMagnitude);
    bool diffractionSpikes = settings.value("diffractionSpikes", false).toBool();
    m_view3d->setDiffractionSpikes(diffractionSpikes);
    int earthMapMonth = settings.value("earthMapMonth", 1).toInt();
    m_view3d->setEarthMapMonth(std::max(0, std::min(11, earthMapMonth)));
    bool planetOrbits = settings.value("planetOrbits", true).toBool();
    m_view3d->setPlanetOrbitsVisibility(planetOrbits);

    m_view3d->setMilkyWayVisible(settings.value("milkyWay", false).toBool());
    m_view3d->setSunGlare(settings.value("sunGlare", true).toBool());
    m_view3d->setShadows(settings.value("generalShadows", false).toBool());
    m_view3d->setCloudsVisible(settings.value("clouds", true).toBool());
    m_view3d->setAtmospheresVisible(settings.value("atmospheres", true).toBool());

    // Guide settings
    m_view3d->setEclipticVisibility(settings.value("ecliptic", false).toBool());
    m_view3d->setEquatorialGridVisibility(settings.value("equatorialGrid", false).toBool());
    m_view3d->setLabelVisibility(settings.value("labels", true).toBool());
    m_view3d->setSurfaceFeatureLabelVisibility(settings.value("surfaceFeatureLabels", false).toBool());
    m_view3d->setConstellationFigureVisibility(settings.value("constellationFigures", false).toBool());
    m_view3d->setConstellationNameVisibility(settings.value("constellationNames", false).toBool());
    m_view3d->setStarNameVisibility(settings.value("starNames", false).toBool());

    m_view3d->setEclipseShadows(true);

    setVideoSize(settings.value("videoSize", "wvga").toString());

    settings.beginGroup("ui");
    setMeasurementSystem(settings.value("measurementSystem", "metric").toString());
    setAutoHideToolBar(settings.value("autoHideToolBar", false).toBool());
    m_fullScreenAction->setChecked(settings.value("fullscreen", true).toBool());
    setFullScreen(m_fullScreenAction->isChecked());
    settings.endGroup();
}


void
Cosmographia::saveSettings()
{
    QSettings settings;

    settings.setValue("ambientLight", m_view3d->ambientLight());
    settings.setValue("limitingMagnitude", m_view3d->limitingMagnitude());
    settings.setValue("diffractionSpikes", m_view3d->diffractionSpikes());
    settings.setValue("earthMapMonth", m_view3d->earthMapMonth());
    settings.setValue("planetOrbits", m_view3d->planetOrbitsVisibility());

    settings.setValue("milkyWay", m_view3d->milkyWayVisible());
    settings.setValue("sunGlare", m_view3d->sunGlare());
    settings.setValue("generalShadows", m_view3d->shadows());
    settings.setValue("clouds", m_view3d->cloudsVisible());
    settings.setValue("atmospheres", m_view3d->atmospheresVisible());

    // Guide settings
    settings.setValue("ecliptic", m_view3d->eclipticVisibility());
    settings.setValue("equatorialGrid", m_view3d->equatorialGridVisibility());
    settings.setValue("labels", m_view3d->labelVisibility());
    settings.setValue("surfaceFeatureLabels", m_view3d->surfaceFeatureLabelVisibility());
    settings.setValue("constellationFigures", m_view3d->constellationFigureVisibility());
    settings.setValue("constellationNames", m_view3d->constellationFigureVisibility());
    settings.setValue("starNames", m_view3d->starNameVisibility());

    settings.setValue("previouslyRun", true);

    settings.setValue("videoSize", videoSize());

    settings.beginGroup("ui");
    settings.setValue("measurementSystem", measurementSystem());
    settings.setValue("autoHideToolBar", m_autoHideToolBar);
    settings.setValue("fullscreen", m_fullScreenAction->isChecked());
    settings.endGroup();
}


// Necessary because QML doesn't currently allow access to QSettings
QVariant
Cosmographia::getSetting(const QString &key)
{
    QSettings settings;
    return settings.value(key);
}


// Useful for converting numbers to human-friendly strings in QML
// (which doesn't seem to have any locale support)
QString
Cosmographia::formatNumber(double value, int precision)
{
    NumberFormat format((unsigned int) std::max(0, precision));
    return format.toString(value);
}


QString
Cosmographia::formatDistance(double value, int precision)
{
    NumberFormat format((unsigned int) std::max(0, precision));
    if (GetDefaultMeasurementSystem() == ImperialUnits)
    {
        return tr("%1 miles").arg(format.toString(ConvertDistance(value, Unit_Kilometer, Unit_Mile)));
    }
    else
    {
        return tr("%1 km").arg(format.toString(value));
    }
}


QString
Cosmographia::formatSpeed(double value, int precision)
{
    NumberFormat format((unsigned int) std::max(0, precision));
    if (GetDefaultMeasurementSystem() == ImperialUnits)
    {
        return tr("%1 mph").arg(format.toString(ConvertDistance(value, Unit_Kilometer, Unit_Mile) * 3600));
    }
    else
    {
        return tr("%1 km/s").arg(format.toString(value));
    }
}


void
Cosmographia::setVideoSize(const QString& videoSize)
{
    if (videoSize != m_videoSize)
    {
        m_videoSize = videoSize;
        emit videoSizeChanged(videoSize);
    }
}


void
Cosmographia::activateCosmoUrl(const QString& url)
{
    if (m_view3d)
    {
        activateWindow();
        raise();
        m_view3d->setStateFromUrl(QUrl(url));
    }
    else
    {
        // Not ready to accept urls. Url requests should be saved and
        // delivered only when the main application window is initialized.
    }
}


void
Cosmographia::recordVideo()
{
#if FFMPEG_SUPPORT || QTKIT_SUPPORT
    if (m_view3d->isRecordingVideo())
    {
        m_view3d->videoEncoder()->close();
        m_view3d->finishVideoRecording();
    }
    else
    {
#ifdef QTKIT_SUPPORT
        QString defaultExtension = "mov";
        QString extensions = "Video (*.mov)";
#else
        QString defaultExtension = "mpeg";
        QString extensions = "Video (*.mkv *.mpeg *.avi)";
#endif
        //QString defaultFileName = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation) + "/cosmo." + defaultExtension;
        QString defaultFileName = pictureFilePath("/cosmo." + defaultExtension);
        QString saveFileName = QFileDialog::getSaveFileName(this, "Save Video As...", defaultFileName, extensions);

        QSize videoDimensions(640, 480);
        if (m_videoSize == "wvga")
        {
            videoDimensions = QSize(854, 480);
        }
        else if (m_videoSize == "vga")
        {
            videoDimensions = QSize(640, 480);
        }
        else if (m_videoSize == "720p")
        {
            videoDimensions = QSize(1280, 720);
        }
        else if (m_videoSize == "1080p")
        {
            videoDimensions = QSize(1920, 1080);
        }

        if (!saveFileName.isEmpty())
        {
            QVideoEncoder* encoder = new QVideoEncoder();
            encoder->createFile(saveFileName, (unsigned int) videoDimensions.width(), (unsigned int) videoDimensions.height(), 5000000, 20);
            m_view3d->startVideoRecording(encoder);
        }
    }
#endif
}


void
Cosmographia::loadCatalog()
{
    QSettings settings;
    //QString defaultFileName = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/cosmo.json";
    QString defaultFileName = documentFilePath("cosmo.json");
    defaultFileName = settings.value("SolarSystemDir", defaultFileName).toString();

    QString solarSystemFileName = QFileDialog::getOpenFileName(this, "Load Catalog", defaultFileName, "Catalog Files (*.json *.ssc)");
    if (!solarSystemFileName.isEmpty())
    {
        loadCatalogFile(solarSystemFileName);
        settings.setValue("SolarSystemDir", solarSystemFileName);
    }
}


void
Cosmographia::unloadLastCatalog()
{
    if (!m_loadedAddOns.empty())
    {
        AddOn* addOn = m_loadedAddOns.last();

        // Remove all objects from the catalog file
        foreach (QString objectName, addOn->objects())
        {
            removeBody(objectName);
        }

        m_loader->unloadSpiceKernels(addOn->spiceKernels());

        // Delete the addOn
        m_loadedAddOns.removeLast();
        delete addOn;
    }

    updateUnloadAction();
}


void
Cosmographia::copyStateUrlToClipboard()
{
    if (m_view3d)
    {
        QByteArray url = m_view3d->getStateUrl().toEncoded();
        QApplication::clipboard()->setText(QString::fromLatin1(url.data()));
        m_view3d->setStatusMessage(tr("Copied viewpoint URL to clipboard"));
    }
}


void
Cosmographia::loadStarNamesFile(const QString &fileName, StarCatalog* starCatalog)
{
    QFile namesFile(fileName);
    if (!namesFile.open(QFile::ReadOnly))
    {
        qDebug() << "Error opening star names file " << fileName;
        return;
    }

    QJson::Parser parser;
    bool parseOk = false;
    QVariant nameListVar = parser.parse(&namesFile, &parseOk);
    if (!parseOk)
    {
        qDebug() << "Error parsing star names list: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
        return;
    }

    if (nameListVar.type() != QVariant::List)
    {
        qDebug() << "Star names file must contain a single JSON list.";
        return;
    }

    SkyLabelLayer* starNamesLayer = new SkyLabelLayer();

    QVariantList nameList = nameListVar.toList();
    foreach (QVariant recordVar, nameList)
    {
        if (recordVar.type() != QVariant::Map)
        {
            qDebug() << "Bad record in star names list.";
        }

        QVariantMap record = recordVar.toMap();
        QVariant tychoIdVar = record.value("tychoId");
        QVariant nameVar = record.value("name");

        if (!tychoIdVar.canConvert(QVariant::Int))
        {
            qDebug() << "Bad or missing Tycho ID in star names list";
        }

        if (nameVar.type() != QVariant::String)
        {
            qDebug() << "Bad or missing name in star names list";
        }

        int tychoId = tychoIdVar.toInt();
        QString name = nameVar.toString();

        const StarCatalog::StarRecord* star = starCatalog->findStarIdentifier(tychoId);
        if (star)
        {
            // Set the minimum FOV so that names of fainter stars pop into view only
            // at higher zoom levels.
            float baseMagnitude = 2.0f;
            float relativeLuminosity = pow(2.512f, baseMagnitude - star->apparentMagnitude);
            float minFov = float(PI) / 2.0f * pow(relativeLuminosity, 1.5f);

            // Add a space to offset the label from the star. It would be better if a pixel offset
            // could be specified for labels.
            QString spaceName = QString(" ") + name;
            starNamesLayer->addLabel(spaceName.toLatin1().data(), star->declination, star->RA, Spectrum(0.5f, 0.5f, 0.7f), minFov);
        }
    }

    starNamesLayer->setFont(m_view3d->font(UniverseView::LabelFont));

    m_universe->setLayer("star names", starNamesLayer);
}


// Remove the body with the specified name. Takes care of removing
// all associated visualizers, but does not remove the object from
// the catalog (since that is indexed by name)
void
Cosmographia::removeBody(vesta::Entity* body)
{
    m_view3d->clearTrajectoryPlots(body);
    m_universe->removeEntity(body);
}


// Remove the body with the specified name. Takes care of removing
// all associated visualizers
void
Cosmographia::removeBody(const QString& name)
{
    Entity* e = m_catalog->find(name);
    m_catalog->removeBody(name);
    if (e)
    {
        removeBody(e);
    }
}


// Adjust the unload last catalog action after loading or unloading a catalog
void
Cosmographia::updateUnloadAction()
{
    // Update the action
    if (m_loadedAddOns.empty())
    {
        m_unloadLastCatalogAction->setDisabled(true);
        m_unloadLastCatalogAction->setText("Unload Last Catalog");
    }
    else
    {
        m_unloadLastCatalogAction->setEnabled(true);
        m_unloadLastCatalogAction->setText(QString("Unload Catalog %1").arg(m_loadedAddOns.last()->title()));
    }
}

void
Cosmographia::showCatalogErrorDialog(const QString& errorMessages)
{
    QDialog errorDialog;
    errorDialog.setMinimumSize(600, 300);
    QVBoxLayout* layout = new QVBoxLayout(&errorDialog);
    layout->addWidget(new QLabel("Error and warning log:", &errorDialog));
    QTextEdit* text = new QTextEdit(&errorDialog);
    layout->addWidget(text);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &errorDialog);
    layout->addWidget(buttonBox);
    errorDialog.setWindowTitle("Error loading catalog file");
    text->setText(errorMessages);
    text->setReadOnly(true);
    connect(buttonBox, SIGNAL(accepted()), &errorDialog, SLOT(accept()));

    errorDialog.exec();
}


void
Cosmographia::loadAddOn(const QString& source)
{
    // If the add-on is already loaded, this method will unload and reload it
    unloadAddOn(source);
    loadCatalogFile(source);
    qDebug() << "Loaded " << source;
}


void
Cosmographia::unloadAddOn(const QString& source)
{
    QFile catalogFile(source);
    QFileInfo info = QFileInfo(catalogFile);
    QString path = info.absoluteFilePath();

    foreach (AddOn* addOn, m_loadedAddOns)
    {
        if (addOn->source() == path)
        {
            // Remove all objects from the catalog file
            foreach (QString objectName, addOn->objects())
            {
                removeBody(objectName);
            }

            // Delete the addOn
            m_loadedAddOns.removeLast();
            delete addOn;
        }
    }
}


void
Cosmographia::loadCatalogFile(const QString& fileName)
{
    if (fileName.isEmpty())
    {
        return;
    }

    QFile catalogFile(fileName);
    QFileInfo info = QFileInfo(catalogFile);
    QString path = info.absolutePath();

    m_loader->clearResourceRequests();

    if (!catalogFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("Solar System File Error"), tr("Could not open file '%1'.").arg(fileName));
        return;
    }

    m_loader->setDataSearchPath(path);
    m_loader->setModelSearchPath(path);

    NetworkTextureLoader* textureLoader = dynamic_cast<NetworkTextureLoader*>(m_loader->textureLoader());
    if (textureLoader)
    {
        textureLoader->setLocalSearchPath(path);
    }

    m_loader->clearMessageLog();
    CatalogContents* contents = m_loader->loadCatalogFile(info.fileName(), m_catalog);
    QStringList bodyNames = contents->bodyNames();
    QString errorMessages = m_loader->messageLog();
    if (!errorMessages.isEmpty())
    {
        showCatalogErrorDialog(errorMessages);
    }
    else
    {
        AddOn* addOn = new AddOn();
        addOn->setSource(info.absoluteFilePath());
        addOn->setTitle(info.fileName());

        addOn->setSpiceKernels(contents->spiceKernels());
        foreach (QString name, bodyNames)
        {
            addOn->addObject(name);
        }

        // If we've previously loaded this add-on, remove it
        for (int i = 0; i < m_loadedAddOns.size(); ++i)
        {
            if (m_loadedAddOns[i]->source() == addOn->source())
            {
                delete m_loadedAddOns[i];
                m_loadedAddOns.removeAt(i);
                break;
            }
        }

        m_loadedAddOns << addOn;
        updateUnloadAction();
    }

    foreach (QString name, bodyNames)
    {
        Entity* e = m_catalog->find(name);
        if (e)
        {
            m_view3d->replaceEntity(e, m_catalog->findInfo(name));
        }
    }

    QSet<QString> resourceRequests = m_loader->resourceRequests();
    if (!resourceRequests.isEmpty())
    {
        qDebug() << "Resource requests:";
        foreach (QString resource, resourceRequests)
        {
            QNetworkRequest request(resource);
            request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
            QNetworkReply* reply = m_networkManager->get(request);

            qDebug() << resource << " -> " << reply->url().toString();
        }
    }

    if (textureLoader)
    {
        textureLoader->setLocalSearchPath(".");
    }
}


void
Cosmographia::loadGallery(const QString& fileName)
{
#if ENABLE_GALLERY
    if (fileName.isEmpty())
    {
        return;
    }

    QFile galleryFile(fileName);
    QFileInfo info = QFileInfo(galleryFile);
    //QString path = info.absolutePath();
    QString path = QDir::current().relativeFilePath(info.absolutePath());

    if (!galleryFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Could not open gallery file " << fileName;
        return;
    }

    QJson::Parser parser;
    bool parseOk = false;
    QVariant nameListVar = parser.parse(&galleryFile, &parseOk);
    if (!parseOk)
    {
        qDebug() << "Error parsing gallery file: " << parser.errorString() << " (line: " << parser.errorLine() << ")";
        return;
    }

    if (nameListVar.type() != QVariant::List)
    {
        qDebug() << "Gallery file must contain a single JSON list.";
        return;
    }

    QVariantList nameList = nameListVar.toList();
    foreach (QVariant recordVar, nameList)
    {
        if (recordVar.type() != QVariant::Map)
        {
            qDebug() << "Bad record in gallery list.";
        }

        QVariantMap record = recordVar.toMap();
        QVariant nameVar = record.value("name");
        QVariant imageVar = record.value("image");

        if (nameVar.type() != QVariant::String)
        {
            qDebug() << "Bad or missing name in gallery file";
        }

        if (imageVar.type() != QVariant::String)
        {
            qDebug() << "Bad or missing name in gallery file";
        }

        QString name = nameVar.toString();
        QString image = imageVar.toString();

        if (!name.isEmpty() && !image.isEmpty())
        {
            QString imageFile = path + "/" + image;
            TextureProperties tileTexProperties(TextureProperties::Clamp);
            m_view3d->gallery()->addTile(m_view3d->textureLoader()->loadTexture(imageFile.toUtf8().data(), tileTexProperties),
                                         name.toUtf8().data());
        }
    }
#endif
}


void
Cosmographia::processReceivedResource(QNetworkReply* reply)
{
    qDebug() << "Resource received: " << reply->url().toString();

    //QVariant fromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute);
    //qDebug() << "Was cached: " << fromCache.toBool();

    if (reply->open(QIODevice::ReadOnly))
    {
        // If the originating object is the main window, it indicates that the
        // requested resource was the announcement.
        if (reply->request().originatingObject() == this && reply->error() == QNetworkReply::NoError)
        {
            QString text = reply->readAll();
            m_helpCatalog->setHelpText("announcement", text);

            QDateTime modifiedTime = reply->header(QNetworkRequest::LastModifiedHeader).toDateTime();
            if (modifiedTime.isValid())
            {
                // Only show the announcement popup on the App Store version (for now). The announcements
                // are still available in the 'News' section of help.
#ifdef MAS_DEPLOY
                showAnnouncement(text, modifiedTime);
#endif
            }
        }
        else
        {
            QTextStream stream(reply);
            m_loader->processTleSet(reply->url().toString(), stream);
            m_loader->processUpdates();
        }
    }
}


void
Cosmographia::showAnnouncement(const QString& text, const QDateTime& modifiedTime)
{
    QSettings settings;

    // If this is the first time that Cosmographia has been run, don't show
    // the news window; the users focus should be on the intro window.
    if (!settings.value("previouslyRun", false).toBool())
    {
        return;
    }

    // Only show the announcement if it has changed
    QDateTime lastAnnouncementTime = settings.value("lastAnnouncementTime", QDateTime(QDate(2000, 1, 1))).toDateTime();
    if (modifiedTime > lastAnnouncementTime)
    {
        settings.setValue("lastAnnouncementTime", modifiedTime);
        emit announcementReceived(text);
    }
}


void
Cosmographia::setStarStyle(QAction *action)
{
    StarsLayer* stars = dynamic_cast<StarsLayer*>(m_view3d->universe()->layer("stars"));
    if (stars)
    {
        int mode = action->data().toInt();

        switch (mode)
        {
        case 0:
            stars->setStyle(StarsLayer::PointStars);
            break;
        case 1:
            stars->setStyle(StarsLayer::GaussianStars);
            stars->setDiffractionSpikeBrightness(0.0f);
            break;
        case 2:
            stars->setStyle(StarsLayer::GaussianStars);
            stars->setDiffractionSpikeBrightness(0.3f);
            break;
        default:
            break;
        }
    }
}


void
Cosmographia::setStereoMode(QAction* action)
{
    m_view3d->setStereoMode(UniverseView::StereoMode(action->data().toInt()));
}


void
Cosmographia::setTimeDisplay(QAction *action)
{
    UniverseView::TimeDisplayMode mode = (UniverseView::TimeDisplayMode) action->data().toInt();
    m_view3d->setTimeDisplay(mode);
}


bool
Cosmographia::event(QEvent* event)
{
    if (event->type() == QEvent::Hide)
    {
        // Reduce CPU usage when the app is minimized or otherwise hidden
        m_view3d->setUpdateInterval(500);
        return QMainWindow::event(event);
    }
    else if (event->type() == QEvent::Show)
    {
        m_view3d->setUpdateInterval(10);
        return QMainWindow::event(event);
    }
    else
    {
        return QMainWindow::event(event);
    }
}


/** Set whether the tool bar automatically disappears/reappears based on
  * the mouse position.
  */
void
Cosmographia::setAutoHideToolBar(bool enabled)
{
    if (enabled != m_autoHideToolBar)
    {
        m_autoHideToolBar = enabled;
        emit autoHideToolBarChanged();
    }
}


void
Cosmographia::minimize()
{
    if (!isFullScreen())
    {
        showMinimized();
    }
}


/** Get the current measurement system used when displaying distances and masses to the user.
  * The return value is either "metric" or "imperial"
  */
QString
Cosmographia::measurementSystem() const
{
    switch (GetDefaultMeasurementSystem())
    {
    case MetricUnits:
        return "metric";
    case ImperialUnits:
        return "imperial";
    default:
        return "";
    }
}


/** Set the measurement system used when displaying distances and masses to the user.
  * The string should be either "metric" or "imperial"
  */
void
Cosmographia::setMeasurementSystem(const QString& ms)
{
    MeasurementSystem m = GetDefaultMeasurementSystem();
    if (ms == "metric")
    {
        m = MetricUnits;
    }
    else if (ms == "imperial")
    {
        m = ImperialUnits;
    }

    if (m != GetDefaultMeasurementSystem())
    {
        SetDefaultMeasurementSystem(m);
        emit measurementSystemChanged(ms);
    }
}
