// This file is part of Cosmographia.
//
// Copyright (C) 2010-2012 Chris Laurel <claurel@gmail.com>
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

#define TEST_SIMPLE_TRAJECTORY 0

#include <cmath>

#include <QGLWidget>

#include <vesta/OGLHeaders.h>
#include "UniverseView.h"
#include "MarkerLayer.h"
#include "GalleryView.h"
#include "UnitConversion.h"

#include "ObserverAction.h"
#include "Viewpoint.h"
#include "InterpolatedStateTrajectory.h"
#include "DateUtility.h"
#include "SkyLabelLayer.h"
#include "ConstellationInfo.h"
#include "TwoVectorFrame.h"
#include "MultiWMSTiledMap.h"
#include "MultiLabelVisualizer.h"
#include "geometry/SimpleTrajectoryGeometry.h"
#include "geometry/FeatureLabelSetGeometry.h"

#include "NumberFormat.h"

#if FFMPEG_SUPPORT
#include "QVideoEncoder.h"
#elif QTKIT_SUPPORT
#include "../video/VideoEncoder.h"
#endif

#include <vesta/Chronology.h>
#include <vesta/Arc.h>
#include <vesta/Body.h>
#include <vesta/Units.h>
#include <vesta/Universe.h>
#include <vesta/UniverseRenderer.h>
#include <vesta/WorldGeometry.h>
#include <vesta/TextureMapLoader.h>
#include <vesta/InertialFrame.h>
#include <vesta/BodyFixedFrame.h>
#include <vesta/TwoBodyRotatingFrame.h>
#include <vesta/CelestialCoordinateGrid.h>
#include <vesta/StarsLayer.h>
#include <vesta/SkyImageLayer.h>
#include <vesta/ConstellationsLayer.h>
#include <vesta/PlaneVisualizer.h>
#include <vesta/AxesVisualizer.h>
#include <vesta/VelocityVisualizer.h>
#include <vesta/NadirVisualizer.h>
#include <vesta/BodyDirectionVisualizer.h>
#include <vesta/LabelVisualizer.h>
#include <vesta/TrajectoryGeometry.h>
#include <vesta/TextureFont.h>
#include <vesta/DataChunk.h>
#include <vesta/SingleTextureTiledMap.h>
#include <vesta/HierarchicalTiledMap.h>
#include <vesta/PlanetGridLayer.h>
#include <vesta/GlareOverlay.h>
#include <vesta/GregorianDate.h>
#include <vesta/Intersect.h>

#include <vesta/interaction/ObserverController.h>

#include <vesta/CubeMapFramebuffer.h>

#include <vesta/ParticleSystemGeometry.h>
#include <vesta/particlesys/ParticleEmitter.h>
#include <vesta/particlesys/PointGenerator.h>
#include <vesta/particlesys/DiscGenerator.h>

#include <Eigen/LU>

#include <QSettings>
#include <QAction>
#include <QMenu>
#include <QClipboard>
#include <QApplication>
#include <QGraphicsItem>
#include <QFile>
#include <QDataStream>

#include <QDebug>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkDiskCache>
#include <QDesktopServices>
#include <QLocale>
#include <QPinchGesture>

#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>

using namespace vesta;
using namespace Eigen;
using namespace std;


// No need to enable this now; it is only useful once there are point
// light sources in the scene.
#define OMNI_SHADOW_MAPS 0

static const int MaxAntialiasingSampleCount = 8;

static const double KeyboardRotationAcceleration = 3.5;

static const unsigned int ShadowMapSize = 2048;
static const unsigned int ReflectionMapSize = 512;

static double StartOfTime = GregorianDate(1800, 1, 1, 13, 0, 0, 0, TimeScale_TDB).toTDBSec();
static double EndOfTime   = GregorianDate(2100, 1, 1, 0, 0, 0, 0, TimeScale_TDB).toTDBSec();

static const double MinimumFOV = toRadians(0.1);
static const double MaximumFOV = toRadians(90.0);
static const double ReticleFOVThreshold = toRadians(20.0);
static const double MaximumDistanceFromSun = 5.0e11;

static const double StatusMessageDuration = 2.5;

static const float CenterMarkerSize = 10.0f;

static const bool ShowTimeInVideos = true;

#ifdef LEO3D_SUPPORT
#include <LeoAPI.h>

class Leo3DState
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Leo3DState() :
        context(0),
        m_lastBirdOrientation(Quaterniond::Identity()),
        m_lastBirdPosition(Vector3d::Zero()),
        m_birdOrientationChange(Quaterniond::Identity()),
        m_birdPositionChange(Vector3d::Zero())
    {}

    LGLContext context;

    void updateInputDeviceState()
    {
        bool birdVisibleNow = leoIsBirdVisible();
        bool birdSmallButtonPressed = leoGetSmallButtonState();

        if (!birdVisibleNow)
        {
            m_birdOrientationChange = Quaterniond::Identity();
            m_birdPositionChange = Vector3d::Zero();
        }
        else
        {
            double x = 0.0;
            double y = 0.0;
            double z = 0.0;
            double xr = 0.0;
            double yr = 0.0;
            double zr = 0.0;

            leoGetBirdPosition(&x, &y, &z, &xr, &yr, &zr);

            Quaterniond q(AngleAxisd(zr, Vector3d::UnitZ()) *
                          AngleAxisd(yr, Vector3d::UnitY()) *
                          AngleAxisd(xr, Vector3d::UnitX()));
            Vector3d r(x, y, z);

            if (birdSmallButtonPressed)
            {
                if (!m_birdSmallButtonWasPressed)
                {
                    m_birdOrientationChange = Quaterniond::Identity();
                    m_birdPositionChange = Vector3d::Zero();
                }
                else
                {
                    m_birdPositionChange = r - m_lastBirdPosition;
                    m_birdOrientationChange = q * m_lastBirdOrientation.conjugate();
                }

                m_lastBirdOrientation = q;
                m_lastBirdPosition = r;
            }
            else
            {
                m_birdOrientationChange = Quaterniond::Identity();
                m_birdPositionChange = Vector3d::Zero();
            }
        }

        m_birdWasVisible = birdVisibleNow;
        m_birdSmallButtonWasPressed = birdSmallButtonPressed;
    }

    Quaterniond birdOrientationChange() const
    {
        return m_birdOrientationChange;
    }

    Vector3d birdPositionChange() const
    {
        return m_birdPositionChange;
    }

    bool birdBigButton() const
    {
        return false;
    }

    bool birdSmallButton() const
    {
        return m_birdSmallButtonWasPressed;
    }

    Quaterniond birdOrientation() const
    {
        return m_lastBirdOrientation;
    }

private:
    Quaterniond m_lastBirdOrientation;
    Vector3d m_lastBirdPosition;

    bool m_birdWasVisible;
    bool m_birdSmallButtonWasPressed;

    Quaterniond m_birdOrientationChange;
    Vector3d m_birdPositionChange;
};

#endif

static TextureProperties SkyLayerTextureProperties()
{
    TextureProperties props;
    props.addressS = TextureProperties::Wrap;
    props.addressT = TextureProperties::Clamp;
    return props;
}



static string TrajectoryVisualizerName(Entity* entity)
{
    return string("traj - ") + entity->name();
}


// Planetographic coordinate with hemisphere information
struct PlanetographicCoordHemi
{
    double latitude;
    double longitude;
    char latitudeHemiId;
    char longitudeHemiId;
};


// Convert cartesian position in body-fixed coordinate to planetographic
// coordinates using the conventions appropriate to the specified body. At the
// moment, we just use the name of the body to determine which coordinate convention
// to use. A more robust solution would be to specify the coordinate convention
// in the catalog files.
//
// This function uses rotational north convention rather than ecliptic north
//
static PlanetographicCoordHemi getPlanetographicCoordinate(const Vector3d& position, const Entity* body)
{
    PlanetographicCoordHemi coord;

    if (body->geometry() && body->geometry()->isEllipsoidal())
    {
        PlanetographicCoord3 pc = body->geometry()->ellipsoid().rectangularToPlanetographic(position);
        coord.latitude = toDegrees(pc.latitude());
        coord.longitude = toDegrees(pc.longitude());
    }
    else
    {
        Vector3d n = position.normalized();
        coord.latitude = toDegrees(asin(n.z()));
        coord.longitude = toDegrees(atan2(n.y(), n.x()));
    }
    coord.longitudeHemiId = 'E';
    coord.latitudeHemiId = 'N';

    if (body->name() == "Earth" || body->name() == "Moon")
    {
        if (coord.longitude < 0.0)
        {
            coord.longitudeHemiId = 'W';
            coord.longitude = -coord.longitude;
        }
    }
    else if (body->name() == "Mars")
    {
        if (coord.longitude < 0.0)
        {
            coord.longitude = 360.0 + coord.longitude;
        }
    }
    else
    {
        if (coord.longitude < 0.0)
        {
            coord.longitude = -coord.longitude;
        }
        else
        {
            coord.longitude = 360.0 - coord.longitude;
        }
        coord.longitudeHemiId = 'W';
    }

    if (coord.latitude < 0.0)
    {
        coord.latitudeHemiId = 'S';
        coord.latitude = -coord.latitude;
    }

    return coord;
}


class UniverseGLWidget : public QGLWidget
{
    //Q_OBJECT

public:
    UniverseGLWidget(QWidget* parent, UniverseRenderer* renderer, const QGLFormat& format) :
        QGLWidget(format, parent),
        m_renderer(renderer)
    {
        setAttribute(Qt::WA_PaintOnScreen);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_OpaquePaintEvent);
        setBackgroundRole(QPalette::Window);
    }


    virtual ~UniverseGLWidget()
    {
    }


    virtual void initializeGL()
    {
        QGLWidget::initializeGL();

        // Assume that the parent is a UniverseView and call it to
        // initialize resources.
        if (dynamic_cast<UniverseView*>(parent()))
        {
            dynamic_cast<UniverseView*>(parent())->initializeGL();
        }
    }

private:
    UniverseRenderer* m_renderer;
};


UniverseView::UniverseView(QWidget *parent, Universe* universe, UniverseCatalog* catalog) :
    QDeclarativeView(parent),
    m_mouseMovement(0),
    m_lastDoubleClickTime(0.0),
    m_mouseClickEventProcessed(false),
    m_mouseMoveEventProcessed(false),
    m_sceneHadFocus(false),
    m_glInitialized(false),
    m_catalog(catalog),
    m_controller(new ObserverController()),
    m_renderer(NULL),
    m_observerFrame(Frame_Inertial),
    m_fovY(toRadians(50.0)),
    m_rollLeft(false),
    m_rollRight(false),
    m_pitchDown(false),
    m_pitchUp(false),
    m_timer(NULL),
    m_realTime(0.0),
    m_simulationTime(0.0),
    m_firstTick(true),
    m_lastTickTime(0.0),
    m_timeScale(1.0),
    m_paused(false),
    m_titleFont(NULL),
    m_labelFont(NULL),
    m_frameCount(0),
    m_frameCountStartTime(0.0),
    m_framesPerSecond(0.0),
    m_reflectionsEnabled(false),
    m_stereoMode(Mono),
    m_antialiasingSamples(1),
    m_sunGlareEnabled(true),
    m_planetOrbitsVisible(false),
    m_infoTextVisible(true),
    m_labelsVisible(true),
    m_surfaceFeatureLabelsVisible(false),
    m_centerIndicatorVisible(true),
    m_gotoObjectTime(6.0),
    m_videoEncoder(NULL),
    m_videoRecordingStartTime(0.0),
    m_timeDisplay(TimeDisplay_UTC),
    m_wireframe(false),
    m_captureNextImage(false),
    m_reticleUpdateTime(-1.0e10),
    m_statusUpdateTime(0.0),
    m_markers(NULL),
    m_galleryView(NULL),
    m_earthMapMonth(1),
    m_leoState(NULL)
{
    setAutoFillBackground(false);
    setMouseTracking(true);

    m_universe = universe;
    m_textureLoader = new NetworkTextureLoader(this);
    m_renderer = new UniverseRenderer();
    m_renderer->setDefaultSunEnabled(false);

    m_labelFont = new TextureFont();
    m_textFont = new TextureFont();
    m_titleFont = new TextureFont();
    m_spacecraftIcon = m_textureLoader->loadTexture(":/icons/disk.png", TextureProperties(TextureProperties::Clamp));

    m_markers = new MarkerLayer();

    m_galleryView = new GalleryView();
    m_galleryView->setGridSize(10, 4);
    m_galleryView->setScale(1.0f);
    m_galleryView->setFont(m_textFont.ptr());

    // Enable multisample antialiasing if its enabled in the settings
    {
        QSettings settings;
        m_antialiasingSamples = std::max(1, std::min(MaxAntialiasingSampleCount, settings.value("AntialiasingSamples", 1).toInt()));
    }

    QGLFormat format = QGLFormat::defaultFormat();
    if (m_antialiasingSamples > 1)
    {
        format.setSampleBuffers(true);
        format.setSamples(m_antialiasingSamples);
    }
    format.setSwapInterval(1); // sync to vertical retrace

    UniverseGLWidget* glWidget = new UniverseGLWidget(this, m_renderer, format);
    glWidget->updateGL();
    setViewport(glWidget);

    initializeObserver();
    initializeSkyLayers();

    setBackgroundBrush(Qt::transparent);
    setResizeMode(SizeRootObjectToView);

    scene()->setBackgroundBrush(Qt::NoBrush);

    // Initialize the base time that will be used as a reference for calculating
    // the elapsed time.
    m_baseTime = QDateTime::currentDateTime();
    m_baseTime.setTime(QTime(0, 0, 0, 0));

    // Set the simulation time to the current time
    QDateTime now = QDateTime::currentDateTime().toUTC();
    GregorianDate startDate(now.date().year(), now.date().month(), now.date().day(),
                            now.time().hour(), now.time().minute(), now.time().second());
    m_simulationTime = startDate.toTDBSec();

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(tick()));
    m_timer->setInterval(0);
    m_timer->start();

    setFocusPolicy(Qt::StrongFocus);

    initNetwork();

    grabGesture(Qt::PinchGesture);
#if 0
    // Initialize settings
    setLimitingMagnitude(8.0);
    emit limitingMagnitudeChanged(limitingMagnitude()); // Force a notification
#endif
}


UniverseView::~UniverseView()
{
    //makeCurrent();
    delete m_galleryView;
    delete m_renderer;
}


TextureFont*
UniverseView::font(FontRole role) const
{
    switch (role)
    {
    case LabelFont:
        return m_labelFont.ptr();
    case TitleFont:
        return m_titleFont.ptr();
    case TextFont:
    default:
        return m_textFont.ptr();
    }
}


void
UniverseView::initializeDeclarativeUi(const QString& qmlFileName)
{
    setSource(QUrl::fromLocalFile(qmlFileName));
    connect(engine(), SIGNAL(quit()), QCoreApplication::instance(), SLOT(quit()));
}


QSize UniverseView::minimumSizeHint() const
{
    return QSize(50, 50);
}


QSize UniverseView::sizeHint() const
{
    return QSize(800, 600);
}


