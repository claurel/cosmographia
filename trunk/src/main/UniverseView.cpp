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

#include <cmath>

#include <vesta/OGLHeaders.h>
#include "UniverseView.h"
#include "MarkerLayer.h"

#include "ObserverAction.h"
#include "Viewpoint.h"
#include "InterpolatedStateTrajectory.h"
#include "DateUtility.h"
#include "SkyLabelLayer.h"
#include "ConstellationInfo.h"
#include "TwoVectorFrame.h"

#if FFMPEG_SUPPORT
#include "QVideoEncoder.h"
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

#include <vesta/interaction/ObserverController.h>

#include <vesta/CubeMapFramebuffer.h>

#include <vesta/ParticleSystemGeometry.h>
#include <vesta/particlesys/ParticleEmitter.h>
#include <vesta/particlesys/PointGenerator.h>
#include <vesta/particlesys/DiscGenerator.h>

#include <Eigen/LU>

#include <QtGui>
#include <QtOpenGL>
#include <QFile>
#include <QDataStream>

#include <QUrl>
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


static const double KeyboardRotationAcceleration = 3.5;

static const unsigned int ShadowMapSize = 2048;
static const unsigned int ReflectionMapSize = 512;

static double StartOfTime = GregorianDate(1900, 1, 1, 0, 0, 0, 0, TimeScale_TDB).toTDBSec();
static double EndOfTime   = GregorianDate(2100, 1, 1, 0, 0, 0, 0, TimeScale_TDB).toTDBSec();

static const double MinimumFOV = toRadians(0.1);
static const double MaximumFOV = toRadians(90.0);

static const double StatusMessageDuration = 2.5;

static const float CenterMarkerSize = 10.0f;

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