// Set up the labels to fade when the labeled object is very close or very distant
static void
setLabelFadeRange(LabelGeometry* label, Entity* body, const BodyInfo* info, vesta::Arc* arc, double fadeSize)
{
    float geometrySize = 1.0f;
    if (body->geometry())
    {
        geometrySize = body->geometry()->boundingSphereRadius();
    }

    float orbitSize = arc->trajectory()->boundingSphereRadius();
    if (fadeSize == 0.0)
    {
        fadeSize = orbitSize;
        if (info && info->classification == BodyInfo::Asteroid)
        {
            // There are a lot of asteroids; prevent them from crowding out everything
            // else in the Solar System.
            fadeSize /= 3;
        }
    }

    float minPixels = 40.0f;
    float maxPixels = 20.0f * fadeSize / geometrySize;

    if (body->name() != "Sun")
    {
        label->setFadeSize(fadeSize);
        label->setFadeRange(new FadeRange(minPixels, maxPixels, minPixels, maxPixels));
    }
}


static Visualizer*
labelBody(Entity* planet, const BodyInfo* info, const QString& labelText, TextureFont* font, TextureMap* icon, const Spectrum& color, double fadeSize, bool visible)
{
    if (!planet || !planet->isVisible())
    {
        return NULL;
    }

    // By convention, objects with names beginning with an underscore are not visible
    // to the user.
    if (!planet->name().empty() && planet->name()[0] == '_')
    {
        return NULL;
    }

    // Use a more complex labeling scheme when an object has a trajectory defined with
    // respect to multiple center objects. This is helpful for spacecraft that orbit planets
    // other than Earth; without special handling, their labels end up overlapping those of the
    // planets.
    bool multiLabelRequired = false;
    for (unsigned int i = 1; i < planet->chronology()->arcCount(); ++i)
    {
        if (planet->chronology()->arc(i)->center() != planet->chronology()->firstArc()->center())
        {
            multiLabelRequired = true;
            break;
        }
    }

    Visualizer* vis = NULL;
    if (multiLabelRequired)
    {
        MultiLabelVisualizer* multiLabel = new MultiLabelVisualizer();
        double startTime = planet->chronology()->beginning();
        for (unsigned int i = 0; i < planet->chronology()->arcCount(); ++i)
        {
            vesta::Arc* arc = planet->chronology()->arc(i);

            LabelVisualizer* label = new LabelVisualizer(labelText.toUtf8().data(), font, color, 6.0f);
            label->label()->setIcon(icon);
            label->label()->setIconColor(color);
            setLabelFadeRange(label->label(), planet, info, arc, fadeSize);
            multiLabel->addLabel(startTime, label);
            startTime += arc->duration();
        }

        vis = multiLabel;
    }
    else
    {
        LabelVisualizer* labelVis = new LabelVisualizer(labelText.toUtf8().data(), font, color, 6.0f);
        labelVis->label()->setIcon(icon);
        labelVis->label()->setIconColor(color);
        setLabelFadeRange(labelVis->label(), planet, info, planet->chronology()->firstArc(), fadeSize);
        vis = labelVis;
    }

    planet->setVisualizer("label", vis);
    vis->setVisibility(visible);
    vis->setDepthAdjustment(Visualizer::AdjustToFront);

    return vis;
}

static bool didInit = false;

void UniverseView::initializeGL()
{
    // Initialize the renderer. This must be done *after* an OpenGL context
    // has been created, otherwise information about OpenGL capabilities is
    // not available.
    if (!m_renderer->initializeGraphics())
    {
        qCritical("Creating renderer failed because OpenGL couldn't be initialized.");
    }

#ifdef LEO3D_SUPPORT
    bool leoOk = leoInitialize();
    if (leoOk)
    {
        qDebug() << "Leonar3do initialized!";

        if (leoIsConnected())
        {
            lglInitialize();
            qDebug() << "Leonar3do connected.";
            m_leoState = new Leo3DState();
            qDebug() << "Creating Leonar3do GL Context";
            m_leoState->context = lglCreateContext(NULL, NULL);

            // DEBUGGING ONLY--REMOVE THIS LINE
            //m_leoState = NULL;
        }
        else
        {
            qDebug() << "Leonar3do not connected";
        }
    }
    else
    {
        qDebug() << "Failed to initialize Leonar3do system.";
    }
#endif

    m_spacecraftIcon->makeResident();

    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

    QFile fontFile("csans-28.txf");
    if (fontFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = fontFile.readAll();
        DataChunk chunk(data.data(), data.size());
        m_titleFont->loadTxf(&chunk);
    }

    QFile textFontFile("csans-16.txf");
    if (textFontFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = textFontFile.readAll();
        DataChunk chunk(data.data(), data.size());
        m_textFont->loadTxf(&chunk);
    }

    QFile labelFontFile("csans-14.txf");
    if (labelFontFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = labelFontFile.readAll();
        DataChunk chunk(data.data(), data.size());
        m_labelFont->loadTxf(&chunk);
        m_renderer->setDefaultFont(m_labelFont.ptr());
    }

    if (m_renderer->shadowsSupported())
    {
        m_renderer->initializeShadowMaps(ShadowMapSize, 1);
    }

#if OMNI_SHADOW_MAPS
    if (m_renderer->omniShadowsSupported())
    {
        m_renderer->initializeOmniShadowMaps(1024, 1);
    }
#endif

    setAmbientLight(false);

    if (CubeMapFramebuffer::supported())
    {
        m_reflectionMap = CubeMapFramebuffer::CreateCubicReflectionMap(ReflectionMapSize, TextureMap::R8G8B8A8);
    }

    m_glareOverlay = m_renderer->createGlareOverlay();

    m_galleryView->initializeGL();
}