class UniverseGLWidget : public QGLWidget
{
    //Q_OBJECT

public:
    UniverseGLWidget(QWidget* parent, UniverseRenderer* renderer) :
        QGLWidget(parent),
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
    m_mouseEventProcessed(false),
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
    m_sunGlareEnabled(true),
    m_infoTextVisible(true),
    m_labelsVisible(true),
    m_centerIndicatorVisible(true),
    m_gotoObjectTime(6.0),
    m_videoEncoder(NULL),
    m_timeDisplay(TimeDisplay_UTC),
    m_wireframe(false),
    m_statusUpdateTime(0.0),
    m_markers(NULL)
{
    setAutoFillBackground(false);

    m_universe = universe;
    m_textureLoader = new NetworkTextureLoader(this);
    m_renderer = new UniverseRenderer();
    m_renderer->setDefaultSunEnabled(false);

    m_labelFont = new TextureFont();
    m_textFont = new TextureFont();
    m_titleFont = new TextureFont();
    m_spacecraftIcon = m_textureLoader->loadTexture(":/icons/disk.png", TextureProperties(TextureProperties::Clamp));

    m_markers = new MarkerLayer();

    UniverseGLWidget* glWidget = new UniverseGLWidget(this, m_renderer);
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

    // Initialize settings
    setLimitingMagnitude(8.0);
    emit limitingMagnitudeChanged(limitingMagnitude()); // Force a notification
}


UniverseView::~UniverseView()
{
    //makeCurrent();
    delete m_renderer;
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



static Visualizer* labelBody(Entity* planet, const QString& labelText, TextureFont* font, TextureMap* icon, const Spectrum& color, double fadeSize, bool visible)
{
    if (!planet || !planet->isVisible())
    {
        return NULL;
    }

    LabelVisualizer* vis = new LabelVisualizer(labelText.toUtf8().data(), font, color, 6.0f);
    vis->label()->setIcon(icon);
    vis->label()->setIconColor(color);

    // Set up the labels to fade when the labeled object is very close or very distant
    float geometrySize = 1.0f;
    if (planet->geometry())
    {
        geometrySize = planet->geometry()->boundingSphereRadius();
    }

    float orbitSize = planet->chronology()->firstArc()->trajectory()->boundingSphereRadius();
    if (fadeSize == 0.0)
    {
        fadeSize = orbitSize;
    }
    float minPixels = 20.0f;
    float maxPixels = 20.0f * fadeSize / geometrySize;

    if (planet->name() != "Sun")
    {
        vis->label()->setFadeSize(fadeSize);
        vis->label()->setFadeRange(new FadeRange(minPixels, maxPixels, minPixels, maxPixels));
    }

    planet->setVisualizer("label", vis);
    vis->setVisibility(visible);
    vis->setDepthAdjustment(Visualizer::AdjustToFront);

    return vis;
}


void UniverseView::initializeGL()
{
    // Initialize the renderer. This must be done *after* an OpenGL context
    // has been created, otherwise information about OpenGL capabilities is
    // not available.
    if (!m_renderer->initializeGraphics())
    {
        qCritical("Creating renderer failed because OpenGL couldn't be initialized.");
    }

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
    }

    if (m_renderer->shadowsSupported())
    {
        m_renderer->initializeShadowMaps(ShadowMapSize, 1);
    }

    if (m_renderer->omniShadowsSupported())
    {
        m_renderer->initializeOmniShadowMaps(1024, 1);
    }

    setAmbientLight(false);

    if (CubeMapFramebuffer::supported())
    {
        m_reflectionMap = CubeMapFramebuffer::CreateCubicReflectionMap(ReflectionMapSize, TextureMap::R8G8B8A8);
    }

    m_glareOverlay = m_renderer->createGlareOverlay();
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


void UniverseView::paintEvent(QPaintEvent* /* event */)
{
    // Make the viewport's GL context current
    dynamic_cast<QGLWidget*>(viewport())->makeCurrent();

    QPainter painter(viewport());

    // Save the state of the painter
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // Render the universe using VESTA
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

    if (GLEW_VERSION_1_5)
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (GLEW_VERSION_2_0)
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
QString readableNumber(double value, int significantDigits)
{
    double roundValue = value;
    int useDigits = 1;

    if (value != 0.0)
    {
        double n = log10(abs(value));
        useDigits = max(0, significantDigits - (int) n - 1);
        double m = pow(10.0, floor(n) - significantDigits + 1);
        roundValue = floor(value / m + 0.5) * m;
    }
    else
    {
        useDigits = significantDigits;
    }

    return QLocale::system().toString(roundValue, 'f', useDigits);
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


/*
// Only required for debugging
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
*/


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


// Called by QML to report when a mouse event is missed and should instead
// be processed by the C++ event handler.
void
UniverseView::setMouseEventProcessed(bool accepted)
{
    m_mouseEventProcessed = accepted;
}


// Draw informational text over the 3D view
// TODO: Convert this to QML
void
UniverseView::drawInfoOverlay()
{
    int viewportWidth = size().width();
    int viewportHeight = size().height();
    glViewport(0, 0, viewportWidth, viewportHeight);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, viewportWidth, 0, viewportHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.125f, 0.125f, 0);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);

    float alpha = max(0.0f, min(1.0f, float((m_realTime - 5.0) / 5.0)));
    Vector4f textColor(0.3f, 0.5f, 1.0f, alpha);
    Vector4f titleColor(0.45f, 0.75f, 1.0f, alpha);

    glColor4fv(textColor.data());

    QLocale locale = QLocale::system();

    if (m_infoTextVisible)
    {
        if (m_textFont.isValid())
        {
            const int titleFontHeight = 30;
            const int textFontHeight = 20;

            // Show the current simulation time
            GregorianDate date = GregorianDate::UTCDateFromTDBSec(m_simulationTime);

            m_textFont->bind();


            // Display of date and time rate now handled by QML
#if 0
            float dateY = float(viewportHeight - titleFontHeight);
            float dateX = viewportWidth - 250.0f;

            if (m_timeDisplay == TimeDisplay_UTC)
            {
                m_textFont->render(formatDate(date).toUtf8().data(), Vector2f(dateX, dateY));
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
            if (m_paused)
            {
                QString timeScaleString = QString("%1x time (paused)").arg(m_timeScale);
                m_textFont->render(timeScaleString.toLatin1().data(), Vector2f(dateX, dateY));
            }
            else
            {
                QString timeScaleString = QString("%1x time").arg(m_timeScale);
                m_textFont->render(timeScaleString.toLatin1().data(), Vector2f(dateX, dateY));
            }
#endif

            //QString frameCountString = QString("%1 fps").arg(m_framesPerSecond);
            //m_textFont->render(frameCountString.toLatin1().data(), Vector2f(viewportWidth - 200.0f, 10.0f));

            // Display information about the selection
            if (m_selectedBody.isValid())
            {
                m_titleFont->bind();
                glColor4fv(titleColor.data());
                m_titleFont->render(m_selectedBody->name(), Vector2f(10.0f, float(viewportHeight - titleFontHeight)));
                glColor4fv(textColor.data());
                m_textFont->bind();

                Vector3d r = m_observer->absolutePosition(m_simulationTime) - m_selectedBody->position(m_simulationTime);
                double distance = r.norm();

                bool isEllipsoidal = m_selectedBody->geometry() && m_selectedBody->geometry()->isEllipsoidal();
                if (isEllipsoidal)
                {
                    distance -= m_selectedBody->geometry()->ellipsoid().semiMajorAxisLength();
                }

                QString distanceString = QString("Distance: %1 km").arg(readableNumber(distance, 6));;
                m_textFont->render(distanceString.toLatin1().data(), Vector2f(10.0f, viewportHeight - 20.0f - titleFontHeight));

                // Display the subpoint for ellipsoidal bodies that are sufficiently close
                // to the observer.
                if (isEllipsoidal && distance < m_selectedBody->geometry()->ellipsoid().semiMajorAxisLength() * 100)
                {
                    Vector3d q = m_selectedBody->orientation(m_simulationTime).conjugate() * r;
                    q = q.normalized();
                    double latitude = toDegrees(asin(q.z()));
                    double longitude = toDegrees(atan2(q.y(), q.x()));
                    QString coordString = QString("Subpoint: %1, %2").arg(latitude, 0, 'f', 3).arg(longitude, 0, 'f', 3);
                    m_textFont->render(coordString.toLatin1().data(), Vector2f(10.0f, viewportHeight - 20.0f - (titleFontHeight + textFontHeight)));
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
                    m_textFont->render(tileCountString.toLatin1().data(), Vector2f(10.0f, viewportHeight - 20.0f - (titleFontHeight + textFontHeight * 2)));
                }
            }

#if 0
            {
                double fovX = atan(tan(m_fovY / 2.0) * double(width()) / double(height())) * 2.0;
                QString fovInfo = QString("%1\260 x %2\260 FOV").arg(toDegrees(fovX), 0, 'f', 1).arg(toDegrees(m_fovY), 0, 'f', 1);
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
        Viewport viewport(size().width(), size().height());
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
            centerMarker.setSize(CenterMarkerSize);
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

    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
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

    Viewport mainViewport(size().width(), size().height());
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
        double eyeSeparation = focusObjectDistance / 30.0;
        float screenPlaneDistance = float(focusObjectDistance * 0.85);
        float nearDistance = 0.00001f;
        float farDistance = 1.0e12f;
        float y = tan(0.5f * m_fovY) * nearDistance;
        float x = y * mainViewport.aspectRatio();

        float frustumOffset = float(eyeSeparation) * nearDistance / screenPlaneDistance;

        Vector3d leftEyePosition = cameraPosition + cameraOrientation * (Vector3d::UnitX() * -eyeSeparation);
        Vector3d rightEyePosition = cameraPosition + cameraOrientation * (Vector3d::UnitX() * eyeSeparation);

        if (m_stereoMode == SideBySide)
        {
            x *= 0.5f;
            PlanarProjection leftProjection(PlanarProjection::Perspective,  -x + frustumOffset, x + frustumOffset, -y, y, nearDistance, farDistance);
            PlanarProjection rightProjection(PlanarProjection::Perspective, -x - frustumOffset, x - frustumOffset, -y, y, nearDistance, farDistance);

            Viewport leftViewport(0, 0, width() / 2, height());
            Viewport rightViewport(width() / 2, 0, width() / 2, height());
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

    if (m_wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    m_renderer->endViewSet();

#if FFMPEG_SUPPORT
    if (m_videoEncoder)
    {
        QImage image = grabFrameBuffer(false);
        image = image.scaled(QSize(m_videoEncoder->getWidth(), m_videoEncoder->getHeight()), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_videoEncoder->encodeImage(image);
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
    setMouseEventProcessed(true);
    QDeclarativeView::mouseReleaseEvent(event);
    if (m_mouseEventProcessed)
    {
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
                m_selectedBody = pickObject(event->pos());
                m_markers->addMarker(m_selectedBody.ptr(), Spectrum(1.0f, 1.0f, 1.0f), 20.0f, Marker::Pulse, m_realTime, 0.5);
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
    setMouseEventProcessed(true);
    QDeclarativeView::mouseMoveEvent(event);
    if (m_mouseEventProcessed)
    {
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
    if (scene()->focusItem())
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
    if (scene()->focusItem())
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
        QAction* nameAction = menu->addAction(QString::fromUtf8(body->name().c_str()));
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
                clearTrajectory(body);
            }
        }
    }
}


// Find the in the view underneat the specified point that is closest to the
// camera.
Entity*
UniverseView::pickObject(const QPoint& point)
{
    // Get the click point in normalized device coordinaes
    Vector2d ndc = Vector2d(double(point.x()) / double(size().width()),
                            double(point.y()) / double(size().height())) * 2.0 - Vector2d::Ones();
    ndc.y() = -ndc.y();

    double pixelAngle = m_fovY / size().height();

    // Convert to a direction in view coordinates
    double aspectRatio = double(size().width()) / double(size().height());
    double h = tan(m_fovY / 2.0f);
    Vector3d pickDirection = Vector3d(h * aspectRatio * ndc.x(), h * ndc.y(), -1.0).normalized();

    // Convert to world coordinates
    pickDirection = m_observer->absoluteOrientation(m_simulationTime) * pickDirection;
    Vector3d pickOrigin = m_observer->absolutePosition(m_simulationTime);

    PickResult pickResult;
    if (m_universe->pickObject(m_simulationTime, pickOrigin, pickDirection, pixelAngle, &pickResult))
    {
        return pickResult.hitObject();
    }
    else
    {
        return NULL;
    }
}


void
UniverseView::updateTrajectoryPlots()
{
    for (vector<TrajectoryPlotEntry>::const_iterator iter = m_trajectoryPlots.begin();
         iter != m_trajectoryPlots.end(); ++iter)
    {
        Visualizer* vis = iter->visualizer.ptr();
        TrajectoryGeometry* plot = dynamic_cast<TrajectoryGeometry*>(vis->geometry());

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

            plot->updateSamples(iter->trajectory.ptr(), startTime, endTime, iter->sampleCount);
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

    SkyImageLayer* milkyWayLayer = new SkyImageLayer();
    milkyWayLayer->setOpacity(0.3f);
    milkyWayLayer->setDrawOrder(-1);
    milkyWayLayer->setTexture(m_textureLoader->loadTexture("textures/milkyway.jpg", SkyLayerTextureProperties()));
    milkyWayLayer->setOrientation(InertialFrame::galactic()->orientation());
    m_universe->setLayer("milky way", milkyWayLayer);
    milkyWayLayer->setVisibility(false);

    ConstellationsLayer* constellations = new ConstellationsLayer(m_universe->starCatalog());
    constellations->setDiagramColor(Spectrum(0.0f, 0.2f, 0.5f));
    constellations->setDefaultConstellations();
    constellations->setVisibility(false);
    m_universe->setLayer("constellation figures", constellations);

    SkyLabelLayer* constellationNamesLayer = new SkyLabelLayer();
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

#if FFMPEG_SUPPORT
    if (m_videoEncoder)
    {
        // Lock time step when recording video
        dt = 1.0 / 30.0;
    }
#endif

    m_realTime += dt;

    if (!isPaused())
    {
        setSimulationTime(m_simulationTime + dt * timeScale());
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

    m_controller->tick(dt);

    if (m_observerAction.isValid())
    {
        bool complete = m_observerAction->updateObserver(m_observer.ptr(), t, m_simulationTime);
        if (complete)
        {
            m_observerAction = NULL;
        }
    }

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
UniverseView::setCenterAndFrame(Entity* center, FrameType f)
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
            ConstantFrameDirection* secondary = new ConstantFrameDirection(InertialFrame::icrf(), Vector3d::UnitZ());
            frame = new TwoVectorFrame(primary, TwoVectorFrame::PositiveX, secondary, TwoVectorFrame::PositiveY);
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
        setCenterAndFrame(m_selectedBody.ptr(), m_observerFrame);
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
    m_simulationTime = tsec;
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
                            dateTime.time().hour(), dateTime.time().minute(), dateTime.time().second());
    setSimulationTime(startDate.toTDBSec());
}


void
UniverseView::bodyFixedObserver(bool checked)
{
    if (checked)
    {
        setCenterAndFrame(m_observer->center(), Frame_BodyFixed);
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
    if (!arc)
    {
        return;
    }

    string visName = TrajectoryVisualizerName(body);
    Visualizer* oldVisualizer = arc->center()->visualizer(visName);

    TrajectoryGeometry* plot = new TrajectoryGeometry();
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
        duration = arc->trajectory()->period();
    }

    plot->setWindowDuration(duration);
    plot->setWindowLead(lead);
    plot->setDisplayedPortion(TrajectoryGeometry::WindowBeforeCurrentTime);
    plot->setFadeFraction(fade);
    plot->setColor(color);
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


void
UniverseView::clearTrajectory(Entity* body)
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
            plot->setWindowDuration(daysToSeconds(3.0));
            plot->setDisplayedPortion(TrajectoryGeometry::WindowBeforeCurrentTime);
            plot->setFadeFraction(0.5);
            plot->setColor(color);
            center->setVisualizer(visName, visualizer);

            TrajectoryPlotEntry plotEntry;
            plotEntry.trajectory = NULL;
            plotEntry.visualizer = visualizer;
            plotEntry.generator = new BodyPositionSampleGenerator(m_selectedBody.ptr(), center, frame);
            plotEntry.sampleCount = 300;
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
    m_renderer->setShadowsEnabled(enable);
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


void
UniverseView::setStereoMode(StereoMode stereoMode)
{
    m_stereoMode = stereoMode;
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
}


void
UniverseView::finishVideoRecording()
{
    m_videoEncoder = NULL;
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
    setStatusMessage(QString("%1\260 x %2\260 FOV").arg(toDegrees(fovX), 0, 'f', 1).arg(toDegrees(m_fovY), 0, 'f', 1));
}


void
UniverseView::findObject()
{
    // Show the QML find object widget
    // This function will be unnecessary once more of the UI moves into QML
    QVariant returnedValue;
    QMetaObject::invokeMethod(rootObject(), "showFindObject", Q_RETURN_ARG(QVariant, returnedValue));
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

    QString labelText = QString::fromUtf8(entity->name().c_str());
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
    }
    labelBody(entity, labelText, m_labelFont.ptr(), m_spacecraftIcon.ptr(), color, fadeSize, m_labelsVisible);

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
            setStatusMessage(QString("Go to %1").arg(QString::fromUtf8(body->name().c_str())));

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
        setStatusMessage(QString("Center %1 in view").arg(QString::fromUtf8(m_selectedBody->name().c_str())));

        m_observerAction = new CenterObserverAction(m_observer.ptr(),
                                                    m_selectedBody.ptr(),
                                                    1.0,
                                                    secondsFromBaseTime(),
                                                    m_simulationTime);
    }
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
        setCenterAndFrame(body->body(), m_observerFrame);
    }
}


BodyObject*
UniverseView::getEarth() const
{
    BodyObject* o = new BodyObject(m_universe->findFirst("Earth"));
    QDeclarativeEngine::setObjectOwnership(o, QDeclarativeEngine::JavaScriptOwnership);
    return o;
}


BodyObject*
UniverseView::getSun() const
{
    BodyObject* o = new BodyObject(m_universe->findFirst("Sun"));
    QDeclarativeEngine::setObjectOwnership(o, QDeclarativeEngine::JavaScriptOwnership);
    return o;
}


BodyObject*
UniverseView::lookupBody(const QString& name) const
{
    Entity* body = m_catalog->find(name, Qt::CaseInsensitive);
    if (body)
    {
        BodyObject* o = new BodyObject(body);
        QDeclarativeEngine::setObjectOwnership(o, QDeclarativeEngine::JavaScriptOwnership);
        return o;
    }
    else
    {
        return NULL;
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
    url.setPath(QString::fromUtf8(m_observer->center()->name().c_str()));

    double jd = secondsToDays(m_simulationTime) + vesta::J2000;
    Vector3d position = m_observer->position();
    Quaterniond orientation = m_observer->orientation();
    double ts = isPaused() ? 0.0 : timeScale();

    url.addQueryItem("frame", "icrf");
    url.addQueryItem("jd", QString::number(jd, 'f'));
    url.addQueryItem("x", QString::number(position.x(), 'f'));
    url.addQueryItem("y", QString::number(position.y(), 'f'));
    url.addQueryItem("z", QString::number(position.z(), 'f'));
    url.addQueryItem("qw", QString::number(orientation.w(), 'f'));
    url.addQueryItem("qx", QString::number(orientation.x(), 'f'));
    url.addQueryItem("qy", QString::number(orientation.y(), 'f'));
    url.addQueryItem("qz", QString::number(orientation.z(), 'f'));
    url.addQueryItem("ts", QString::number(ts));
    url.addQueryItem("fov", QString::number(toDegrees(m_fovY)));

    qDebug() << "URL: " << url.toString();

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

    position.x() = url.queryItemValue("x").toDouble();
    position.y() = url.queryItemValue("y").toDouble();
    position.z() = url.queryItemValue("z").toDouble();

    orientation.w() = url.queryItemValue("qw").toDouble();
    orientation.x() = url.queryItemValue("qx").toDouble();
    orientation.y() = url.queryItemValue("qy").toDouble();
    orientation.z() = url.queryItemValue("qz").toDouble();

    jd = url.queryItemValue("jd").toDouble();

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

    // Now actually set the state
    setSimulationTime(tdbSec);
    setCenterAndFrame(centerBody, m_observerFrame);
    m_observer->setPosition(position);
    m_observer->setOrientation(orientation);

    bool ok = false;
    double fov = toRadians(url.queryItemValue("fov").toDouble(&ok));
    if (ok)
    {
        // Only set the field of view we have a valid value
        if (fov >= MinimumFOV && fov <= MaximumFOV)
        {
            setFOV(fov);
        }
    }

    double timeScale = url.queryItemValue("ts").toDouble(&ok);
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
}