void
UniverseView::initNetwork()
{
    WMSRequester* wms = m_textureLoader->wmsHandler();
    wms->addSurfaceDefinition("bmng-jan-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Jan_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("bmng-feb-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Feb_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("bmng-mar-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Mar_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("bmng-apr-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Apr_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("bmng-may-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=May_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("bmng-jun-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Jun_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("bmng-jul-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Jul_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("bmng-aug-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Aug_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("bmng-sep-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Sep_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("bmng-oct-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Oct_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("bmng-nov-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Nov_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("bmng-dec-nb",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=BMNG&srs=EPSG:4326&format=image/jpeg&styles=Dec_nb",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              480, 480);
    wms->addSurfaceDefinition("mars-viking",
                              "http://www.mapaplanet.org/explorer-bin/imageMaker.cgi?map=Mars&VERSION=1.1.1&REQUEST=GetMap&SRS=IAU2000:49911&LAYERS=mars_viking_color&FORMAT=image/jpeg",
                              WMSRequester::LatLongBoundingBox(-180.0, -90.0, 0.0, 90.0),
                              512, 512);
    wms->addSurfaceDefinition("earth-global-mosaic",
                              "http://wms.jpl.nasa.gov/wms.cgi?request=GetMap&layers=global_mosaic&srs=EPSG:4326&format=image/jpeg&styles=visual",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              512, 512);
    wms->addSurfaceDefinition("moon-lo",
                              "http://onmoon.jpl.nasa.gov/wms.cgi?version=1.1.1&service=wms&request=GetMap&styles=&srs=IAU2000:30100&layers=LO&width=512&height=512&format=image/jpeg",
                              WMSRequester::LatLongBoundingBox(-180.0, -198.0, 108.0, 90.0),
                              512, 512);
    wms->addSurfaceDefinition("moon-clementine",
                              "http://onmoon.jpl.nasa.gov/wms.cgi?version=1.1.1&service=wms&request=GetMap&styles=&srs=IAU2000:30100&layers=Clementine&width=512&height=512&format=image/jpeg",
                              WMSRequester::LatLongBoundingBox(-180.0, -150.0, 60.0, 90.0),
                              512, 512);
    wms->addSurfaceDefinition("mars-mdim",
                              "http://onmars.jpl.nasa.gov/wms.cgi?request=GetMap&layers=mars&srs=IAU2000:49900&format=image/jpeg&styles=",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              512, 512);
    wms->addSurfaceDefinition("mars-mdim-moc_na",
                              "http://onmars.jpl.nasa.gov/wms.cgi?request=GetMap&layers=mars,moc_na&srs=IAU2000:49900&format=image/jpeg&styles=",
                              WMSRequester::LatLongBoundingBox(-180.0, -166.0, 76.0, 90.0),
                              512, 512);
}


/** Set the month to use for the map of Earth.
  *
  * TODO: This definitely needs to be replaced with a more general mechanism for
  * changing the texture maps on planets.
  */
void
UniverseView::setEarthMapMonth(int month)
{
    m_earthMapMonth = month;

    static const char* bmngLayerNames[] =
    {
        "bmng-jan-nb", "bmng-feb-nb", "bmng-mar-nb",
        "bmng-apr-nb", "bmng-may-nb", "bmng-jun-nb",
        "bmng-jul-nb", "bmng-aug-nb", "bmng-sep-nb",
        "bmng-oct-nb", "bmng-nov-nb", "bmng-dec-nb"
    };

    month = min(11, max(0, month));

    Entity* earth = m_catalog->find("Earth");
    if (!earth)
    {
        return;
    }

    WorldGeometry* world = dynamic_cast<WorldGeometry*>(earth->geometry());
    if (!world)
    {
        return;
    }

    MultiWMSTiledMap* map = dynamic_cast<MultiWMSTiledMap*>(world->tiledMap());
    if (!map)
    {
        return;
    }

    QString topLevelPattern = QString("textures/earth/%1").arg(bmngLayerNames[month]) + ("_%1_%2_%3.jpg");
    MultiWMSTiledMap* newMap = new MultiWMSTiledMap(m_textureLoader.ptr(), topLevelPattern, bmngLayerNames[month], 7, "earth-global-mosaic", 13, 480);
    world->setBaseMap(newMap);
}


void UniverseView::paintEvent(QPaintEvent* /* event */)
{
    // Make the viewport's GL context current
    dynamic_cast<QGLWidget*>(viewport())->makeCurrent();

    // Make sure that we've initialized our GL resources
    if (!m_glInitialized)
    {
        m_glInitialized = true;
        dynamic_cast<QGLWidget*>(viewport())->updateGL();
    }

    QPainter painter(viewport());

    // Save the state of the painter
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Render the universe using VESTA
    // glEnable(GL_LINE_SMOOTH);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_CULL_FACE);
    paintGL();

    // Restore the state of the painter
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopClientAttrib();
    glPopAttrib();

    //if (GLEW_VERSION_1_5)
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    //if (GLEW_VERSION_2_0)
    {
        glUseProgram(0);
    }

    glShadeModel(GL_FLAT);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // Draw the user interface
    QRectF viewRect(0.0f, 0.0f, width(), height());
    scene()->render(&painter, viewRect, viewRect);

    // The painter automatically calls swapBuffers
    painter.end();
}


static
QString formatDate(const GregorianDate& date)
{
    const char* timeSystem = "";
    switch (date.timeScale())
    {
    case TimeScale_TDB:
        timeSystem = "TDB";
        break;
    case TimeScale_TT:
        timeSystem = "TT";
        break;
    case TimeScale_TAI:
        timeSystem = "TAI";
        break;
    case TimeScale_UTC:
        timeSystem = "UTC";
        break;
    }

    return QString("%1-%2-%3 %4:%5:%6 %7")
            .arg(date.year())
            .arg(QLocale::system().monthName(date.month(), QLocale::ShortFormat))
            .arg(date.day(), 2, 10, QChar('0'))
            .arg(date.hour(), 2, 10, QChar('0'))
            .arg(date.minute(), 2, 10, QChar('0'))
            .arg(date.second(), 2, 10, QChar('0'))
            .arg(timeSystem);
}


static void drawQuad(float x, float y, float width, float height)
{
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x, y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x + width, y);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x + width, y + height);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x, y + height);
    glEnd();
}


QString
UniverseView::currentTimeString() const
{
    GregorianDate date = GregorianDate::UTCDateFromTDBSec(m_simulationTime);

    if (m_timeDisplay == TimeDisplay_UTC)
    {
        return formatDate(date);
        //m_textFont->render(formatDate(date).toUtf8().data(), Vector2f(dateX, dateY));
    }
    else if (m_timeDisplay == TimeDisplay_Local)
    {
        QDateTime localTime = VestaDateToQtDate(date).toLocalTime();
        return localTime.toString("yyyy-MMM-dd hh:mm:ss 'Local'");
        //m_textFont->render(localTime.toString("yyyy-MMM-dd hh:mm:ss 'Local'").toLatin1().constData(), Vector2f(dateX, dateY));
    }
    else if (m_timeDisplay == TimeDisplay_Multiple)
    {
        //m_textFont->render(formatDate(date).toUtf8().data(), Vector2f(dateX, dateY));
        GregorianDate ttDate = GregorianDate::TDBDateFromTDBSec(m_simulationTime);
        ttDate.setTimeScale(TimeScale_TT);
        //m_textFont->render(formatDate(ttDate).toUtf8().data(), Vector2f(dateX, dateY));
        //m_textFont->render(QString("JD %1 TT").arg(ttDate.toTTJD(), 0, 'f', 6).toLatin1().constData(), Vector2f(dateX, dateY));
        return formatDate(date) + "\n" + formatDate(ttDate) + "\n" + QString("JD %1 TT").arg(ttDate.toTTJD());
    }
    else
    {
        return "";
    }
}


// Called by QML to report when a mouse release event is missed and should instead
// be processed by the C++ event handler.
void
UniverseView::setMouseClickEventProcessed(bool accepted)
{
    m_mouseClickEventProcessed = accepted;
}


// Called by QML to report when a mouse move event is missed and should instead
// be processed by the C++ event handler.
void
UniverseView::setMouseMoveEventProcessed(bool accepted)
{
    m_mouseMoveEventProcessed = accepted;
}


// Get the name of a body as a QString
QString
UniverseView::bodyName(const Entity* body) const
{
    if (!body)
    {
        return QString();
    }
    else
    {
        return QString::fromUtf8(body->name().c_str(), body->name().size());
    }
}


static void drawArc(float fromAngle, float toAngle, float radius, const Vector2f& center, unsigned int segmentCount)
{
    float phi = toAngle - fromAngle;

    glBegin(GL_LINE_STRIP);
    for (unsigned int i = 0; i <= segmentCount; ++i)
    {
        float theta = fromAngle + phi * float(i) / float(segmentCount);
        Vector2f v = center + Vector2f(cos(theta), sin(theta)) * radius;
        glVertex2fv(v.data());
    }
    glEnd();
}


// Set up for 2D drawing
void
UniverseView::begin2DDrawing()
{
    int viewportWidth = size().width() * window()->devicePixelRatio();
    int viewportHeight = size().height() * window()->devicePixelRatio();
    glViewport(0, 0, viewportWidth, viewportHeight);

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set up matrices for text rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, viewportWidth, 0, viewportHeight, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.125f, 0.125f, 0);
}


void
UniverseView::end2DDrawing()
{
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


// Draw a dark frame around a rectangular area. This is used to darken the
// regions that will be cropped during video recording.
void
UniverseView::drawFrame(float width, float height)
{
    int viewportWidth = size().width();
    int viewportHeight = size().height();

    begin2DDrawing();
    glDisable(GL_TEXTURE_2D);

    float sideWidth = (viewportWidth - width) / 2.0f;
    float topHeight = (viewportHeight - height) / 2.0f;

    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);

    drawQuad(0.0f, 0.0f, viewportWidth, topHeight); // bottom
    drawQuad(0.0f, height + topHeight, viewportWidth, topHeight); // top
    drawQuad(0.0f, topHeight, sideWidth, height);
    drawQuad(sideWidth + width, topHeight, sideWidth, height);

    end2DDrawing();
}


static QString
readableDistance(double km, unsigned int precision)
{
    if (GetDefaultMeasurementSystem() == ImperialUnits)
    {
        double miles = ConvertDistance(km, Unit_Kilometer, Unit_Mile);
        if (miles < 0.5)
        {
            double feet = ConvertDistance(km, Unit_Kilometer, Unit_Foot);
            return QString(QObject::tr("%1 feet").arg(NumberFormat(precision).toString(feet)));
        }
        else
        {
            return QString(QObject::tr("%1 miles").arg(NumberFormat(precision).toString(miles)));
        }
    }
    else
    {
        if (km < 0.5)
        {
            double meters = ConvertDistance(km, Unit_Kilometer, Unit_Meter);
            return QString(QObject::tr("%1 m").arg(NumberFormat(precision).toString(meters)));
        }
        else
        {
            return QString(QObject::tr("%1 km").arg(NumberFormat(precision).toString(km)));
        }
    }
}


// Draw informational text over the 3D view
void
UniverseView::drawInfoOverlay()
{
    int viewportWidth = size().width() * window()->devicePixelRatio();
    int viewportHeight = size().height() * window()->devicePixelRatio();
    glViewport(0, 0, viewportWidth, viewportHeight);

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    float reticleBrightness = float(1.0 - (m_realTime - m_reticleUpdateTime));
    if (reticleBrightness > 0)
    {
        drawFovReticle(reticleBrightness * 0.5f);
    }

    const double textFadeDuration = 1.0;
    float alpha = max(0.0f, min(1.0f, float((m_realTime - 3.0) / textFadeDuration)));
    Vector4f textColor(0.3f, 0.5f, 1.0f, alpha);
    Vector4f titleColor(0.45f, 0.75f, 1.0f, alpha);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    QLocale locale = QLocale::system();

    const float textLeftMargin = 32.0f;

    if (m_galleryView)
    {
        Viewport viewport(size().width(), size().height());
        m_galleryView->render(viewport);
    }

    begin2DDrawing();

    glColor4fv(textColor.data());
    glEnable(GL_TEXTURE_2D);

    if (m_infoTextVisible)
    {
        if (m_textFont.isValid())
        {
            const int titleFontHeight = 30;
            const int textFontHeight = 20;

            // Show the current simulation time
            GregorianDate date = GregorianDate::UTCDateFromTDBSec(m_simulationTime);

            m_textFont->bind();

            float dateY = float(viewportHeight - titleFontHeight);
            float dateX = viewportWidth - 350.0f;

            if (m_timeDisplay == TimeDisplay_UTC)
            {
                m_textFont->renderUtf8(formatDate(date).toUtf8().data(), Vector2f(dateX, dateY));
                dateY -= textFontHeight;
            }
            else if (m_timeDisplay == TimeDisplay_Local)
            {
                QDateTime localTime = VestaDateToQtDate(date).toLocalTime();
                m_textFont->render(localTime.toString("yyyy-MMM-dd hh:mm:ss 'Local'").toLatin1().constData(), Vector2f(dateX, dateY));
                dateY -= textFontHeight;
            }
            else if (m_timeDisplay == TimeDisplay_Multiple)
            {
                m_textFont->render(formatDate(date).toUtf8().data(), Vector2f(dateX, dateY));
                dateY -= textFontHeight;
                GregorianDate ttDate = GregorianDate::TDBDateFromTDBSec(m_simulationTime);
                ttDate.setTimeScale(TimeScale_TT);
                m_textFont->render(formatDate(ttDate).toUtf8().data(), Vector2f(dateX, dateY));
                dateY -= textFontHeight;
                m_textFont->render(QString("JD %1 TT").arg(ttDate.toTTJD(), 0, 'f', 6).toLatin1().constData(), Vector2f(dateX, dateY));
                dateY -= textFontHeight;
            }

            // Show the time rate below the time/date
            {
                // Some complexity to show the time nicely formatted...
                QString timeScaleFormatted;
                if (abs(m_timeScale) > 100)
                {
                    int sigDigits = 1;
                    sigDigits = int(abs(floor(log10(abs(m_timeScale))))) + 1;
                    timeScaleFormatted = NumberFormat((unsigned int) sigDigits).toString(m_timeScale);
                }
                else
                {
                    timeScaleFormatted = QLocale::system().toString(m_timeScale, 'g');
                }

                if (m_paused)
                {
                    QString timeScaleString = QString("%1x time (paused)").arg(timeScaleFormatted);
                    m_textFont->render(timeScaleString.toLatin1().data(), Vector2f(dateX, dateY));
                }
                else
                {
                    QString timeScaleString = QString("%1x time").arg(timeScaleFormatted);
                    m_textFont->render(timeScaleString.toLatin1().data(), Vector2f(dateX, dateY));
                }
            }

            /*
            QString frameCountString = QString("%1 fps").arg(m_framesPerSecond);
            QString texMemString = QString("%1 MB textures").arg(double(m_textureLoader->textureMemoryUsed()) / (1024 * 1024));
            m_textFont->render(frameCountString.toLatin1().data(), Vector2f(viewportWidth - 200.0f, 30.0f));
            m_textFont->render(texMemString.toLatin1().data(), Vector2f(viewportWidth - 200.0f, 10.0f));
            */

            // Display information about the selection
            if (m_selectedBody.isValid())
            {
                m_titleFont->bind();
                glColor4fv(titleColor.data());
                m_titleFont->render(m_selectedBody->name(), Vector2f(textLeftMargin, float(viewportHeight - titleFontHeight)));
                glColor4fv(textColor.data());
                m_textFont->bind();

                Vector3d r = m_observer->absolutePosition(m_simulationTime) - m_selectedBody->position(m_simulationTime);
                double distance = r.norm();

                bool isEllipsoidal = m_selectedBody->geometry() && m_selectedBody->geometry()->isEllipsoidal();
                if (isEllipsoidal)
                {
                    PlanetographicCoord3 pc = m_selectedBody->geometry()->ellipsoid().rectangularToPlanetographic(r);
                    distance = pc.height();
                }

                QString distanceString = tr("Distance: ") + readableDistance(distance, 6u);
                string distanceStdString = string(distanceString.toUtf8().constData());
                m_textFont->renderUtf8(distanceStdString, Vector2f(textLeftMargin, viewportHeight - 20.0f - titleFontHeight));

                // Display the subpoint for ellipsoidal bodies that are sufficiently close
                // to the observer.
                if (isEllipsoidal && distance < m_selectedBody->geometry()->ellipsoid().semiMajorAxisLength() * 5)
                {
                    float dx = m_textFont->textWidth(distanceStdString);
                    Vector3d q = m_selectedBody->orientation(m_simulationTime).conjugate() * r;

                    PlanetographicCoordHemi coord = getPlanetographicCoordinate(q, m_selectedBody.ptr());
                    QString coordString = QString(" (%1\260%2, %3\260%4)").
                            arg(coord.latitude, 0, 'f', 3).
                            arg(coord.latitudeHemiId).
                            arg(coord.longitude, 0, 'f', 3).
                            arg(coord.longitudeHemiId);

                    m_textFont->render(coordString.toLatin1().data(), Vector2f(textLeftMargin + dx, viewportHeight - 20.0f - (titleFontHeight)));
                }

                // Show the size of the selected object
                if (m_selectedBody->geometry())
                {
                    Geometry* geom = m_selectedBody->geometry();
                    double radius = geom->boundingSphereRadius();
                    if (dynamic_cast<WorldGeometry*>(geom))
                    {
                        radius = dynamic_cast<WorldGeometry*>(geom)->meanRadius();
                    }

                    // For now, only show the size of objects larger than spacecraft.
                    // TODO: Use the object class field to determine when something is spacecraft or now
                    if (radius > 0.01)
                    {
                        QString sizeString = tr("Radius: ") + readableDistance(radius, 4u);
                        m_textFont->render(sizeString.toLatin1().data(), Vector2f(textLeftMargin, viewportHeight - 20.0f - (titleFontHeight + textFontHeight)));
                    }
                }
            }

            // Show the globe tile download status
            {
                unsigned int tileCount = 0;

                if (m_textureLoader->wmsHandler())
                {
                    tileCount = m_textureLoader->wmsHandler()->pendingTileCount();
                }

                if (tileCount > 0)
                {
                    QString tileCountString = QString("Loading tiles: %1").arg(tileCount);
                    m_textFont->render(tileCountString.toLatin1().data(), Vector2f(textLeftMargin, viewportHeight - 20.0f - (titleFontHeight + textFontHeight * 2)));
                }
            }

#if 0
            {
                double fovX = atan(tan(m_fovY / 2.0) * double(width()) / double(height())) * 2.0;
                QString fovInfo = QString("%1\260 x %2\260 field of view").arg(toDegrees(fovX), 0, 'f', 1).arg(toDegrees(m_fovY), 0, 'f', 1);
                m_textFont->render(fovInfo.toLatin1().data(), Vector2f(float(viewportWidth / 2), 10.0f));
            }
#endif

            // Show the status message
            {
                float alpha = 1.0f - ((m_realTime - m_statusUpdateTime) / StatusMessageDuration);
                if (alpha > 0.0f)
                {
                    alpha = min(1.0f, alpha * 2.0f);
                    float textWidth = m_textFont->textWidth(m_statusMessage.toLatin1().data());
                    glColor4f(titleColor.x(), titleColor.y(), titleColor.z(), alpha);
                    m_textFont->render(m_statusMessage.toLatin1().data(), Vector2f(float(viewportWidth / 2) - textWidth / 2, viewportHeight - 20.0f));
                }
            }

        }
    }

    glDisable(GL_TEXTURE_2D);

    if (m_markers)
    {
        Viewport viewport(size().width() * devicePixelRatio(), size().height() * devicePixelRatio());
        PlanarProjection projection = PlanarProjection::CreatePerspective(m_fovY, viewport.aspectRatio(), 1.0f, 100.0f);
        Vector3d observerPosition = m_observer->absolutePosition(m_simulationTime);
        Quaterniond observerOrientation = m_observer->absoluteOrientation(m_simulationTime);

        m_markers->renderMarkers(observerPosition,
                                 observerOrientation,
                                 projection,
                                 viewport,
                                 m_simulationTime,
                                 m_realTime);

        if (m_centerIndicatorVisible)
        {
            Marker centerMarker;

            Entity* centerBody = m_observer->center();
            if (dynamic_cast<GotoObserverAction*>(m_observerAction.ptr()) != NULL)
            {
                // During a goto action, we mark the goto target instead; if the goto action is
                // allowed to complete, the goto target will become the center object
                centerBody = dynamic_cast<GotoObserverAction*>(m_observerAction.ptr())->target();
            }

            centerMarker.setBody(centerBody);
            centerMarker.setColor(Spectrum(0.8f, 0.8f, 1.0f));
            centerMarker.setSize(CenterMarkerSize * devicePixelRatio());
            centerMarker.setStyle(Marker::Spin);
            centerMarker.setTargetSizeThreshold(CenterMarkerSize / 2.0f);
            centerMarker.setDirectionIndicatorEnabled(true);
            m_markers->renderMarker(observerPosition,
                                    observerOrientation,
                                    projection,
                                    viewport,
                                    m_simulationTime,
                                    m_realTime,
                                    &centerMarker);
        }
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Intro fade animation
    if (m_realTime < 5.0)
    {
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f - float(m_realTime) / 5.0f);
        glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(viewportWidth, 0.0f);
        glVertex2f(viewportWidth, viewportHeight);
        glVertex2f(0.0f, viewportHeight);
        glEnd();
    }

    end2DDrawing();
}


void
UniverseView::drawFovReticle(float brightness)
{
    int viewportWidth = size().width();
    int viewportHeight = size().height();

    // Angular extent of reticle rings
    static const float reticleRingRadii[] = { 90.0f, 30.0f, 10.0f, 3.0f, 1.0f,
                                              20.0f / 60.0f, 5.0f / 60.0f, 1.0f / 60.0f };

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.5f);
    for (unsigned int rindex = 0; rindex < sizeof(reticleRingRadii) / sizeof(reticleRingRadii[0]); ++rindex)
    {
        const unsigned int subdivisions = 100;

        double fovX = atan(tan(m_fovY / 2.0) * double(width()) / double(height())) * 2.0;
        float radiusDeg = reticleRingRadii[rindex];

        float viewFraction = toRadians(radiusDeg) / float(max(m_fovY, fovX));
        float fade = 1.0f;
        if (viewFraction < 0.1f)
        {
            fade = max(0.0f, (viewFraction - 0.05f) / 0.05f);
        }
        else if (viewFraction > 1.0f)
        {
            fade = max(0.0f, (1.1f - viewFraction) * 10.0f);
        }

        fade *= brightness;

        if (fade > 0)
        {
            float radius = 0.5f * viewportHeight * toRadians(radiusDeg) / float(m_fovY);
            glColor4f(0.3f, 0.5f, 1.0f, fade);
            Vector2f center(viewportWidth * 0.5f, viewportHeight * 0.5f);
            const float gapPixels = 12.0f;
            float gap = 2.0f * asin(gapPixels / radius);
            drawArc(float(PI) * 0.5f + gap, float(PI) * 2.5f - gap, radius, center, subdivisions);

            glEnable(GL_TEXTURE_2D);
            QString label;
            if (radiusDeg >= 1.0)
            {
                label = QString("%1\260").arg(radiusDeg);
            }
            else
            {
                label = QString("%1'").arg(radiusDeg * 60.0f);
            }
            if (m_labelFont.isValid())
            {
                std::string labelStr = label.toLatin1().data();
                float labelWidth = m_labelFont->textWidth(labelStr);
                m_labelFont->bind();
                m_labelFont->render(labelStr, center + Vector2f(-labelWidth / 2.0f, radius - 8.0f));
            }
            glDisable(GL_TEXTURE_2D);
        }

    }
    glLineWidth(1.0f);
    glDisable(GL_LINE_SMOOTH);
}


void UniverseView::paintGL()
{
    // Update the frame counter
    double elapsedTime = secondsFromBaseTime();

    if (m_frameCount == 0)
    {
        m_frameCountStartTime = elapsedTime;
    }
    else if (elapsedTime - m_frameCountStartTime > 1.0)
    {
        m_framesPerSecond = m_frameCount / (elapsedTime - m_frameCountStartTime);
        m_frameCount = 0;
        m_frameCountStartTime = elapsedTime;
    }

    m_frameCount++;

    m_textureLoader->incrementFrameCount();
    m_textureLoader->evictTextures();
    m_textureLoader->realizeLoadedTextures();

    updateTrajectoryPlots();

    // Adjust the amount of glare based on the window size
    if (m_glareOverlay.isValid())
    {
        m_glareOverlay->setGlareSize(max(width(), height()) / 20.0f);
    }

    m_renderer->beginViewSet(m_universe.ptr(), m_simulationTime);

    if (m_reflectionsEnabled && !m_reflectionMap.isNull())
    {
        // Draw the reflection map; disable sky layers because they look bad when rendered
        // at low resolution into the reflection map. Visualizers are also disabled because
        // we want to reflect only physical geometry.
        Vector3d reflectionCenter = m_observer->absolutePosition(m_simulationTime);
        m_renderer->setVisualizersEnabled(false);
        m_renderer->setSkyLayersEnabled(false);

        // Set the near clip plane distance to 1km so that only distant objects are drawn
        // into the reflection map.
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        m_renderer->renderCubeMap(NULL, reflectionCenter, m_reflectionMap.ptr(), 1.0f);

        // Generate mipmaps
        // TODO: Move this into GLCubeMapFramebuffer
        /*
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_reflectionMap->cubeMapId());
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmapEXT(GL_TEXTURE_CUBE_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        */

        m_renderer->setVisualizersEnabled(true);
        m_renderer->setSkyLayersEnabled(true);
    }

    // Draw the 3D scene
    glDepthMask(GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Enable multisampling when we have a multisample render target
    if (qobject_cast<QGLWidget*>(viewport())->format().samples() > 1)// && GLEW_ARB_multisample)
    {
        glEnable(GL_MULTISAMPLE_ARB);
    }

    float pixelScale = window()->devicePixelRatio();
    Viewport mainViewport(size().width() * pixelScale, size().height() * pixelScale);
    LightingEnvironment lighting;
    if (m_reflectionsEnabled && m_reflectionMap.isValid())
    {
        // Add reflection map info to the lighting evironment
        lighting.reset();
        ReflectionRegion cameraRegion;
        cameraRegion.cubeMap = m_reflectionMap->colorTexture();
        cameraRegion.region = BoundingSphere<float>(Vector3f::Zero(), 1.0f);
        lighting.reflectionRegions().push_back(cameraRegion);
    }

    if (m_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

#ifdef LEO3D_SUPPORT
    if (m_leoState)
    {
        double leoProjection[16];
        //unsigned int leoCount = lglGetRenderCount(m_leoState->context);
        lglStartRender(m_leoState->context, 0, leoProjection);
    }
#endif

    if (m_stereoMode != Mono)
    {
        Quaterniond cameraOrientation = m_observer->absoluteOrientation(m_simulationTime);
        Vector3d cameraPosition = m_observer->absolutePosition(m_simulationTime);
        Entity* focusObject = m_observer->center();
        double focusObjectDistance = m_observer->position().norm();
        if (focusObject && focusObject->geometry() && focusObject->geometry()->isEllipsoidal())
        {
            double r = focusObject->geometry()->ellipsoid().semiMajorAxisLength();
            if (r < focusObjectDistance)
            {
                focusObjectDistance -= r;
            }
        }
        double eyeSeparation = focusObjectDistance / 60.0;
        float screenPlaneDistance = float(focusObjectDistance * 0.85);
        float nearDistance = 0.00001f;
        float farDistance = 1.0e12f;
        float y = tan(0.5f * m_fovY) * nearDistance;
        float x = y * mainViewport.aspectRatio();

        float frustumOffset = float(eyeSeparation) * nearDistance / screenPlaneDistance;

        Vector3d leftEyePosition = cameraPosition + cameraOrientation * (Vector3d::UnitX() * -eyeSeparation);
        Vector3d rightEyePosition = cameraPosition + cameraOrientation * (Vector3d::UnitX() * eyeSeparation);

        bool leo3dStereo = m_leoState != NULL;

        if (leo3dStereo)
        {
#if LEO3D_SUPPORT
            PlanarProjection leftProjection(PlanarProjection::Perspective,  -x + frustumOffset, x + frustumOffset, -y, y, nearDistance, farDistance);
            PlanarProjection rightProjection(PlanarProjection::Perspective, -x - frustumOffset, x - frustumOffset, -y, y, nearDistance, farDistance);

            Viewport halfHeightViewport(mainViewport.width(), mainViewport.height() / 2.0f);
            m_renderer->renderView(&lighting, rightEyePosition, cameraOrientation, rightProjection, halfHeightViewport);

            double projection[16];
            lglStartRender(m_leoState->context, 1, projection);
            m_renderer->renderView(&lighting, leftEyePosition, cameraOrientation, leftProjection, halfHeightViewport);

            LGLProjection projParams;
            lglGetProjection(m_leoState->context, 1, &projParams);
            qDebug() << "bottom right: " << projParams.rbX << "," << projParams.rbY << "     top left: " << projParams.ltX << "," << projParams.ltY;
#endif
        }
        else if (m_stereoMode == SideBySide)
        {
            x *= 0.5f;
            PlanarProjection leftProjection(PlanarProjection::Perspective,  -x + frustumOffset, x + frustumOffset, -y, y, nearDistance, farDistance);
            PlanarProjection rightProjection(PlanarProjection::Perspective, -x - frustumOffset, x - frustumOffset, -y, y, nearDistance, farDistance);

            Viewport leftViewport(0, 0, width() / 2 * pixelScale, height() * pixelScale);
            Viewport rightViewport(width() / 2 * pixelScale, 0, width() / 2 * pixelScale, height() * pixelScale);
            glEnable(GL_SCISSOR_TEST);
            glScissor(leftViewport.x(), leftViewport.y(), leftViewport.width(), leftViewport.height());
            m_renderer->renderView(&lighting, leftEyePosition, cameraOrientation, leftProjection, leftViewport);
            glScissor(rightViewport.x(), rightViewport.y(), rightViewport.width(), rightViewport.height());
            m_renderer->renderView(&lighting, rightEyePosition, cameraOrientation, rightProjection, rightViewport);
            glDisable(GL_SCISSOR_TEST);
        }
        else  // anaglyph stereo
        {
            PlanarProjection leftProjection(PlanarProjection::Perspective,  -x + frustumOffset, x + frustumOffset, -y, y, nearDistance, farDistance);
            PlanarProjection rightProjection(PlanarProjection::Perspective, -x - frustumOffset, x - frustumOffset, -y, y, nearDistance, farDistance);
            //PlanarProjection leftProjection(PlanarProjection::Perspective,  -x + float(frustumOffset), x, -y, y, nearDistance, farDistance);
            //PlanarProjection rightProjection(PlanarProjection::Perspective, -x, x - float(frustumOffset), -y, y, nearDistance, farDistance);

            glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_TRUE); // red
            //glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);  // green
            m_renderer->renderView(&lighting, leftEyePosition, cameraOrientation, leftProjection, mainViewport);
            glColorMask(GL_FALSE, GL_TRUE, GL_TRUE, GL_TRUE); // cyan
            //glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_TRUE);   // magenta
            glDepthMask(GL_TRUE);
            glClear(GL_DEPTH_BUFFER_BIT);
            m_renderer->renderView(&lighting, rightEyePosition, cameraOrientation, rightProjection, mainViewport);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
    }
    else
    {
        m_renderer->renderView(&lighting, m_observer.ptr(), m_fovY, mainViewport);
        if (m_sunGlareEnabled && m_glareOverlay.isValid())
        {
            m_glareOverlay->adjustBrightness();
            m_renderer->renderLightGlare(m_glareOverlay.ptr());
        }
    }

#ifdef LEO3D_SUPPORT
    if (m_leoState)
    {
        lglFinishRender(m_leoState->context);
    }
#endif

    if (m_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    m_renderer->endViewSet();

    // Capture the framebuffer *before* rendering the UI
    if (m_captureNextImage)
    {
        m_captureNextImage = false;
        QImage screenShot = grabFrameBuffer(false);
        QApplication::clipboard()->setImage(screenShot);
    }

#if FFMPEG_SUPPORT || QTKIT_SUPPORT
    if (m_videoEncoder)
    {
        int fbWidth = width();
        int fbHeight = height();

        float imageAspectRatio = float(fbWidth) / float(fbHeight);
        float videoAspectRatio = float(m_videoEncoder->getWidth()) / float(m_videoEncoder->getHeight());

        // Dimensions of the area of the screen that will be captured (and then scaled isotropically
        // to the video size.)
        int captureWidth = fbWidth;
        int captureHeight = fbHeight;

        if (videoAspectRatio > imageAspectRatio)
        {
            captureHeight = int(fbWidth / videoAspectRatio + 0.5f);
        }
        else
        {
            captureWidth = int(fbHeight * videoAspectRatio + 0.5f);
        }

        if (ShowTimeInVideos)
        {
            // Display the time and date in the video
            begin2DDrawing();
            m_titleFont->bind();
            QByteArray dateString = formatDate(GregorianDate::UTCDateFromTDBSec(m_simulationTime)).toUtf8();
            float textWidth = m_titleFont->textWidth(dateString.data());
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            m_titleFont->renderUtf8(dateString.data(), Vector2f((fbWidth - textWidth) / 2.0f, (fbHeight + captureHeight) / 2.0f - 45.0f));
            end2DDrawing();
        }

        QImage image = grabFrameBuffer(false);
        if (captureHeight < fbHeight)
        {
            image = image.copy(0, (image.height() - captureHeight) / 2, image.width(), captureHeight);
        }
        else
        {
            image = image.copy((image.width() - captureWidth) / 2, 0, captureWidth, image.height());
        }

        image = image.scaled(QSize(m_videoEncoder->getWidth(), m_videoEncoder->getHeight()), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
#if QTKIT_SUPPORT
        image = image.rgbSwapped();
#endif
        m_videoEncoder->encodeImage(image);

        drawFrame(captureWidth, captureHeight);
    }
#endif

    drawInfoOverlay();
}


void UniverseView::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}


void UniverseView::mousePressEvent(QMouseEvent *event)
{
    QDeclarativeView::mousePressEvent(event);

    m_lastMousePosition = event->pos();
    m_mouseMovement = 0;
}


void UniverseView::mouseReleaseEvent(QMouseEvent* event)
{
    setMouseClickEventProcessed(true);
    QDeclarativeView::mouseReleaseEvent(event);
    if (m_mouseClickEventProcessed)
    {
        return;
    }

    if (m_galleryView->isVisible())
    {
        if (m_galleryView->mouseReleased(Viewport(width(), height()), event->x(), event->y()))
        {
            int selected = m_galleryView->selectedTile();
            if (selected >= 0)
            {
                QString selectedName = QString::fromUtf8(m_galleryView->tileName(selected).c_str());
                setSelectedBody(selectedName);
                gotoSelectedObject();
            }
        }
        return;
    }

    // Process the mouse release as a click if the mouse hasn't moved
    // much since the mouse button was pressed
    if (m_mouseMovement < 4)
    {        
        if (event->button() == Qt::LeftButton)
        {
            // Mouse double click events can also generate release events; throw away release
            // events that occur too close to the last double click.
            if (secondsFromBaseTime() - m_lastDoubleClickTime > 0.2)
            {
                m_controller->stop();
                m_selectedBody = pickObject(event->pos());
                if (m_selectedBody.isValid())
                {
                    BodyInfo* info = m_catalog->findInfo(m_selectedBody->name().c_str());
                    Spectrum markerColor = Spectrum::White();
                    if (info)
                    {
                        markerColor = info->labelColor;
                    }

                    m_markers->addMarker(m_selectedBody.ptr(), markerColor, 20.0f, Marker::Pulse, m_realTime, 0.5);                    
                }
            }
        }
        else if (event->button() == Qt::RightButton)
        {
            // Right click invokes the context menu; lie about the origin of the context
            // menu event so that the event handler can distinguish standard mouse-generated
            // context menu events and discard them. Ordinary context menu events interfere with
            // right-dragging to pan the camera view.
            QContextMenuEvent menuEvent(QContextMenuEvent::Keyboard, event->pos(), event->globalPos());
            QCoreApplication::sendEvent(this, &menuEvent);
        }
    }
}


void UniverseView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QDeclarativeView::mouseDoubleClickEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        double t = secondsFromBaseTime();
        m_lastDoubleClickTime = t;
        Entity* clickedObject = pickObject(event->pos());
        if (clickedObject != NULL)
        {
            m_selectedBody = clickedObject;
            gotoSelectedObject();

            // Alternate double click behavior to center selection:
            // setObserverCenter();
            // m_observerAction = new CenterObserverAction(m_observer.ptr(), clickedObject, 1.0, t, m_simulationTime);
        }
    }
}


void UniverseView::mouseMoveEvent(QMouseEvent *event)
{
    setMouseMoveEventProcessed(true);
    QDeclarativeView::mouseMoveEvent(event);
    if (m_mouseMoveEventProcessed)
    {
        return;
    }

    if (m_galleryView->isVisible())
    {
        m_galleryView->mouseMoved(Viewport(width(), height()), event->x(), event->y());
        return;
    }

    int dx = event->x() - m_lastMousePosition.x();
    int dy = event->y() - m_lastMousePosition.y();
    m_mouseMovement += abs(dx) + abs(dy);

    // Mouse controls:
    // Left drag: orbit the target object
    // Right drag (or Alt+left drag): rotate the camera
    // Shift+left drag: dolly the camera

    bool leftButton = (event->buttons() & Qt::LeftButton) != 0;
    bool rightButton = (event->buttons() & Qt::RightButton) != 0;
    bool alt = (event->modifiers() & Qt::AltModifier) != 0;
    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0;

    if (leftButton && shift)
    {
        m_observer->changeDistance(std::pow(2.0, (double) dy / 200.0));
    }
    else if (rightButton && shift)
    {
        double zoomFactor = exp(dy / 1000.0);
        setFOV(m_fovY * zoomFactor);
        if (m_fovY < ReticleFOVThreshold)
        {
            m_reticleUpdateTime = m_realTime;
        }
    }
    else if (rightButton || (leftButton && alt))
    {
        // Right dragging changes the observer's orientation without
        // modifying the position. Rotate by an amount that depends on
        // the current field of view.
        double fovAdjust = toDegrees(m_fovY) / 25.0;
        double xrotation = (double) dy / 100.0 * fovAdjust;
        double yrotation = (double) dx / 100.0 * fovAdjust;

        m_controller->pitch(xrotation);
        m_controller->yaw(yrotation);
    }
    else if (leftButton)
    {
        // Left dragging makes the observer orbit the focus object
        double xrotation = (double) dy / 100.0;
        double yrotation = (double) dx / 100.0;

        // Reduce rotation speed when the center object is a planet and the
        // observer is close to the surface of the planet.
        Entity* center = m_observer->center();
        if (center)
        {
            WorldGeometry* world = dynamic_cast<WorldGeometry*>(center->geometry());
            if (world)
            {
                double distance = m_observer->position().norm() - world->maxRadius();
                float scale = distance / (world->maxRadius() * 0.1f);
                scale = min(1.0f, max(0.0f, scale));
                xrotation *= scale;
                yrotation *= scale;
            }
        }

        m_controller->applyOrbitTorque(Vector3d::UnitX() * -xrotation);
        m_controller->applyOrbitTorque(Vector3d::UnitY() * -yrotation);
    }

    m_lastMousePosition = event->pos();

    update();
}


void UniverseView::wheelEvent(QWheelEvent* event)
{
    QDeclarativeView::wheelEvent(event);
    if (event->isAccepted())
    {
        // Event was eaten by QML
        return;
    }

    if (event->orientation() == Qt::Vertical)
    {
        // Delta in steps of 1/8 of degree; typical
        // mouse rotation is 15 degrees per wheel click
        float clickZoom = 1.03f;
        float clickCount = event->delta() / 120.0f;
        double zoomFactor = std::pow(clickZoom, -clickCount / 50.0f);

        m_controller->dolly(zoomFactor);

        update();
    }
}


void
UniverseView::keyPressEvent(QKeyEvent* event)
{
    if (scene()->focusItem() && scene()->focusItem()->hasFocus())
    {
        QDeclarativeView::keyPressEvent(event);
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Left:
        m_rollLeft = true;
        break;
    case Qt::Key_Right:
        m_rollRight = true;
        break;
    case Qt::Key_Up:
        m_pitchUp = true;
        break;
    case Qt::Key_Down:
        m_pitchDown = true;
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}


void
UniverseView::keyReleaseEvent(QKeyEvent* event)
{
    if (scene()->focusItem() && scene()->focusItem()->hasFocus())
    {
        QDeclarativeView::keyReleaseEvent(event);
        return;
    }

    switch (event->key())
    {
    case Qt::Key_Left:
        m_rollLeft = false;
        break;
    case Qt::Key_Right:
        m_rollRight = false;
        break;
    case Qt::Key_Up:
        m_pitchUp = false;
        break;
    case Qt::Key_Down:
        m_pitchDown = false;
        break;
    case Qt::Key_Escape:
        if (m_observerAction.isValid())
        {
            setStatusMessage("Motion canceled");
            m_observerAction = NULL;
        }

        // Dismiss the gallery if it's visible
        if (m_galleryView && m_galleryView->isVisible())
        {
            m_galleryView->setVisible(false);
        }

        break;
    default:
        QWidget::keyReleaseEvent(event);
        break;
    }

    // Star brightness adjustment
    if (event->text() == "[")
    {
        setLimitingMagnitude(max(3.0, limitingMagnitude() - 0.2));
    }
    else if (event->text() == "]")
    {
        setLimitingMagnitude(min(13.0, limitingMagnitude() + 0.2));
    }

    // Alt+Shift+W enables wireframe mode
    // TODO: This should only be available in debug builds
    if (event->key() == Qt::Key_W && (event->modifiers() & Qt::AltModifier) && (event->modifiers() & Qt::ShiftModifier))
    {
        m_wireframe = !m_wireframe;
    }
}


bool
UniverseView::event(QEvent* e)
{
    if (e->type() == QEvent::Gesture)
    {
        return gestureEvent(static_cast<QGestureEvent*>(e));
    }
    else
    {
        if (e->type() == QEvent::Leave)
        {
            // Save the focus state for when the next focus in event arrives. We need
            // to do some special handling to prevent Qt from always setting the focus
            // back to an item in the declarative view.
            m_sceneHadFocus = scene()->hasFocus() && scene()->focusItem() != NULL;
        }
        return QDeclarativeView::event(e);
    }
}


bool
UniverseView::gestureEvent(QGestureEvent* event)
{
    QPinchGesture* pinch = static_cast<QPinchGesture*>(event->gesture(Qt::PinchGesture));
    if (pinch)
    {
        float fovScale = pinch->lastScaleFactor() / pinch->scaleFactor();

        // Setting the field of view will generate a status message; don't show this if the
        // FOV hasn't changed appreciably.
        if (abs(fovScale - 1.0f) > 1.0e-4f)
        {
            setFOV(m_fovY * fovScale);
            if (m_fovY < ReticleFOVThreshold)
            {
                m_reticleUpdateTime = m_realTime;
            }
        }

        float rotationAngle = float(toRadians(pinch->rotationAngle() - pinch->lastRotationAngle()));
        m_controller->roll(rotationAngle * 5);

        return true;
    }
    else
    {
        return false;
    }
}


void
UniverseView::contextMenuEvent(QContextMenuEvent* event)
{
    if (event->reason() == QContextMenuEvent::Mouse)
    {
        // Mouse triggered menu events interfere with right-dragging, so
        // we ignore them.
        return;
    }

    Entity* body = pickObject(event->pos());
    if (body)
    {
        BodyObject* o = new BodyObject(body);
        QDeclarativeEngine::setObjectOwnership(o, QDeclarativeEngine::JavaScriptOwnership);
        emit contextMenuTriggered(event->pos().x(), event->pos().y(), o);

        return;
        float visualizerSize = 1.0f;
        if (body->geometry())
        {
            visualizerSize = body->geometry()->boundingSphereRadius() * 2.0f;
        }

        // Build the context menu. The first item in the menu
        // is the object name (non-selectable.)
        QMenu* menu = new QMenu(this);
        QAction* nameAction = menu->addAction(bodyName(body));
        nameAction->setEnabled(false);

        QAction* centerAction = menu->addAction(tr("Set as Center"));
        QAction* gotoAction = menu->addAction(tr("Go to"));

        // Add actions for displaying reference vectors
        menu->addSeparator();
        QAction* bodyAxesAction = menu->addAction(tr("Body Axes"));
        QAction* frameAxesAction = menu->addAction(tr("Frame Axes"));
        QAction* velocityDirectionAction = menu->addAction("Velocity Vector");
        QAction* sunDirectionAction = NULL;
        QAction* earthDirectionAction = NULL;

        if (body->name() != "Sun")
        {
            sunDirectionAction = menu->addAction(tr("Sun Direction"));
        }
        if (body->name() != "Earth")
        {
            earthDirectionAction = menu->addAction(tr("Earth Direction"));
        }

        menu->addSeparator();
        QAction* plotTrajectoryAction = menu->addAction("Plot Trajectory");

        bodyAxesAction->setCheckable(true);
        bodyAxesAction->setChecked(body->visualizer("body axes") != NULL);
        frameAxesAction->setCheckable(true);
        frameAxesAction->setChecked(body->visualizer("frame axes") != NULL);
        velocityDirectionAction->setCheckable(true);
        velocityDirectionAction->setChecked(body->visualizer("velocity direction") != NULL);
        if (sunDirectionAction)
        {
            sunDirectionAction->setCheckable(true);
            sunDirectionAction->setChecked(body->visualizer("sun direction") != NULL);
        }
        if (earthDirectionAction)
        {
            earthDirectionAction->setCheckable(true);
            earthDirectionAction->setChecked(body->visualizer("earth direction") != NULL);
        }

        plotTrajectoryAction->setCheckable(true);
        vesta::Arc* arc = body->chronology()->activeArc(m_simulationTime);
        if (arc)
        {
            Entity* center = arc->center();
            if (center)
            {
                if (center->visualizer(TrajectoryVisualizerName(body)))
                {
                    plotTrajectoryAction->setChecked(true);
                }
            }
        }

        QAction* chosenAction = menu->exec(event->globalPos(), bodyAxesAction);
        if (chosenAction == centerAction)
        {
            setCenterAndFrame(body, m_observerFrame);
        }
        else if (chosenAction == gotoAction)
        {
            double distanceFromTarget = 500.0;
            if (body->geometry())
            {
                distanceFromTarget = body->geometry()->boundingSphereRadius() * 3.0;
            }

            m_observerAction = new GotoObserverAction(m_observer.ptr(),
                                                      body,
                                                      m_gotoObjectTime,
                                                      secondsFromBaseTime(),
                                                      m_simulationTime,
                                                      distanceFromTarget);

        }
        else if (chosenAction == bodyAxesAction)
        {
            if (bodyAxesAction->isChecked())
            {
                AxesVisualizer* axes = new AxesVisualizer(AxesVisualizer::BodyAxes, visualizerSize);
                axes->setLabelEnabled(true, ArrowGeometry::AllAxes);
                axes->setVisibility(true);
                //axes->arrows()->setMinimumScreenSize(100.0f);
                body->setVisualizer("body axes", axes);
            }
            else
            {
                body->removeVisualizer("body axes");
            }
        }
        else if (chosenAction == frameAxesAction)
        {
            if (frameAxesAction->isChecked())
            {
                AxesVisualizer* axes = new AxesVisualizer(AxesVisualizer::FrameAxes, visualizerSize);
                axes->setLabelEnabled(true, ArrowGeometry::AllAxes);
                axes->setVisibility(true);
                axes->arrows()->setOpacity(0.3f);
                //axes->arrows()->setMinimumScreenSize(100.0f);
                body->setVisualizer("frame axes", axes);
            }
            else
            {
                body->removeVisualizer("frame axes");
            }
        }
        else if (chosenAction == velocityDirectionAction)
        {
            if (velocityDirectionAction->isChecked())
            {
                VelocityVisualizer* arrow = new VelocityVisualizer(visualizerSize);
                arrow->setVisibility(true);
                arrow->setColor(Spectrum(0.25f, 1.0f, 1.0f));
                arrow->setLabelEnabled(true);
                arrow->setLabelText("Velocity");
                body->setVisualizer("velocity direction", arrow);
            }
            else
            {
                body->removeVisualizer("velocity direction");
            }
        }
        else if (chosenAction == sunDirectionAction && sunDirectionAction)
        {
            if (sunDirectionAction->isChecked())
            {
                ArrowVisualizer* arrow = new BodyDirectionVisualizer(visualizerSize, m_universe->findFirst("Sun"));
                arrow->setVisibility(true);
                arrow->setColor(Spectrum(1.0f, 1.0f, 0.7f));
                arrow->setLabelEnabled(true);
                arrow->setLabelText("Sun");
                body->setVisualizer("sun direction", arrow);
            }
            else
            {
                body->removeVisualizer("sun direction");
            }
        }
        else if (chosenAction == earthDirectionAction && earthDirectionAction)
        {
            if (earthDirectionAction->isChecked())
            {
                ArrowVisualizer* arrow = new BodyDirectionVisualizer(visualizerSize, m_universe->findFirst("Earth"));
                arrow->setVisibility(true);
                arrow->setColor(Spectrum(0.7f, 0.7f, 1.0f));
                arrow->setLabelEnabled(true);
                arrow->setLabelText("Earth");
                body->setVisualizer("earth direction", arrow);
            }
            else
            {
                body->removeVisualizer("earth direction");
            }
        }
        else if (chosenAction == plotTrajectoryAction)
        {
            if (plotTrajectoryAction->isChecked())
            {
                plotTrajectory(body, m_catalog->findInfo(QString::fromUtf8(body->name().c_str())));
            }
            else
            {
                clearTrajectoryPlots(body);
            }
        }
    }
}


void
UniverseView::focusOutEvent(QFocusEvent* event)
{
    QDeclarativeView::focusOutEvent(event);
}


void
UniverseView::focusInEvent(QFocusEvent* event)
{
    // Special handling for FocusIn events. By default, Qt is giving focus
    // back to the last active widget even if focus had been cleared. This
    // would be OK, except that some keyboard processing is handled by
    // UniverseView.
    if (m_sceneHadFocus)
    {
        QDeclarativeView::focusInEvent(event);
    }
    else
    {
        QAbstractScrollArea::focusInEvent(event);
    }
}


// Find the in the view underneath the specified point that is closest to the
// camera.
Entity*
UniverseView::pickObject(const QPoint& point)
{
    // Convert to world coordinates
    Quaterniond cameraOrientation = m_observer->absoluteOrientation(m_simulationTime);
    //Vector3d pickDirection = cameraOrientation * pickDirection;
    Vector3d pickOrigin = m_observer->absolutePosition(m_simulationTime);
    Vector2d pickPoint(point.x(), size().height() - point.y());
    Viewport viewport(size().width(), size().height());
    PlanarProjection projection = PlanarProjection::CreatePerspective(m_fovY, viewport.aspectRatio(), 1.0f, 100.0f);

    PickResult pickResult;
    //if (m_universe->pickObject(m_simulationTime, pickOrigin, pickDirection, pixelAngle, &pickResult))
    if (m_universe->pickViewportObject(m_simulationTime, pickPoint, pickOrigin, cameraOrientation, projection, viewport, &pickResult))
    {
#if 0
        // Debugging code to show pick coordinates in the local coordinate system of the
        // clicked object.
        Entity* hit = pickResult.hitObject();
        if (hit->geometry() && hit->geometry()->isEllipsoidal())
        {
            Vector3d p = pickResult.intersectionPoint() - hit->position(m_simulationTime);
            p = hit->orientation(m_simulationTime).conjugate() * p;
            qDebug() << "hit: " << p.x() << ", " << p.y() << ", " << p.z();
        }
#endif

        return pickResult.hitObject();
    }
    else
    {
        return NULL;
    }
}


// Constrain the viewer's position to lie within maxRange kilometers of the origin
void
UniverseView::constrainViewerPosition(double maxRange)
{
    Vector3d absPos = m_observer->absolutePosition(m_simulationTime);
    if (absPos.norm() > maxRange)
    {
        Vector3d constrainedPos = absPos.normalized() * maxRange;
        if (m_observer->center())
        {
            // Constrain the viewer's position to a point on the origin centered sphere of
            // radius MaxDistanceFromSun, but preserve the direction to the center object.
            Vector3d centerPos = m_observer->center()->position(m_simulationTime);

            // If the test below fails, it means that the center object is beyond the allowed
            // distance from the Sun.
            double d = 0.0;
            if (TestRaySphereIntersection(centerPos, (absPos - centerPos).normalized(), Vector3d::Zero(), maxRange, &d))
            {
                constrainedPos = centerPos + (absPos - centerPos).normalized() * d;
            }

            // Transform the constrained position into the observer's frame
            Vector3d p = m_observer->positionFrame()->orientation(m_simulationTime).conjugate() * (constrainedPos - centerPos);
            m_observer->setPosition(p);
        }
        else
        {
            m_observer->setPosition(constrainedPos);
        }
    }
}


#if TEST_SIMPLE_TRAJECTORY
class BasicTrajectoryPlotGenerator : public vesta::TrajectoryPlotGenerator
{
public:
    BasicTrajectoryPlotGenerator(const Trajectory* trajectory) :
        m_trajectory(trajectory)
    {
    }

    StateVector state(double t) const
    {
        return m_trajectory->state(t);
    }

    double startTime() const
    {
        return m_trajectory->startTime();
    }

    double endTime() const
    {
        return m_trajectory->endTime();
    }

private:
    const Trajectory* m_trajectory;
};
#endif


void
UniverseView::updateTrajectoryPlots()
{
    for (vector<TrajectoryPlotEntry>::const_iterator iter = m_trajectoryPlots.begin();
         iter != m_trajectoryPlots.end(); ++iter)
    {
        Visualizer* vis = iter->visualizer.ptr();
#if TEST_SIMPLE_TRAJECTORY
        SimpleTrajectoryGeometry* plot = dynamic_cast<SimpleTrajectoryGeometry*>(vis->geometry());
#else
        TrajectoryGeometry* plot = dynamic_cast<TrajectoryGeometry*>(vis->geometry());
#endif

        if (iter->generator)
        {
            plot->updateSamples(iter->generator, m_simulationTime - plot->windowDuration(), m_simulationTime, iter->sampleCount);
        }
        else if (iter->trajectory.isValid())
        {
            double startTime = m_simulationTime - plot->windowDuration() + plot->windowLead();
            double endTime = m_simulationTime + plot->windowLead();

            startTime = max(startTime, iter->trajectory->startTime());
            endTime = min(endTime, iter->trajectory->endTime());

#if TEST_SIMPLE_TRAJECTORY
            BasicTrajectoryPlotGenerator gen(iter->trajectory.ptr());
            plot->updateSamples(&gen, m_simulationTime - plot->windowDuration(), m_simulationTime, iter->sampleCount);
#else
            plot->updateSamples(iter->trajectory.ptr(), startTime, endTime, iter->sampleCount);
#endif
        }
    }
}


TextureMap*
UniverseView::loadTexture(const QString& location, const TextureProperties& texProps)
{
    return m_textureLoader->loadTexture(location.toUtf8().data(), texProps);
}


void
UniverseView::initializeObserver()
{
    Entity* center = new Entity();
    m_observer = new Observer(center);
    m_observer->addRef();
    m_observer->setPosition(Vector3d(0.0, 0.0, 1.0e9));

    m_controller->setObserver(m_observer.ptr());
}


void
UniverseView::initializeSkyLayers()
{
    // Add coordinate grids: equatorial and the ecliptic
    CelestialCoordinateGrid* equatorialGrid = new CelestialCoordinateGrid();
    equatorialGrid->setColor(Spectrum(0.2f, 0.2f, 0.5f));
    equatorialGrid->setVisibility(false);
    m_universe->setLayer("equatorial grid", equatorialGrid);

    CelestialCoordinateGrid* ecliptic = new CelestialCoordinateGrid();
    ecliptic->setGridStyle(CelestialCoordinateGrid::EquatorOnly);
    ecliptic->setOrientation(InertialFrame::eclipticJ2000()->orientation(0.0));
    ecliptic->setColor(Spectrum(0.6f, 0.0f, 0.0f));
    ecliptic->setVisibility(false);
    m_universe->setLayer("ecliptic", ecliptic);

    StarsLayer* starsLayer = new StarsLayer(m_universe->starCatalog());
    starsLayer->setLimitingMagnitude(8.0f);
    starsLayer->setVisibility(true);
    m_universe->setLayer("stars", starsLayer);

#ifdef Q_OS_MAC
    // The ATI Radeon X1600 seems to have troubles rendering points with fragment shaders enabled.
    // In order to prevent them from appearing like white squares, we'll fall back to point stars
    // when the renderer string contains X1600. This problem has only been observed on the Mac (so far)
    QString glRenderer = QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
    if (glRenderer.contains("X1600"))
    {
        starsLayer->setStyle(StarsLayer::PointStars);
    }
#endif

    SkyImageLayer* milkyWayLayer = new SkyImageLayer();
    milkyWayLayer->setOpacity(0.3f);
    milkyWayLayer->setDrawOrder(-1);
    milkyWayLayer->setTexture(m_textureLoader->loadTexture("textures/milkyway.jpg", SkyLayerTextureProperties()));

    // Adjustment applied because the ESO all-sky map is not exactly aligned with the galactic coordinate system.
    // TODO: Sky images should be set up in a configuration file, not hard-coded.
    Quaterniond rotationOffset(AngleAxisd(toRadians(3.8767255), Vector3d(-0.5411, -0.6441, 0.5406)));
    milkyWayLayer->setOrientation(rotationOffset * InertialFrame::galactic()->orientation());

    m_universe->setLayer("milky way", milkyWayLayer);
    milkyWayLayer->setVisibility(false);

    ConstellationsLayer* constellations = new ConstellationsLayer(m_universe->starCatalog());
    constellations->setDiagramColor(Spectrum(0.0f, 0.2f, 0.5f));
    constellations->setDefaultConstellations();
    constellations->setVisibility(false);
    m_universe->setLayer("constellation figures", constellations);

    SkyLabelLayer* constellationNamesLayer = new SkyLabelLayer();
    constellationNamesLayer->setLabelCulling(false);
    constellationNamesLayer->setVisibility(false);

    constellationNamesLayer->setFont(m_textFont.ptr());
    Spectrum labelColor(0.2f, 0.1f, 0.6f);
    const vector<ConstellationInfo>& allConstellations = ConstellationInfo::constellations();
    for (unsigned int i = 0; i < allConstellations.size(); ++i)
    {
        const ConstellationInfo& info = allConstellations[i];
        constellationNamesLayer->addLabel(info.name(),
                                          toRadians(info.labelLocation().y()),
                                          toRadians(info.labelLocation().x() * 15.0f),
                                          labelColor);
    }
    m_universe->setLayer("constellation names", constellationNamesLayer);
}


// Return the number of seconds since the base time
double
UniverseView::secondsFromBaseTime() const
{
    QDateTime currentTime = QDateTime::currentDateTime();
    double t = 86400.0 * m_baseTime.daysTo(currentTime) +
               0.001 * QTime(0, 0, 0, 0).msecsTo(currentTime.time());

    return t;
}


void
UniverseView::tick()
{
    double t = secondsFromBaseTime();

    if (m_firstTick)
    {
        m_firstTick = false;
        m_lastTickTime = t;
    }

    double dt = t - m_lastTickTime;
    m_lastTickTime = t;

#if FFMPEG_SUPPORT || QTKIT_SUPPORT
    if (m_videoEncoder)
    {
        // Lock time step when recording video
        dt = 1.0 / 30.0;
    }
#endif

    if (m_galleryView)
    {
        m_galleryView->update(dt);
    }

    m_realTime += dt;

    if (!isPaused())
    {
        setSimulationTime(m_simulationTime + dt * timeScale());
    }

    if (m_videoEncoder)
    {
        // If we're recording video, notify any listeners that the length has
        // changed.
        emit recordedVideoLengthChanged(recordedVideoLength());
    }

    if (m_rollLeft)
    {
        m_controller->roll(dt * KeyboardRotationAcceleration);
    }
    else if (m_rollRight)
    {
        m_controller->roll(-dt * KeyboardRotationAcceleration);
    }
    else if (m_pitchUp)
    {
        m_controller->pitch(dt * KeyboardRotationAcceleration);
    }
    else if (m_pitchDown)
    {
        m_controller->pitch(-dt * KeyboardRotationAcceleration);
    }

#ifdef LEO3D_SUPPORT
    if (m_leoState)
    {
        m_leoState->updateInputDeviceState();

        // View
        //m_controller->observer()->rotate(m_leoState->birdOrientationChange().conjugate());
        //m_controller->observer()->orbit(m_leoState->birdOrientationChange().conjugate());

        if (m_leoState->birdSmallButton())
        {
            float rotationScale = 1.0f;

            // Reduce rotation speed when the center object is a planet and the
            // observer is close to the surface of the planet.
            Entity* center = m_observer->center();
            if (center && center->geometry())
            {
                if (center->geometry()->isEllipsoidal())
                {
                    double r = center->geometry()->ellipsoid().semiMajorAxisLength();
                    double distance = m_observer->position().norm() - r;
                    rotationScale = distance / (r * 0.1f);
                    rotationScale = min(1.0f, max(0.0f, rotationScale));
                }
            }

            AngleAxisd aa = m_leoState->birdOrientation().conjugate();
            m_controller->applyOrbitTorque(aa.axis() * (rotationScale * aa.angle() * 0.2));

            //float clickZoom = 1.03f;
            //float clickCount = event->delta() / 120.0f;
            double zoomFactor = std::pow(2.0, -m_leoState->birdPositionChange().z() / 100.0);

            m_controller->dolly(zoomFactor);

        }

    }
#endif

    m_controller->tick(dt);

    if (m_observerAction.isValid())
    {
        bool complete = m_observerAction->updateObserver(m_observer.ptr(), t, m_simulationTime);
        if (complete)
        {
            m_observerAction = NULL;
            m_observer->updatePointingFrame(InertialFrame::icrf(), m_simulationTime);
            m_observer->updatePositionFrame(InertialFrame::icrf(), m_simulationTime);
        }
    }

    constrainViewerPosition(MaximumDistanceFromSun);

    viewport()->update();

    if (m_markers)
    {
        m_markers->expireMarkers(m_realTime);
    }

    if (dt != 0.0)
    {
        emit timeChanged();
    }
}


void
UniverseView::setCenterAndFrame(Entity* center, FrameType f, const Eigen::Vector3d& refVector)
{
    m_observerFrame = f;

    Frame* frame = NULL;
    if (f == Frame_BodyFixed)
    {
        frame = new BodyFixedFrame(center);
    }
    else if (f == Frame_Synodic)
    {
        Entity* target = center->chronology()->firstArc()->center();
        if (target)
        {
            frame = new TwoBodyRotatingFrame(target, center);
        }
        else
        {
            frame = InertialFrame::equatorJ2000();
        }
    }
    else if (f == Frame_Locked)
    {
        if (center == m_selectedBody.ptr() || m_selectedBody.isNull())
        {
            frame = InertialFrame::equatorJ2000();
        }
        else
        {
            RelativePositionVector* primary = new RelativePositionVector(center, m_selectedBody.ptr());
            RelativeVelocityVector* secondary = new RelativeVelocityVector(center, m_selectedBody.ptr());
            frame = new TwoVectorFrame(primary, TwoVectorFrame::NegativeZ, secondary, TwoVectorFrame::PositiveY);
        }
    }
    else if (f == Frame_LockedLevel)
    {
        if (center == m_selectedBody.ptr() || m_selectedBody.isNull())
        {
            frame = InertialFrame::equatorJ2000();
        }
        else
        {
            RelativePositionVector* primary = new RelativePositionVector(center, m_selectedBody.ptr());
            ConstantFrameDirection* secondary = new ConstantFrameDirection(InertialFrame::icrf(), refVector);
            frame = new TwoVectorFrame(primary, TwoVectorFrame::NegativeZ, secondary, TwoVectorFrame::PositiveY);
        }
    }
    else
    {
        frame = InertialFrame::equatorJ2000();
    }

    m_observer->updateCenter(center, m_simulationTime);
    m_observer->updatePositionFrame(frame, m_simulationTime);
    m_observer->updatePointingFrame(frame, m_simulationTime);

    if (center)
    {
        setStatusMessage(QString("Set center to %1").arg(center->name().c_str()));
    }
}


void
UniverseView::setObserverCenter()
{
    if (m_selectedBody.isValid())
    {
        setCenterAndFrame(m_selectedBody.ptr(), Frame_Inertial);
    }
}


void
UniverseView::setPaused(bool paused)
{
    getStateUrl();
    if (paused != m_paused)
    {
        m_paused = paused;
        emit pauseStateChanged(paused);
    }
}


void
UniverseView::setCurrentTime()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    GregorianDate date = GregorianDate(currentTime.date().year(), currentTime.date().month(), currentTime.date().day(),
                                       currentTime.time().hour(), currentTime.time().minute(), currentTime.time().second(),
                                       currentTime.time().msec() * 1000);
    m_simulationTime = date.toTDBSec();
}


void
UniverseView::setTimeScale(double timeScale)
{
    if (timeScale != m_timeScale)
    {
        m_timeScale = timeScale;
        emit timeScaleChanged(timeScale);
    }
}


/** Set the simulation time.
  *
  * \param tsec seconds since J2000 TDB
  */
void
UniverseView::setSimulationTime(double tsec)
{
    m_simulationTime = std::max(StartOfTime, std::min(EndOfTime, tsec));
    emit simulationDateTimeChanged();
}


/** Get the current simulation time as a Qt QDateTime structure.
  * Note that Qt's DateTime structure doesn't handle leap seconds.
  */
QDateTime
UniverseView::simulationDateTime() const
{
    GregorianDate calendarDate = GregorianDate::UTCDateFromTDBSec(m_simulationTime);
    return QDateTime(QDate(calendarDate.year(), calendarDate.month(), calendarDate.day()),
                     QTime(calendarDate.hour(), calendarDate.minute(), std::min(59u, calendarDate.second()), calendarDate.usec() / 1000),
                     Qt::UTC);
}


void
UniverseView::setSimulationDateTime(QDateTime dateTime)
{
    GregorianDate startDate(dateTime.date().year(), dateTime.date().month(), dateTime.date().day(),
                            dateTime.time().hour(), dateTime.time().minute(), dateTime.time().second(),
                            dateTime.time().msec() * 1000);
    setSimulationTime(startDate.toTDBSec());
}


void
UniverseView::bodyFixedObserver(bool checked)
{
    if (checked)
    {
        setCenterAndFrame(m_observer->center(), Frame_BodyFixed);
        setStatusMessage(QString("Set %1 fixed frame").arg(bodyName(m_observer->center())));
    }
}


void
UniverseView::inertialObserver(bool checked)
{
    if (checked)
    {
        setCenterAndFrame(m_observer->center(), Frame_Inertial);
    }
}


void
UniverseView::synodicObserver(bool checked)
{
    if (checked)
    {
        setCenterAndFrame(m_observer->center(), Frame_Synodic);
    }
}


void
UniverseView::lockedObserver(bool checked)
{
    if (checked)
    {
        setCenterAndFrame(m_observer->center(), Frame_Locked);
    }
}


void
UniverseView::setMilkyWayVisible(bool checked)
{
    SkyLayer* layer = m_universe->layer("milky way");
    if (layer)
    {
        layer->setVisibility(checked);
    }
}


void
UniverseView::setPlanetOrbitsVisibility(bool enabled)
{
    const char* planetNames[] =
    {
        "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Moon"
    };

    for (unsigned int i = 0; i < sizeof(planetNames) / sizeof(planetNames[0]); ++i)
    {
        Entity* planet = m_catalog->find(planetNames[i]);
        BodyInfo* info = m_catalog->findInfo(planetNames[i]);

        if (enabled)
        {
            plotTrajectory(planet, info);
        }
        else
        {
            clearTrajectoryPlots(planet);
        }
    }

    if (enabled != m_planetOrbitsVisible)
    {
        m_planetOrbitsVisible = enabled;
        //emit planetOrbitsVisibileChanged();
    }
}


void
UniverseView::setEclipticVisibility(bool checked)
{
    setSkyLayerVisible("ecliptic", checked);
}


void
UniverseView::setEquatorialPlaneVisibility(bool checked)
{
    Entity* earth = m_universe->findFirst("Earth");
    if (earth)
    {
        if (checked)
        {
            PlaneVisualizer* plane = new PlaneVisualizer(12000.0);
            plane->setFrame(InertialFrame::equatorJ2000());
            plane->plane()->setColor(Spectrum(0.1f, 0.5f, 0.5f));
            plane->plane()->setGridLineSpacing(1000.0);
            earth->setVisualizer("Equatorial Plane", plane);
        }
        else
        {
            earth->removeVisualizer("Equatorial Plane");
        }
    }
}


void
UniverseView::setPlanetographicGridVisibility(bool enable)
{
    Entity* earth = m_universe->findFirst("Earth");
    if (earth)
    {
        WorldGeometry* world = dynamic_cast<WorldGeometry*>(earth->geometry());
        if (world)
        {
            if (enable)
            {
                PlanetGridLayer* grid = new PlanetGridLayer();
                grid->setVisibility(true);
                grid->setGridOpacity(0.33f);
                world->setLayer("grid", grid);
            }
            else
            {
                world->removeLayer("grid");
            }
        }
    }
}


void
UniverseView::setLabelVisibility(bool enable)
{
    m_labelsVisible = enable;
    foreach (QString name, m_catalog->names())
    {
        Entity* body = m_catalog->find(name);
        if (body)
        {
            Visualizer* vis = body->visualizer("label");
            if (vis)
            {
                vis->setVisibility(enable);
            }
        }
    }
}


void
UniverseView::setSurfaceFeatureLabelVisibility(bool enable)
{
    m_surfaceFeatureLabelsVisible = enable;
    FeatureLabelSetGeometry::setGlobalOpacity(enable ? 1.0f : 0.0f);
}


void
UniverseView::setCenterIndicatorVisibility(bool enable)
{
    if (enable != m_centerIndicatorVisible)
    {
        m_centerIndicatorVisible = enable;
        emit centerIndicatorVisibilityChanged(enable);
    }
}


bool
UniverseView::skyLayerVisible(const std::string& layerName) const
{
    SkyLayer* layer = m_universe->layer(layerName);
    if (layer)
    {
        return layer->isVisible();
    }
    else
    {
        return false;
    }
}


void
UniverseView::setSkyLayerVisible(const std::string& layerName, bool checked)
{
    SkyLayer* layer = m_universe->layer(layerName);
    if (layer)
    {
        layer->setVisibility(checked);
    }
}


bool
UniverseView::constellationFigureVisibility() const
{
    return skyLayerVisible("constellation figures");
}


bool
UniverseView::constellationNameVisibility() const
{
    return skyLayerVisible("constellation names");
}


bool
UniverseView::starNameVisibility() const
{
    return skyLayerVisible("star names");
}


bool UniverseView::equatorialGridVisibility() const
{
    return skyLayerVisible("equatorial grid");
}


void
UniverseView::setEquatorialGridVisibility(bool checked)
{
    if (checked != equatorialGridVisibility())
    {
        setSkyLayerVisible("equatorial grid", checked);
        emit equatorialGridVisibilityChanged(checked);
    }
}


bool UniverseView::planetOrbitsVisibility() const
{
    return m_planetOrbitsVisible;
}


bool UniverseView::eclipticVisibility() const
{
    return skyLayerVisible("ecliptic");
}


void
UniverseView::setConstellationFigureVisibility(bool checked)
{
    setSkyLayerVisible("constellation figures", checked);
}


void
UniverseView::setConstellationNameVisibility(bool checked)
{
    setSkyLayerVisible("constellation names", checked);
}


void
UniverseView::setStarNameVisibility(bool checked)
{
    setSkyLayerVisible("star names", checked);
}


bool
UniverseView::milkyWayVisible() const
{
    return skyLayerVisible("milky way");
}


void
UniverseView::setStatusMessage(const QString& message)
{
    m_statusMessage = message;
    m_statusUpdateTime = m_realTime;
}


void
UniverseView::plotTrajectory(Entity* body, const BodyInfo* info)
{
    if (!body)
    {
        return;
    }

    vesta::Arc* arc = body->chronology()->activeArc(m_simulationTime);
    if (!arc || !arc->center())
    {
        return;
    }

    string visName = TrajectoryVisualizerName(body);
    Visualizer* oldVisualizer = arc->center()->visualizer(visName);

#if TEST_SIMPLE_TRAJECTORY
    SimpleTrajectoryGeometry* plot = new SimpleTrajectoryGeometry();
#else
    TrajectoryGeometry* plot = new TrajectoryGeometry();
#endif
    Visualizer* visualizer = new Visualizer(plot);

    plot->setFrame(arc->trajectoryFrame());

    double duration = 0.0;
    Spectrum color = Spectrum::White();
    double lead = 0.0;
    double fade = 0.0;
    unsigned int sampleCount = 100;
    if (info)
    {
        duration = info->trajectoryPlotDuration;
        lead = info->trajectoryPlotLead;
        fade = info->trajectoryPlotFade;
        color = info->trajectoryPlotColor;
        sampleCount = info->trajectoryPlotSamples;
    }

    if (duration <= 0.0)
    {
        duration = arc->trajectory()->period() * 0.99;
    }

    plot->setWindowDuration(duration);
    plot->setWindowLead(lead);
    plot->setDisplayedPortion(TrajectoryGeometry::WindowBeforeCurrentTime);
    plot->setFadeFraction(fade);
    plot->setColor(color * 0.5f);
    //plot->setLineWidth(1.5f);
    arc->center()->setVisualizer(visName, visualizer);

    TrajectoryPlotEntry plotEntry;
    plotEntry.visualizer = visualizer;
    plotEntry.generator = NULL;
    plotEntry.sampleCount = sampleCount;

    const InterpolatedStateTrajectory* interpolatedTraj = dynamic_cast<const InterpolatedStateTrajectory*>(arc->trajectory());
    if (interpolatedTraj)
    {
        // Special handling for interpolated trajectories: we just use the states from the
        // trajectory for the plot so that the plotted line follows the spacecraft motion
        // exactly (both plotting and InterpolateStateTrajectory use cubic Hermite interpolation)
        for (unsigned int i = 0; i < interpolatedTraj->stateCount(); ++i)
        {
            double t = interpolatedTraj->time(i);
            plot->addSample(t, interpolatedTraj->state(t));
        }
        plotEntry.trajectory = NULL;
    }
    else
    {
        plotEntry.trajectory = arc->trajectory();
    }

    // Remove the old entry (if there was one)
    if (oldVisualizer)
    {
        for (vector<TrajectoryPlotEntry>::iterator iter = m_trajectoryPlots.begin(); iter != m_trajectoryPlots.end(); ++iter)
        {
            if (iter->visualizer.ptr() == oldVisualizer)
            {
                m_trajectoryPlots.erase(iter);
                break;
            }
        }
    }

    m_trajectoryPlots.push_back(plotEntry);
}


/** Clear all trajectory plots for a body.
  */
void
UniverseView::clearTrajectoryPlots(Entity* body)
{
    if (!body)
    {
        return;
    }

    // Clear all trajectory visualizers (there may be one per trajectory arc)
    for (unsigned int i = 0; i < body->chronology()->arcCount(); ++i)
    {
        vesta::Arc* arc = body->chronology()->arc(i);
        if (arc->center())
        {
            arc->center()->removeVisualizer(TrajectoryVisualizerName(body));
        }
    }
}


/** Return true if there are trajectory plots for the specified body.
  */
bool
UniverseView::hasTrajectoryPlots(Entity* body) const
{
    if (!body)
    {
        return false;
    }

    vesta::Arc* arc = body->chronology()->activeArc(m_simulationTime);
    if (!arc)
    {
        return false;
    }

    string visName = TrajectoryVisualizerName(body);
    Visualizer* vis = NULL;
    if (arc->center())
    {
        vis = arc->center()->visualizer(visName);
    }

    return vis != NULL;
}


/** Plot a trajectory for a body using the default plotting
  * parameters.
  *
  * This method is used by the script interface to UniverseView.
  */
void
UniverseView::plotTrajectory(QObject *bodyObj)
{
    BodyObject* body = qobject_cast<BodyObject*>(bodyObj);
    if (!body || !body->body())
    {
        return;
    }

    BodyInfo* info = m_catalog->findInfo(body->body());
    plotTrajectory(body->body(), info);
}


/** Clear all trajectory plots for a body.
  *
  * This method is used by the script interface to UniverseView.
  */
void
UniverseView::clearTrajectoryPlots(QObject* bodyObj)
{
    BodyObject* body = qobject_cast<BodyObject*>(bodyObj);
    if (body && body->body())
    {
        clearTrajectoryPlots(body->body());
    }
}


/** Return true if there are any trajectory plots visible for a body.
  *
  * This method is used by the script interface to UniverseView.
  */
bool
UniverseView::hasTrajectoryPlots(QObject* bodyObj) const
{
    BodyObject* body = qobject_cast<BodyObject*>(bodyObj);
    if (!body || !body->body())
    {
        return false;
    }
    else
    {
        return hasTrajectoryPlots(body->body());
    }
}


class BodyPositionSampleGenerator : public TrajectoryPlotGenerator
{
public:
    BodyPositionSampleGenerator(Entity* body, Entity* center, Frame* frame) :
        m_body(body),
        m_center(center),
        m_frame(frame)
    {
    }

    StateVector state(double t) const
    {
        StateVector state_emej2000 = m_body->state(t) - m_center->state(t);
        Matrix<double, 6, 6> stateTransform = m_frame->inverseStateTransform(t);
        return StateVector(stateTransform * state_emej2000.state());
    }

    double startTime() const
    {
        return m_body->chronology()->beginning();
    }

    double endTime() const
    {
        return m_body->chronology()->ending();
    }

private:
    counted_ptr<Entity> m_body;
    counted_ptr<Entity> m_center;
    counted_ptr<Frame> m_frame;
};



void
UniverseView::plotTrajectoryObserver(const BodyInfo* info)
{
    Spectrum color = Spectrum::White();
    if (info != NULL)
    {
        color = info->trajectoryPlotColor;
    }

    if (m_selectedBody.isValid())
    {
        Entity* center = m_observer->center();
        Frame* frame = m_observer->positionFrame();
        string visName = TrajectoryVisualizerName(m_selectedBody.ptr());
        Visualizer* vis = center->visualizer(visName);
        if (!vis)
        {
            TrajectoryGeometry* plot = new TrajectoryGeometry();
            Visualizer* visualizer = new Visualizer(plot);
            plot->setFrame(frame);
            plot->setWindowDuration(daysToSeconds(2.0));
            plot->setDisplayedPortion(TrajectoryGeometry::WindowBeforeCurrentTime);
            plot->setFadeFraction(0.5);
            plot->setColor(color);
            center->setVisualizer(visName, visualizer);

            TrajectoryPlotEntry plotEntry;
            plotEntry.trajectory = NULL;
            plotEntry.visualizer = visualizer;
            plotEntry.generator = new BodyPositionSampleGenerator(m_selectedBody.ptr(), center, frame);
            plotEntry.sampleCount = 1000;
            m_trajectoryPlots.push_back(plotEntry);
        }
    }
}


bool
UniverseView::shadows() const
{
    return m_renderer->shadowsEnabled();
}


void
UniverseView::setShadows(bool enable)
{
    //m_renderer->setShadowsEnabled(enable);
}


bool
UniverseView::eclipseShadows() const
{
    return m_renderer->eclipseShadowsEnabled();
}


void
UniverseView::setEclipseShadows(bool enable)
{
    m_renderer->setEclipseShadowsEnabled(enable);
}


bool
UniverseView::reflections() const
{
    return m_reflectionsEnabled;
}


void
UniverseView::setReflections(bool enable)
{
    m_reflectionsEnabled = enable;
}


bool
UniverseView::atmospheresVisible() const
{
    return WorldGeometry::atmospheresVisible();
}


void
UniverseView::setAtmospheresVisible(bool enable)
{
    WorldGeometry::setAtmospheresVisible(enable);
}


bool
UniverseView::cloudsVisible() const
{
    return WorldGeometry::cloudLayersVisible();
}


void
UniverseView::setCloudsVisible(bool enable)
{
    WorldGeometry::setCloudLayersVisible(enable);
}


void
UniverseView::setAmbientLight(bool enable)
{
    float light = enable ? 0.15f : 0.0f;
    m_renderer->setAmbientLight(Spectrum(light, light, light));
}


double
UniverseView::limitingMagnitude() const
{
    StarsLayer* stars = dynamic_cast<StarsLayer*>(m_universe->layer("stars"));
    return double(stars->limitingMagnitude());
}


void
UniverseView::setLimitingMagnitude(double appMag)
{
    StarsLayer* stars = dynamic_cast<StarsLayer*>(m_universe->layer("stars"));
    if (stars->limitingMagnitude() != (float) appMag)
    {
        stars->setLimitingMagnitude(float(appMag));
        emit limitingMagnitudeChanged(appMag);
    }
}


double
UniverseView::ambientLight() const
{
    return double(m_renderer->ambientLight().green());
}


void
UniverseView::setAmbientLight(double brightness)
{
    double currentBrightness = m_renderer->ambientLight().green();
    if (currentBrightness != brightness)
    {
        m_renderer->setAmbientLight(Spectrum::Flat(float(brightness)));
        emit ambientLightChanged(brightness);
    }
}


bool
UniverseView::sunGlare() const
{
    return m_sunGlareEnabled;
}


void
UniverseView::setSunGlare(bool enable)
{
    m_sunGlareEnabled = enable;
}


bool
UniverseView::diffractionSpikes() const
{
    StarsLayer* stars = dynamic_cast<StarsLayer*>(m_universe->layer("stars"));
    if (stars)
    {
        return stars->diffractionSpikeBrightness() > 0.0f;
    }
    else
    {
        return false;
    }
}


void
UniverseView::setDiffractionSpikes(bool enable)
{
    StarsLayer* stars = dynamic_cast<StarsLayer*>(m_universe->layer("stars"));
    if (stars)
    {
        stars->setDiffractionSpikeBrightness(enable ? 0.3f : 0.0f);
    }
}


void
UniverseView::setStereoMode(StereoMode stereoMode)
{
    m_stereoMode = stereoMode;
#ifdef LEO3D_SUPPORT
    if (m_leoState)
    {
        leoSetStereo(m_stereoMode != Mono);
    }
#endif
}


/** Set the number of anti-aliasing samples to use for multisampling. Setting
  * the sample count to 1 will disable anti-aliasing. The results of changing
  * the anti-aliasing mode do not take effect until the application is restarted.
  */
void
UniverseView::setAntialiasingSamples(int samples)
{
    m_antialiasingSamples = std::max(1, std::min(MaxAntialiasingSampleCount, samples));

    // Don't set anything, just save the value in the settings.
    QSettings settings;
    settings.setValue("AntialiasingSamples", m_antialiasingSamples);
}


void
UniverseView::setInfoText(bool enable)
{
    m_infoTextVisible = enable;
}


void
UniverseView::startVideoRecording(QVideoEncoder* encoder)
{
    m_videoEncoder = encoder;
    m_videoRecordingStartTime = m_realTime;
    emit recordingVideoChanged();
}


void
UniverseView::finishVideoRecording()
{
    m_videoEncoder = NULL;
    emit recordingVideoChanged();
}


double
UniverseView::recordedVideoLength() const
{
    if (m_videoEncoder)
    {
        return m_realTime - m_videoRecordingStartTime;
    }
    else
    {
        return 0.0;
    }
}


QImage
UniverseView::grabFrameBuffer(bool withAlpha)
{
    return dynamic_cast<QGLWidget*>(viewport())->grabFrameBuffer(withAlpha);
}


void
UniverseView::setSelectedBody(Entity* body)
{
    if (body != m_selectedBody.ptr())
    {
        m_selectedBody = body;
    }
}


void
UniverseView::setSelectedBody(const QString& name)
{
    Entity* body = NULL;

    QStringList allNames = m_catalog->names();
    foreach (QString s, allNames)
    {
        if (name.compare(s, Qt::CaseInsensitive) == 0)
        {
            body = m_catalog->find(s);
            break;
        }
    }

    if (body)
    {
        setSelectedBody(body);
    }
}


void
UniverseView::setFOV(double fovY)
{
    m_fovY = max(MinimumFOV, min(MaximumFOV, fovY));

    double fovX = atan(tan(m_fovY / 2.0) * double(width()) / double(height())) * 2.0;
    setStatusMessage(QString("%1\260 x %2\260 field of view").arg(toDegrees(fovX), 0, 'f', 1).arg(toDegrees(m_fovY), 0, 'f', 1));
}


void
UniverseView::findObject()
{
    // Show the QML find object widget
    // This function will be unnecessary once more of the UI moves into QML
    QVariant returnedValue;
    QMetaObject::invokeMethod(rootObject(), "showFindObject", Q_RETURN_ARG(QVariant, returnedValue));
}


bool
UniverseView::isGalleryVisible() const
{
    return m_galleryView && m_galleryView->isVisible();
}


void
UniverseView::setGalleryVisible(bool visible)
{
    if (m_galleryView)
    {
        m_galleryView->setVisible(visible);
    }
}

void
UniverseView::toggleGallery()
{
    setGalleryVisible(!isGalleryVisible());

#if 0
    // Debugging code to show position of center object in body fixed frame of selection
    Entity* center = m_observer->center();
    Entity* selection = m_selectedBody.ptr();

    if (center && selection)
    {
        counted_ptr<BodyFixedFrame> frame(new BodyFixedFrame(selection));
        Vector3d centerPos = center->position(m_simulationTime);
        Vector3d selectionPos = selection->position(m_simulationTime);
        Vector3d v = frame->orientation(m_simulationTime).conjugate() * (centerPos - selectionPos);
        GregorianDate now = GregorianDate::TDBDateFromTDBSec(m_simulationTime);
        QDateTime qnow = VestaDateToQtDate(now);
        qDebug() << qnow.toString("yyyy-mm-dd hh:mm:ss.zzz") << QString("%1 %2 %3").arg(v.x(), 0, 'g', 16).arg(v.y(), 0, 'g', 16).arg(v.z(), 0, 'g', 16);
    }
#endif
}


void
UniverseView::setTimeDisplay(TimeDisplayMode mode)
{
    m_timeDisplay = mode;
}


void
UniverseView::replaceEntity(Entity* entity, const BodyInfo* info)
{
    Entity* existingBody = m_universe->findFirst(entity->name());
    if (existingBody)
    {
        m_universe->removeEntity(existingBody);
    }

    m_universe->addEntity(entity);

    QString labelText = bodyName(entity);
    int slashPos = labelText.lastIndexOf('/');
    if (slashPos >= 0)
    {
        labelText = labelText.right(labelText.length() - slashPos - 1);
    }

    Spectrum color = Spectrum::White();
    double fadeSize = 0.0;
    if (info)
    {
        color = info->labelColor;
        fadeSize = info->labelFadeSize;
        if (!info->labelTextVisible)
        {
            labelText = "";
        }
    }
    labelBody(entity, info, labelText, m_labelFont.ptr(), m_spacecraftIcon.ptr(), color, fadeSize, m_labelsVisible);

    // Special handling for the Sun
    if (entity->name() == "Sun")
    {
        LightSource* sunlight = new LightSource();
        sunlight->setLightType(LightSource::Sun);
        sunlight->setShadowCaster(true);
        sunlight->setSpectrum(Spectrum::White());
        sunlight->setGlareTexture(m_textureLoader->loadTexture("textures/flare.png", TextureProperties(TextureProperties::Clamp)));
        entity->setLightSource(sunlight);
    }
}


void
UniverseView::gotoSelectedObject()
{
    if (m_selectedBody.isValid())
    {
        Entity* body = m_selectedBody.ptr();
        double distanceFromTarget = 500.0;
        if (body->geometry())
        {
            distanceFromTarget = body->geometry()->boundingSphereRadius() * 3.0;
        }

        double currentDistance = (body->position(m_simulationTime) - m_observer->absolutePosition(m_simulationTime)).norm();
        if (currentDistance > distanceFromTarget * 1.1)
        {
            setStatusMessage(QString("Go to %1").arg(bodyName(body)));

            m_observerAction = new GotoObserverAction(m_observer.ptr(),
                                                      body,
                                                      m_gotoObjectTime,
                                                      secondsFromBaseTime(),
                                                      m_simulationTime,
                                                      distanceFromTarget);
        }
    }
}


void
UniverseView::centerSelectedObject()
{
    if (m_selectedBody.isValid())
    {
        setStatusMessage(QString("Center %1 in view").arg(bodyName(m_selectedBody.ptr())));

        m_observerAction = new CenterObserverAction(m_observer.ptr(),
                                                    m_selectedBody.ptr(),
                                                    1.0,
                                                    secondsFromBaseTime(),
                                                    m_simulationTime);
    }
}


void
UniverseView::gotoHome()
{
    m_observerAction = new OrbitGotoObserverAction(m_observer.ptr(),
                                                   m_catalog->find("Sun"),
                                                   7.0,
                                                   secondsFromBaseTime(),
                                                   m_simulationTime,
                                                   1.0e9);
}


void
UniverseView::setViewpoint(Viewpoint *viewpoint)
{
    viewpoint->positionObserver(m_observer.ptr(), m_simulationTime);
    m_selectedBody = viewpoint->centerBody();
}


void
UniverseView::setUpdateInterval(unsigned int msec)
{
    m_timer->setInterval(msec);
}


UniverseView::TrajectoryPlotEntry::TrajectoryPlotEntry() :
    generator(NULL),
    sampleCount(100),
    leadDuration(0.0)
{
}


// Wrapper functions
BodyObject*
UniverseView::getSelectedBody() const
{
    BodyObject* o = new BodyObject(m_selectedBody.ptr());
    QDeclarativeEngine::setObjectOwnership(o, QDeclarativeEngine::JavaScriptOwnership);
    return o;
}


void
UniverseView::setSelectedBody(BodyObject* body)
{
    if (body && body->body())
    {
        this->m_selectedBody = body->body();
    }
    else
    {
        this->m_selectedBody = NULL;
    }
}


BodyObject*
UniverseView::getCentralBody() const
{
    BodyObject* o = new BodyObject(m_observer->center());
    QDeclarativeEngine::setObjectOwnership(o, QDeclarativeEngine::JavaScriptOwnership);
    return o;
}


void
UniverseView::setCentralBody(BodyObject* body)
{
    if (body && body->body())
    {
        setCenterAndFrame(body->body(), Frame_Inertial);
        setStatusMessage(QString("Set center to %1").arg(bodyName(body->body())));
    }
}


void
UniverseView::setCentralBodyFixed(BodyObject* body)
{
    if (body && body->body())
    {
        setCenterAndFrame(body->body(), Frame_BodyFixed);
        setStatusMessage(QString("Set fixed center to %1").arg(bodyName(body->body())));
    }
}


void
UniverseView::trackBody(BodyObject* body)
{
    if (body && body->body())
    {
        if (m_observer->center() != body->body())
        {
            setSelectedBody(body->body());
            setCenterAndFrame(m_observer->center(), Frame_Locked);
            setStatusMessage(QString("Tracking %1").arg(bodyName(body->body())));
        }
    }
}


void
UniverseView::trackBodyLevel(BodyObject* body)
{
    if (body && body->body())
    {
        if (m_observer->center() != body->body())
        {
            Vector3d up = m_observer->absoluteOrientation(m_simulationTime) * Vector3d::UnitY();
            setSelectedBody(body->body());
            setCenterAndFrame(m_observer->center(), Frame_LockedLevel, up);
            setStatusMessage(QString("Tracking %1").arg(bodyName(body->body())));
        }
    }
}


VisualizerObject*
UniverseView::createBodyDirectionVisualizer(BodyObject* from, BodyObject* target)
{
    if (from == NULL   || from->body() == NULL ||
        target == NULL || target->body() == NULL)
    {
        return NULL;
    }

    float visualizerSize = 1.0f;
    if (from->body()->geometry())
    {
        visualizerSize = from->body()->geometry()->boundingSphereRadius() * 2.0f;
    }

    ArrowVisualizer* arrow = new BodyDirectionVisualizer(visualizerSize, target->body());
    arrow->setVisibility(true);
    arrow->setColor(Spectrum(1.0f, 1.0f, 0.7f));
    arrow->setLabelEnabled(true);
    arrow->setLabelText(target->body()->name());

    VisualizerObject* o = new VisualizerObject(arrow);
    QDeclarativeEngine::setObjectOwnership(o, QDeclarativeEngine::JavaScriptOwnership);
    return o;
}


/** Construct a URL from the current observer state, time, and time rate.
  *
  * A Cosmographia URL has the scheme cosmo and a path equal to the current
  * center object. The stored position and orientation are relative to the center object
  * in the observer frame.
  *
  * The time scale and fov are optional fields.
  */
QUrl
UniverseView::getStateUrl()
{
    QUrl url;

    url.setScheme("cosmo");
    url.setPath(bodyName(m_observer->center()));

    QUrlQuery query;

    double jd = secondsToDays(m_simulationTime) + vesta::J2000;
    Vector3d position = m_observer->position();
    Quaterniond orientation = m_observer->orientation();
    double ts = isPaused() ? 0.0 : timeScale();

    QString frame = "icrf";
    if (m_observerFrame == Frame_BodyFixed)
    {
        frame = "bfix";
    }
    else if (m_observerFrame == Frame_Locked)
    {
        TwoVectorFrame* f = dynamic_cast<TwoVectorFrame*>(m_observer->positionFrame());
        if (f)
        {
            RelativePositionVector* vec = dynamic_cast<RelativePositionVector*>(f->primaryDirection());
            if (vec)
            {
                frame = "track";
                query.addQueryItem("ftarget", bodyName(vec->target()));
            }
        }
    }
    else if (m_observerFrame == Frame_LockedLevel)
    {
        TwoVectorFrame* f = dynamic_cast<TwoVectorFrame*>(m_observer->positionFrame());
        if (f)
        {
            RelativePositionVector* vec = dynamic_cast<RelativePositionVector*>(f->primaryDirection());
            if (vec)
            {
                frame = "tracklev";
                query.addQueryItem("ftarget", bodyName(vec->target()));
            }

            ConstantFrameDirection* dir = dynamic_cast<ConstantFrameDirection*>(f->secondaryDirection());
            if (dir)
            {
                query.addQueryItem("upx", QString::number(dir->vector().x(), 'f'));
                query.addQueryItem("upy", QString::number(dir->vector().y(), 'f'));
                query.addQueryItem("upz", QString::number(dir->vector().z(), 'f'));
            }
        }
    }

    if (selectedBody())
    {
        query.addQueryItem("select", bodyName(selectedBody()));
    }

    query.addQueryItem("frame", frame);
    query.addQueryItem("jd", QString::number(jd, 'f'));
    query.addQueryItem("x", QString::number(position.x(), 'f'));
    query.addQueryItem("y", QString::number(position.y(), 'f'));
    query.addQueryItem("z", QString::number(position.z(), 'f'));
    query.addQueryItem("qw", QString::number(orientation.w(), 'f'));
    query.addQueryItem("qx", QString::number(orientation.x(), 'f'));
    query.addQueryItem("qy", QString::number(orientation.y(), 'f'));
    query.addQueryItem("qz", QString::number(orientation.z(), 'f'));
    query.addQueryItem("ts", QString::number(ts));
    query.addQueryItem("fov", QString::number(toDegrees(m_fovY)));

    url.setQuery(query);

    return url;
}


void
UniverseView::setStateFromUrl(const QUrl& url)
{
    if (url.scheme() != "cosmo")
    {
        qDebug() << "Not a Cosmographia URL";
        return;
    }

    QString centerBodyName = url.path();
    if (centerBodyName.isEmpty())
    {
        qDebug() << "Central body is missing from URL";
        return;
    }

    Entity* centerBody = m_catalog->find(centerBodyName);
    if (!centerBody)
    {
        qDebug() << "Central body " << centerBodyName << " from URL not found in catalog";
        return;
    }

    double jd = 0.0;
    Vector3d position = Vector3d::Zero();
    Quaterniond orientation = Quaterniond::Identity();

    QUrlQuery query(url.query());
    position.x() = query.queryItemValue("x").toDouble();
    position.y() = query.queryItemValue("y").toDouble();
    position.z() = query.queryItemValue("z").toDouble();

    orientation.w() = query.queryItemValue("qw").toDouble();
    orientation.x() = query.queryItemValue("qx").toDouble();
    orientation.y() = query.queryItemValue("qy").toDouble();
    orientation.z() = query.queryItemValue("qz").toDouble();

    jd = query.queryItemValue("jd").toDouble();

    if (orientation.coeffs().isZero())
    {
        qDebug() << "Bad or missing orientation in URL";
        return;
    }

    // Ensure that we have a unit quaternion so that the view transform doesn't
    // get messed up.
    orientation.normalize();

    double tdbSec = daysToSeconds(jd - vesta::J2000);
    if (tdbSec < StartOfTime || tdbSec > EndOfTime)
    {
        qDebug() << "URL time is outside the allowed range";
        return;
    }

    // Get the frame from the URL
    QString frame = query.queryItemValue("frame");
    FrameType newFrame = m_observerFrame;
    if (frame == "icrf")
    {
        newFrame = Frame_Inertial;
    }
    else if (frame == "bfix")
    {
        newFrame = Frame_BodyFixed;
    }
    else if (frame == "track")
    {
        newFrame = Frame_Locked;
        QString targetName = query.queryItemValue("ftarget");
        setSelectedBody(m_catalog->find(targetName));
    }
    else if (frame == "tracklev")
    {
        newFrame = Frame_LockedLevel;
        QString targetName = query.queryItemValue("ftarget");
        setSelectedBody(m_catalog->find(targetName));
    }

    // Now actually set the state
    setSimulationTime(tdbSec);
    setCenterAndFrame(centerBody, newFrame);
    setSelectedBody(centerBody);
    m_observer->setPosition(position);
    m_observer->setOrientation(orientation);

    // Use the selection stored in the URL, if present
    QString selectionName = query.queryItemValue("select");
    if (!selectionName.isEmpty())
    {
        Entity* selection = m_catalog->find(selectionName);
        if (selection)
        {
            setSelectedBody(selection);
        }
    }

    bool ok = false;
    double fov = toRadians(query.queryItemValue("fov").toDouble(&ok));
    if (ok)
    {
        // Only set the field of view we have a valid value
        if (fov >= MinimumFOV && fov <= MaximumFOV)
        {
            setFOV(fov);
        }
    }

    double timeScale = query.queryItemValue("ts").toDouble(&ok);
    if (ok)
    {
        if (timeScale == 0.0)
        {
            setPaused(true);
        }
        else if (abs(timeScale) >= 0.01 && abs(timeScale) <= 1.0e8)
        {
            setPaused(false);
            setTimeScale(timeScale);
        }
    }

    int clouds = query.queryItemValue("clouds").toInt(&ok);
    if (ok)
    {
        setCloudsVisible(clouds != 0);
    }

    int atmospheres = query.queryItemValue("atm").toInt(&ok);
    if (ok)
    {
        setAtmospheresVisible(atmospheres != 0);
    }
}


/** Save the next rendered frame to the clipboard, optionally with
  * alpha information. We wait for the next frame in order to capture
  * the image before the UI overlay is rendered on top of it.
  */
void
UniverseView::copyNextFrameToClipboard(bool /* withAlpha */)
{
    m_captureNextImage = true;
    setStatusMessage(tr("Captured screenshot to clipboard"));
}

