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

#include <cmath>

#include <vesta/OGLHeaders.h>
#include "UniverseView.h"
#include "NetworkTextureLoader.h"
#include "WMSRequester.h"
#include "TleTrajectory.h"
#include "ChebyshevPolyTrajectory.h"
#include "JPLEphemeris.h"
#include "KeplerianSwarm.h"

#include "QVideoEncoder.h"

#include <vesta/Chronology.h>
#include <vesta/Arc.h>
#include <vesta/Body.h>
#include <vesta/Units.h>
#include <vesta/FixedPointTrajectory.h>
#include <vesta/FixedRotationModel.h>
#include <vesta/KeplerianTrajectory.h>
#include <vesta/MeshGeometry.h>
#include <vesta/UniformRotationModel.h>
#include <vesta/Universe.h>
#include <vesta/UniverseRenderer.h>
#include <vesta/WorldGeometry.h>
#include <vesta/PlanetaryRings.h>
#include <vesta/TextureMapLoader.h>
#include <vesta/InertialFrame.h>
#include <vesta/BodyFixedFrame.h>
#include <vesta/TwoBodyRotatingFrame.h>
#include <vesta/CelestialCoordinateGrid.h>
#include <vesta/StarsLayer.h>
#include <vesta/SkyImageLayer.h>
#include <vesta/ConstellationsLayer.h>
#include <vesta/AxesVisualizer.h>
#include <vesta/PlaneVisualizer.h>
#include <vesta/VelocityVisualizer.h>
#include <vesta/NadirVisualizer.h>
#include <vesta/SensorVisualizer.h>
#include <vesta/LabelGeometry.h>
#include <vesta/BillboardGeometry.h>
#include <vesta/TrajectoryGeometry.h>
#include <vesta/TextureFont.h>
#include <vesta/DataChunk.h>
#include <vesta/Atmosphere.h>
#include <vesta/SingleTextureTiledMap.h>
#include <vesta/HierarchicalTiledMap.h>
#include <vesta/PlanetGridLayer.h>
#include <vesta/DDSLoader.h>
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

using namespace vesta;
using namespace Eigen;
using namespace std;


static const double KeyboardRotationAcceleration = 3.5;

static const QString CloudTextureSource = "earth-clouds-alpha.png";
static const QString EarthTextureSource = "earth.jpg";
static const QString EarthRealisticTextureSource = "bm-earth-may-water.png";

static const unsigned int ShadowMapSize = 2048;
static const unsigned int ReflectionMapSize = 512;

static JPLEphemeris* g_jplEph = NULL;

static double StartOfTime = GregorianDate(1900, 1, 1).toTDBSec();

static TextureProperties PlanetTextureProperties()
{
    TextureProperties props;
    props.addressS = TextureProperties::Wrap;
    props.addressT = TextureProperties::Clamp;
    return props;
}

static const char* AsteroidFamilyNames[] =
{
    "Main Belt Asteroids",
    "Hilda Asteroids",
    "Jupiter Trojans",
    "Kuiper Belt",
    "Near Earth Objects"
};


const char* CloseApproachers[] =
{
#if 0
    "2009 SC170",
    "2009 SP104",
    "2009 SN103",
    "2009 SH2",
    "2009 TB",
    "2009 SU104",
    "2009 TD17",
    "2009 TU",
    "2009 TM8",
    "2009 TH8",
    "2009 UE",
    "2009 UD",
    "2009 UU1",
    "2009 UW87",
    "2009 VA",
    "2009 VT1",
    "2009 VX",
    "2009 VZ39",
    "2009 WP6",
    "2009 WU25",
    "2009 WQ6",
    "2009 WX7",
    "2009 WR52",
    "2009 WW7",
    "2009 WJ6",
    "2009 WD54",
    "2009 WG106",
    "2009 WV51",
    "2009 WQ52",
    "2009 WV25",
    "2009 YS",
    "2009 XR1",
    "2009 YR",
    "2010 AH3",
    "2010 AR1",
    "2010 AN60",
    "2010 AL30",
    "2010 AG30",
    "2010 AF40",
    "2010 CB19",
    "2010 CS19",
    "2010 CK19",
    "2010 CJ18",
    "2010 DJ1",
    "2010 DE2",
    "2010 DU1",
    "2010 ES12",
    "2010 FS",
    "2010 FD6",
    "2010 FM",
    "2010 FU9",
    "2010 FW9",
    "2010 GH7",
    "2010 GV23",
    "2010 GF7",
    "2010 GA6",
    "2010 GM23",
    "2010 HP20",
    "2010 HF",
    "2010 JW34",
    "2010 JA",
    "2010 JO33",
    "2010 JL88",
    "2010 KK37",
    "2010 KO10",
    "2010 KV39",
    "2010 NH",
    "2010 NN",
    "2010 PJ9",
    "2010 RZ11",
    "2010 QG2",
    "2010 RB12",
    "2010 RM80",
    "2010 RX30",
    "2010 RF12",
    "2010 RS80",
    "2010 RM82",
#endif
    "2010 SP3",
    "Cruithne",
    "1998 UP1",
};

struct TLESet
{
    QString name;
    QUrl url;
};

TLESet s_TLESets[] =
{
    { "brightest", QString("http://www.celestrak.com/NORAD/elements/visual.txt") },
    //{ "iridium",   QString("http://www.celestrak.com/NORAD/elements/iridium.txt") },
    // { "iridium 33 debris", QString("http://www.celestrak.com/NORAD/elements/iridium-33-debris.txt") },
    //{ "geostationary", QString("http://www.celestrak.com/NORAD/elements/geo.txt") },
    { "gps", QString("http://www.celestrak.com/NORAD/elements/gps-ops.txt") },
};


class MoonRotationModel : public RotationModel
{
public:
    MoonRotationModel() {}

    virtual Eigen::Quaterniond orientation(double t) const
    {
        double d = secondsToDays(t); // time in Julian days
        double T = d / 36525.0; // time in Julian centuries

        double E1  = toRadians(125.045 -  0.0529921 * d);
        double E2  = toRadians(250.089 -  0.1059842 * d);
        double E3  = toRadians(260.008 + 13.012009  * d);
        double E4  = toRadians(176.625 + 13.3407154 * d);
        double E5  = toRadians(357.529 +  0.9856993 * d);
        double E6  = toRadians(311.589 + 26.4057084 * d);
        double E7  = toRadians(134.963 + 13.0649930 * d);
        double E8  = toRadians(276.617 +  0.3287146 * d);
        double E9  = toRadians( 34.226 +  1.7484877 * d);
        double E10 = toRadians( 15.134 -  0.1589763 * d);
        double E11 = toRadians(119.743 +  0.0036096 * d);
        double E12 = toRadians(239.961 +  0.1643573 * d);
        double E13 = toRadians( 25.053 + 12.9590088 * d);

        double a0 = 269.9949
                    + 0.0013*T
                    - 3.8787 * sin(E1)
                    - 0.1204 * sin(E2)
                    + 0.0700 * sin(E3)
                    - 0.0172 * sin(E4)
                    + 0.0072 * sin(E6)
                    - 0.0052 * sin(E10)
                    + 0.0043 * sin(E13);

        double d0 = 66.5392
                    + 0.0130 * T
                    + 1.5419 * cos(E1)
                    + 0.0239 * cos(E2)
                    - 0.0278 * cos(E3)
                    + 0.0068 * cos(E4)
                    - 0.0029 * cos(E6)
                    + 0.0009 * cos(E7)
                    + 0.0008 * cos(E10)
                    - 0.0009 * cos(E13);

        double W =    38.3213
                    + 13.17635815 * d
                    - 1.4e-12 * d * d
                    + 3.5610 * sin(E1)
                    + 0.1208 * sin(E2)
                    - 0.0642 * sin(E3)
                    + 0.0158 * sin(E4)
                    + 0.0252 * sin(E5)
                    - 0.0066 * sin(E6)
                    - 0.0047 * sin(E7)
                    - 0.0046 * sin(E8)
                    + 0.0028 * sin(E9)
                    + 0.0052 * sin(E10)
                    + 0.0040 * sin(E11)
                    + 0.0019 * sin(E12)
                    - 0.0044 * sin(E13);

        return Quaterniond(AngleAxisd(toRadians(a0),        Vector3d::UnitZ())) *
               Quaterniond(AngleAxisd(toRadians(90.0 - d0), Vector3d::UnitX())) *
               Quaterniond(AngleAxisd(toRadians(90.0 + W),  Vector3d::UnitZ()));
    }

    virtual Eigen::Vector3d angularVelocity(double /* t */) const
    {
        return Vector3d::UnitZ();
    }
};


class FrameVisualizer : public Visualizer
{
public:
    FrameVisualizer(Geometry* geometry, Frame* frame) :
        Visualizer(geometry),
        m_frame(frame)
    {
    }

    Quaterniond orientation(const Entity* /* parent */, double t) const
    {
        if (m_frame.isValid())
        {
            return m_frame->orientation(t);
        }
        else
        {
            return Quaterniond::Identity();
        }
    }

private:
    counted_ptr<Frame> m_frame;
};


// LocalTiledMap loads texture tiles from a directory structure on
// a file system.
class LocalTiledMap : public HierarchicalTiledMap
{
public:
    // The pattern is a string that will be used to construct a tile name given
    // the level, column, and row. %1, %2, and %3 in the string will be replaced
    // with the values of the level, column, and row, respectively.
    //
    // Example pattern: "earthmap/level%1/tile_%2_%3.png"
    LocalTiledMap(TextureMapLoader* loader, const QString& tileNamePattern, bool flipped, unsigned int tileSize, unsigned int levelCount) :
        HierarchicalTiledMap(loader, tileSize),
        m_tileNamePattern(tileNamePattern),
        m_flipped(flipped),
        m_levelCount(levelCount)
    {
    }

    virtual string tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row)
    {
        // Row may be inverted here if the tiles are arranged so that the northernmost
        // tile in a level is at row 0.
        unsigned int y = m_flipped ? (1 << level) - 1 - row : row;
        QString s = m_tileNamePattern.arg(level).arg(column).arg(y);
        return string(s.toUtf8().data());
    }

    virtual bool isValidTileAddress(unsigned int level, unsigned int column, unsigned int row)
    {
        return level < m_levelCount && column < (1u << (level + 1)) && row < (1u << level);
    }

    virtual bool tileResourceExists(const std::string& resourceId)
    {
        if (QString(resourceId.c_str()).startsWith("wms:"))
        {
            return true;//level < levelCount;
        }
        else
        {
            return QFileInfo(resourceId.c_str()).exists();
        }
    }

private:
    QString m_tileNamePattern;
    bool m_flipped;
    unsigned int m_levelCount;
};


// LocalTiledMap loads texture tiles from a Web Map Server
class WMSTiledMap : public HierarchicalTiledMap
{
public:
    WMSTiledMap(TextureMapLoader* loader, const QString& layerName, unsigned int tileSize, unsigned int levelCount) :
        HierarchicalTiledMap(loader, tileSize),
        m_levelCount(levelCount)
    {
        m_tileNamePattern = QString("wms:") + layerName + ",%1,%2,%3";
    }

    virtual string tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row)
    {
        QString s = m_tileNamePattern.arg(level).arg(column).arg(row);
        return string(s.toUtf8().data());
    }

    virtual bool isValidTileAddress(unsigned int level, unsigned int column, unsigned int row)
    {
        return level < m_levelCount && column < (1u << (level + 1)) && row < (1u << level);
    }

    virtual bool tileResourceExists(const std::string& /* resourceId */)
    {
        return true;//level < levelCount;
    }

private:
    QString m_tileNamePattern;
    unsigned int m_levelCount;
};


// LocalTiledMap loads texture tiles from a Web Map Server
class MultiWMSTiledMap : public HierarchicalTiledMap
{
public:
    MultiWMSTiledMap(TextureMapLoader* loader,
                const QString& baseLayerName,
                unsigned int baseLayerLevelCount,
                const QString& detailLayerName,
                unsigned int detailLayerLevelCount,
                unsigned int tileSize) :
        HierarchicalTiledMap(loader, tileSize),
        m_baseLayerLevelCount(baseLayerLevelCount),
        m_detailLayerLevelCount(detailLayerLevelCount)
    {
        m_baseTileNamePattern = QString("wms:") + baseLayerName + ",%1,%2,%3";
        m_detailTileNamePattern = QString("wms:") + detailLayerName + ",%1,%2,%3";
    }

    virtual string tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row)
    {
        QString s;
        if (level < m_baseLayerLevelCount)
        {
            s = m_baseTileNamePattern.arg(level).arg(column).arg(row);
        }
        else
        {
            s = m_detailTileNamePattern.arg(level).arg(column).arg(row);
        }
        return string(s.toUtf8().data());
    }

    virtual bool isValidTileAddress(unsigned int level, unsigned int column, unsigned int row)
    {
        return level < max(m_baseLayerLevelCount, m_detailLayerLevelCount) && column < (1u << (level + 1)) && row < (1u << level);
    }

    virtual bool tileResourceExists(const std::string& /* resourceId */)
    {
        return true;//level < levelCount;
    }

private:
    QString m_baseTileNamePattern;
    QString m_detailTileNamePattern;
    unsigned int m_baseLayerLevelCount;
    unsigned int m_detailLayerLevelCount;
};


static double JDtoSeconds(double jd)
{
    return daysToSeconds(jd - vesta::J2000);
}


static void SafeRelease(Object* obj)
{
    if (obj)
    {
        obj->release();
    }
}


static Spectrum ObjectLabelColor(const QString& name)
{
    Spectrum labelColor(1.0f, 1.0f, 1.0f);
    if (name.contains("IRIDIUM 33 DEB"))
    {
        labelColor = Spectrum(0.35f, 0.25f, 1.0f);
    }
    else if (name.contains("IRIDIUM"))
    {
        labelColor = Spectrum(0.30f, 1.0f, 0.0f);
    }
    else if (name.startsWith("GPS"))
    {
        labelColor = Spectrum(0.8, 0.0f, 1.0f);
    }
    else if (name == "Sun")
    {
        labelColor = Spectrum(1.0f, 1.0f, 0.0f);
    }
    else if (name == "Mercury")
    {
        labelColor = Spectrum(0.8f, 0.4f, 0.1f);
    }
    else if (name == "Venus")
    {
        labelColor = Spectrum(1.0f, 1.0f, 0.9f);
    }
    else if (name == "Earth")
    {
        labelColor = Spectrum(0.7f, 0.8f, 1.0f);
    }
    else if (name == "Mars")
    {        
        labelColor = Spectrum(0.8f, 0.4f, 0.3f);
    }
    else if (name == "Jupiter")
    {
        labelColor = Spectrum(1.0f, 1.0f, 0.5f);
    }
    else if (name == "Saturn")
    {
        labelColor = Spectrum(0.8f, 1.0f, 0.5f);
    }
    else if (name == "Uranus")
    {
        labelColor = Spectrum(0.5f, 1.0f, 1.0f);
    }
    else if (name == "Neptune")
    {
        labelColor = Spectrum(0.5f, 0.5f, 1.0f);
    }
    else if (name == "Pluto")
    {
        labelColor = Spectrum(0.5f, 0.5f, 0.5f);
    }
    else if (name == "Moon")
    {
        labelColor = Spectrum(0.5f, 0.5f, 0.5f);
    }
    else if (name.startsWith("20"))
    {
        labelColor = Spectrum(0.7f, 0.5f, 0.3f);
    }
    return labelColor;
}


UniverseView::UniverseView(QWidget *parent) :
    QGLWidget(parent),
    m_mouseMovement(0),
    m_universe(NULL),
    m_observer(NULL),
    m_spacecraftObserver(NULL),
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
    m_earthAtmosphere(NULL),
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
    m_anaglyphEnabled(false),
    m_highlightedAsteroidFamily(0),
    m_infoTextVisible(true),
    m_networkManager(NULL),
    m_videoEncoder(NULL)
{
    m_textureLoader = new NetworkTextureLoader(this);
    m_renderer = new UniverseRenderer();

    m_labelFont = new TextureFont();
    m_titleFont = new TextureFont();
    m_spacecraftIcon = m_textureLoader->loadTexture(":/icons/disk.png", TextureProperties(TextureProperties::Clamp));

    initPlanetEphemeris();
    initializeUniverse();
    initializeSkyLayers();

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
    m_timer->start();

    setFocusPolicy(Qt::StrongFocus);

    initNetwork();
}


UniverseView::~UniverseView()
{
    makeCurrent();

    SafeRelease(m_universe);
    SafeRelease(m_observer);
    SafeRelease(m_spacecraftObserver);

    delete m_renderer;
}


QSize UniverseView::minimumSizeHint() const
{
    return QSize(50, 50);
}


QSize UniverseView::sizeHint() const
{
    return QSize(800, 600);
}


static void labelPlanet(Entity* planet, TextureFont* font, TextureMap* icon)
{
    if (planet)
    {
        Spectrum color = ObjectLabelColor(planet->name().c_str());
        LabelGeometry* label = new LabelGeometry(planet->name(), font, color, 6.0f);
        label->setIcon(icon);
        label->setIconColor(color);
        planet->setVisualizer("label", new Visualizer(label));
    }
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

    glShadeModel(GL_FLAT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

    QFile fontFile("sans-light-24.txf");
    if (fontFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = fontFile.readAll();
        DataChunk chunk(data.data(), data.size());
        m_titleFont->loadTxf(&chunk);
    }
    else
    {
        qDebug() << "missing font";
    }

    QFile labelFontFile("sans-12.txf");
    if (labelFontFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = labelFontFile.readAll();
        DataChunk chunk(data.data(), data.size());
        m_labelFont->loadTxf(&chunk);
    }

#if ENABLE_CONSTELLATIONS
    ConstellationsLayer* constellations = new ConstellationsLayer(m_universe->starCatalog());
    constellations->setDiagramColor(Spectrum(0.0f, 0.2f, 0.5f));
    constellations->setDefaultConstellations();
    constellations->setVisibility(true);
    m_renderer->addSkyLayer(constellations);
#endif

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

    labelPlanet(m_universe->findFirst("Sun"), m_labelFont.ptr(), m_spacecraftIcon.ptr());
    labelPlanet(m_universe->findFirst("Mercury"), m_labelFont.ptr(), m_spacecraftIcon.ptr());
    labelPlanet(m_universe->findFirst("Venus"), m_labelFont.ptr(), m_spacecraftIcon.ptr());
    labelPlanet(m_universe->findFirst("Earth"), m_labelFont.ptr(), m_spacecraftIcon.ptr());
    labelPlanet(m_universe->findFirst("Mars"), m_labelFont.ptr(), m_spacecraftIcon.ptr());
    labelPlanet(m_universe->findFirst("Jupiter"), m_labelFont.ptr(), m_spacecraftIcon.ptr());
    labelPlanet(m_universe->findFirst("Saturn"), m_labelFont.ptr(), m_spacecraftIcon.ptr());
    labelPlanet(m_universe->findFirst("Uranus"), m_labelFont.ptr(), m_spacecraftIcon.ptr());
    labelPlanet(m_universe->findFirst("Neptune"), m_labelFont.ptr(), m_spacecraftIcon.ptr());
    labelPlanet(m_universe->findFirst("Pluto"), m_labelFont.ptr(), m_spacecraftIcon.ptr());
}


bool
UniverseView::initPlanetEphemeris()
{
    g_jplEph = JPLEphemeris::load("de406_1800-2100.dat");

    return g_jplEph != NULL;
}


void
UniverseView::initNetwork()
{
    m_networkManager = new QNetworkAccessManager();
    QNetworkDiskCache* cache = new QNetworkDiskCache();
    cache->setCacheDirectory(QDesktopServices::storageLocation(QDesktopServices::CacheLocation));
    m_networkManager->setCache(cache);

    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(tleDataReceived(QNetworkReply*)));

    for (unsigned int i = 0; i < sizeof(s_TLESets) / sizeof(s_TLESets[0]); ++i)
    {
        const TLESet& tleSet = s_TLESets[i];
        QNetworkRequest request(QNetworkRequest(tleSet.url));
        // Offline mode:
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork);
        QNetworkReply* reply = m_networkManager->get(request);
    }

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

    m_renderer->beginViewSet(m_universe, m_simulationTime);

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    if (m_anaglyphEnabled)
    {
        Quaterniond cameraOrientation = m_observer->absoluteOrientation(m_simulationTime);
        Vector3d cameraPosition = m_observer->absolutePosition(m_simulationTime);
        double eyeSeparation = m_observer->position().norm() / 50.0;//0.0001;
        float focalPlaneDistance = eyeSeparation * 25.0f;
        float nearDistance = 0.00001f;
        float farDistance = 1.0e12f;
        float y = tan(0.5f * m_fovY) * nearDistance;
        float x = y * mainViewport.aspectRatio();

        float stereoOffset = float(eyeSeparation) * nearDistance / focalPlaneDistance;

        PlanarProjection leftProjection(PlanarProjection::Perspective,  -x + float(stereoOffset), x, -y, y, nearDistance, farDistance);
        PlanarProjection rightProjection(PlanarProjection::Perspective, -x, x - float(stereoOffset), -y, y, nearDistance, farDistance);

        Vector3d leftEyePosition = cameraPosition + cameraOrientation * (Vector3d::UnitX() * -eyeSeparation);
        Vector3d rightEyePosition = cameraPosition + cameraOrientation * (Vector3d::UnitX() * eyeSeparation);

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
    else
    {
        m_renderer->renderView(&lighting, m_observer, m_fovY, mainViewport);
    }

#define ZOOM_INSET 0
#if ZOOM_INSET
    // Multiview example: small zoomed view in lower right corner
    int insetWidth = size().width() / 3;
    int insetHeight = size().height() / 3;
    Viewport insetViewport(size().width() - insetWidth, 0, insetWidth, insetHeight);
    glScissor(insetViewport.x(), insetViewport.y(), insetViewport.width(), insetViewport.height());
    glEnable(GL_SCISSOR_TEST);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);
    m_renderer->renderView(m_spacecraftObserver, toRadians(5.0f), insetViewport);
#endif

    m_renderer->endViewSet();

    if (m_videoEncoder)
    {
        QImage image = grabFrameBuffer(false);
        image = image.scaled(QSize(m_videoEncoder->getWidth(), m_videoEncoder->getHeight()), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_videoEncoder->encodeImage(image);
    }

    // Draw informational text over the 3D view
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

    glColor4f(0.2f, 0.4f, 1.0f, max(0.0f, min(1.0f, float((m_realTime - 5.0) / 5.0))));
    // Show the title
    if (m_infoTextVisible)
    {
        if (m_titleFont.isValid())
        {
            m_titleFont->bind();

            const char* title = "Cosmographia";
            int titleWidth = m_titleFont->textWidth(title);
            m_titleFont->render(title, Vector2f(floor((viewportWidth - titleWidth) / 2.0f), viewportHeight - 30.0f));
        }

        if (m_labelFont.isValid())
        {
            // Show the current simulation time
            GregorianDate date = GregorianDate::UTCDateFromTDBSec(m_simulationTime);

            m_labelFont->bind();
            m_labelFont->render(date.toString(), Vector2f(10.0f, 10.0f));

            QString frameCountString = QString("%1 fps").arg(m_framesPerSecond);
            m_labelFont->render(frameCountString.toLatin1().data(), Vector2f(viewportWidth - 200.0f, 10.0f));

            // Display information about the selection
            if (m_selectedBody.isValid())
            {
                m_labelFont->render(m_selectedBody->name(), Vector2f(10.0f, viewportHeight - 20.0f));
                Vector3d r = m_observer->absolutePosition(m_simulationTime) - m_selectedBody->position(m_simulationTime);
                double distance = r.norm();

                WorldGeometry* world = dynamic_cast<WorldGeometry*>(m_selectedBody->geometry());
                if (world)
                {
                    distance -= dynamic_cast<WorldGeometry*>(m_selectedBody->geometry())->maxRadius();
                }

                QString distanceString = QString("Distance: %1 km").arg(distance);
                m_labelFont->render(distanceString.toLatin1().data(), Vector2f(10.0f, viewportHeight - 35.0f));

                if (world)
                {
                    Vector3d q = m_selectedBody->orientation(m_simulationTime).conjugate() * r;
                    q = q.normalized();
                    double latitude = toDegrees(asin(q.z()));
                    double longitude = toDegrees(atan2(q.y(), q.x()));
                    QString coordString = QString("Subpoint: %1, %2").arg(latitude).arg(longitude);
                    m_labelFont->render(coordString.toLatin1().data(), Vector2f(10.0f, viewportHeight - 50.0f));
                }
            }

            {
                unsigned int tileCount = 0;

                if (m_textureLoader->wmsHandler())
                {
                    tileCount = m_textureLoader->wmsHandler()->pendingTileCount();
                }

                if (tileCount > 0)
                {
                    QString tileCountString = QString("Loading tiles: %1").arg(tileCount);
                    m_labelFont->render(tileCountString.toLatin1().data(), Vector2f(10.0f, viewportHeight - 65.0f));
                }
            }

            {
                QString fovInfo = QString("FOV: %1").arg(toDegrees(m_fovY), 6, 'f', 1);
                m_labelFont->render(fovInfo.toLatin1().data(), Vector2f(float(viewportWidth / 2), 10.0f));
            }

            if (m_paused)
            {
                QString timeScaleString = QString("%1x (paused)").arg(m_timeScale);
                m_labelFont->render(timeScaleString.toLatin1().data(), Vector2f(viewportWidth - 100.0f, 10.0f));
            }
            else
            {
                QString timeScaleString = QString("%1x").arg(m_timeScale);
                m_labelFont->render(timeScaleString.toLatin1().data(), Vector2f(viewportWidth - 100.0f, 10.0f));
            }
        }
    }

    glDisable(GL_TEXTURE_2D);

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


void UniverseView::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}


void UniverseView::mousePressEvent(QMouseEvent *event)
{
    m_lastMousePosition = event->pos();
    m_mouseMovement = 0;
}


void UniverseView::mouseReleaseEvent(QMouseEvent* event)
{
    // Process the mouse release as a click if the mouse hasn't moved
    // much since the mouse button was pressed
    if (m_mouseMovement < 4)
    {        
        if (event->button() == Qt::LeftButton)
        {
            // Left-click selects the object beneath the cursor

            // Get the click point in normalized device coordinaes
            Vector2d ndc = Vector2d(double(event->pos().x()) / double(size().width()),
                                    double(event->pos().y()) / double(size().height())) * 2.0 - Vector2d::Ones();
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
                m_selectedBody = pickResult.hitObject();
            }
            else
            {
                m_selectedBody = NULL;
            }
        }
    }
}


void UniverseView::mouseMoveEvent(QMouseEvent *event)
{
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
        m_fovY *= zoomFactor;
        m_fovY = max(toRadians(1.0), min(toRadians(90.0), m_fovY));
    }
    else if (rightButton || (leftButton && alt))
    {
        // Right dragging changes the observer's orientation without
        // modifying the position. Rotate by an amount that depends on
        // the current field of view.
        double fovAdjust = toDegrees(m_fovY) / 50.0;
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
    default:
        QWidget::keyReleaseEvent(event);
        break;
    }

    // Star brightness adjustment
    StarsLayer* stars = dynamic_cast<StarsLayer*>(m_universe->layer("stars"));
    if (stars)
    {
        static unsigned int earthLayer = 0;
        const char* EarthLayerNames[] = {
            "bmng-jan-nb", "bmng-feb-nb", "bmng-mar-nb",
            "bmng-apr-nb", "bmng-may-nb", "bmng-jun-nb",
            "bmng-jul-nb", "bmng-aug-nb", "bmng-sep-nb",
            "bmng-oct-nb", "bmng-nov-nb", "bmng-dec-nb",
        };
        if (event->text() == "[")
        {
            stars->setLimitingMagnitude(max(3.0f, stars->limitingMagnitude() - 0.2f));
        }
        else if (event->text() == "]")
        {
            stars->setLimitingMagnitude(min(13.0f, stars->limitingMagnitude() + 0.2f));
        }
        else if (event->text() == ")")
        {
            earthLayer = (earthLayer + 1) % 12;
            setPlanetMap("Earth", new WMSTiledMap(m_textureLoader.ptr(), EarthLayerNames[earthLayer], 512, 7));
        }
        else if (event->text() == "(")
        {
            if (earthLayer == 0)
            {
                earthLayer = 11;
            }
            else
            {
                earthLayer--;
            }
            setPlanetMap("Earth", new WMSTiledMap(m_textureLoader.ptr(), EarthLayerNames[earthLayer], 512, 7));
        }
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
            plot->updateSamples(iter->generator, m_simulationTime - plot->windowDuration(), m_simulationTime, 300);
        }
        else
        {
            plot->updateSamples(iter->trajectory.ptr(), m_simulationTime - plot->windowDuration(), m_simulationTime, 100);
        }
    }
}


static MeshGeometry*
loadMeshFile(const string& fileName, TextureMapLoader* textureLoader)
{
    MeshGeometry* meshGeometry = MeshGeometry::loadFromFile(fileName, textureLoader);
    if (!meshGeometry)
    {
        QMessageBox::warning(NULL, "Missing mesh file", QString("Error opening mesh file %1.").arg(fileName.c_str()));
    }
    else
    {
        // Optimize the mesh. The optimizations can be expensive for large meshes, but they can dramatically
        // improve rendering performance. The best solution is to use mesh files that are already optimized, but
        // the average model loaded off the web benefits from some preprocessing at load time.
        meshGeometry->mergeSubmeshes();
        meshGeometry->uniquifyVertices();
    }

    return meshGeometry;
}


static Body*
createComponentBody(const string& name, Entity* parent, double startTime, double duration)
{
    Body* body = new Body();
    body->setName(name);
    vesta::Arc* arc = new vesta::Arc();
    arc->setCenter(parent);
    arc->setDuration(duration);

    BodyFixedFrame* parentFixedFrame = new BodyFixedFrame(parent);
    arc->setBodyFrame(parentFixedFrame);
    arc->setTrajectoryFrame(parentFixedFrame);
    body->chronology()->addArc(arc);
    body->chronology()->setBeginning(startTime);

    return body;
}


/** Create a new planet
  * @param rotationPeriod rotation period in hours
  */
static Body* createPlanet(const QString& name,
                          Entity* parent,
                          double rotationPeriod)
{
    Body* body = new Body();
    body->setName(name.toUtf8().data());

    vesta::Arc* arc = new vesta::Arc();
    arc->setCenter(parent);
    arc->setDuration(daysToSeconds(365.25 * 200.0));
    arc->setRotationModel(new UniformRotationModel(Vector3d::UnitZ(), toRadians(360.0 / (rotationPeriod * 3600)), 0.0, 0.0));
    body->chronology()->setBeginning(StartOfTime);
    body->chronology()->addArc(arc);

    return body;
}


/** Create a new planet
  * @param rotationPeriod rotation period in hours
  */
static Body* createPlanet(const QString& name,
                          Entity* parent,
                          Trajectory* orbit,
                          RotationModel* rotation,
                          double radius)
{
    Body* body = new Body();
    body->setName(name.toUtf8().data());

    vesta::Arc* arc = new vesta::Arc();
    arc->setCenter(parent);
    arc->setDuration(daysToSeconds(365.25 * 200.0));
    arc->setTrajectory(orbit);
    arc->setRotationModel(rotation);
    body->chronology()->setBeginning(StartOfTime);
    body->chronology()->addArc(arc);

    WorldGeometry* globe = new WorldGeometry();
    globe->setSphere(float(radius));
    body->setGeometry(globe);

    return body;
}


TextureMap*
UniverseView::loadTexture(const QString& location, const TextureProperties& texProps)
{
    return m_textureLoader->loadTexture(location.toUtf8().data(), texProps);
}


static QList<Body*> LoadAsteroidOrbits(KeplerianSwarm* mainBelt,
                                       KeplerianSwarm* hildaFamily,
                                       KeplerianSwarm* jupiterTrojans,
                                       KeplerianSwarm* kuiperBelt,
                                       KeplerianSwarm* nearEarthObjects,
                                       const QString& fileName,
                                       unsigned int maxOrbits = 1000000)
{
    QList<Body*> NEOs;
    int hildaCount = 0;
    const double AU = 149597870.691;

    QFile orbitFile(fileName);
    if (!orbitFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Asteroid orbits file not found";
    }

    QSet<QString> closeApproachers;
    for (unsigned int i = 0; i < sizeof(CloseApproachers) / sizeof(CloseApproachers[0]); ++i)
    {
        closeApproachers.insert(CloseApproachers[i]);
    }

    QRegExp provisionalDesignation("\\d\\d\\d\\d [A-Z][A-Z]\\d*");

    unsigned int orbitsRead = 0;
    QTextStream in(&orbitFile);
    while (in.status() == QTextStream::Ok && orbitsRead < maxOrbits)
    {
        QString record = in.readLine();
        if (in.status() == QTextStream::Ok)
        {
            QString epochYear = record.mid(106, 4);
            QString epochMonth = record.mid(110, 2);
            QString epochDay = record.mid(112, 2);
            QString meanAnomaly = record.mid(115, 10);
            QString argOfPeri = record.mid(126, 10);
            QString ascendingNode = record.mid(137, 10);
            QString inclination = record.mid(148, 9);
            QString eccentricity = record.mid(158, 10);
            QString sma = record.mid(169, 12);

            QString name = record.mid(7, 19).trimmed();

            double discoveryTime = -daysToSeconds(365.25 * 100);
            if (provisionalDesignation.indexIn(name) == 0)
            {
                double year = name.mid(0, 4).toDouble();
                double halfMonth = name.at(5).toAscii() - 'A';
                discoveryTime = (year - 2000.0) * 365.25 + halfMonth * (365.25 / 24.0);
                discoveryTime *= 86400.0;
            }

            // float absMag = record.mid(43, 5).toFloat();

            // Epoch is Terrestrial Time
            GregorianDate epoch(epochYear.toInt(), epochMonth.toInt(), epochDay.toInt(), 12, 0, 0);
            epoch.setTimeScale(TimeScale_TT);
            double smaAU = sma.toDouble();
            double periodYears = pow(smaAU, 1.5);

            OrbitalElements el;
            el.eccentricity = eccentricity.toDouble();
            el.periapsisDistance = (1.0 - el.eccentricity) * smaAU * AU;
            el.inclination = toRadians(inclination.toDouble());
            el.longitudeOfAscendingNode = toRadians(ascendingNode.toDouble());
            el.argumentOfPeriapsis = toRadians(argOfPeri.toDouble());
            el.meanAnomalyAtEpoch = toRadians(meanAnomaly.toDouble());
            el.meanMotion = 2.0 * PI / daysToSeconds(365.25 * periodYears);
            el.epoch = epoch.toTDBSec();

            bool isNEO = el.periapsisDistance / AU < 1.3;
            bool isHilda = el.inclination < toRadians(20.0) && el.eccentricity < 0.3 && smaAU > 3.7 && smaAU < 4.1;
            bool isJupiterTrojan = smaAU > 5.1 && smaAU < 5.35 && el.eccentricity < 0.25;
            bool isKBO = smaAU >= 30.0;

            if (isHilda)
            {
                hildaFamily->addObject(el, discoveryTime);
                hildaCount++;
            }
            else if (isJupiterTrojan)
            {
                jupiterTrojans->addObject(el, discoveryTime);
            }
            else if (isKBO)
            {
                kuiperBelt->addObject(el, discoveryTime);
            }
            else if (isNEO)
            {
                nearEarthObjects->addObject(el, discoveryTime);
            }
            else
            {
                mainBelt->addObject(el, discoveryTime);
            }

            if (closeApproachers.contains(name))
            {
                qDebug() << name << ", " << epoch.toString().c_str();

                Body* neo = new Body();
                neo->setName(name.toUtf8().data());

                vesta::Arc* arc = new vesta::Arc();
                arc->setDuration(daysToSeconds(365.25 * 200.0));
                arc->setTrajectoryFrame(InertialFrame::eclipticJ2000());
                arc->setBodyFrame(InertialFrame::eclipticJ2000());
                arc->setTrajectory(new KeplerianTrajectory(el));

                neo->chronology()->setBeginning(0.0);
                neo->chronology()->addArc(arc);

                NEOs << neo;
            }

            ++orbitsRead;
        }
    }

    qDebug() << "hildas: " << hildaCount;

    return NEOs;
}


Body* CreateAsteroidGroup(Body* sun, const QString& name)
{
    KeplerianSwarm* swarmGeometry = new KeplerianSwarm();
    swarmGeometry->setEpoch(GregorianDate(2010, 1, 1).toTDBSec());
    swarmGeometry->setColor(Spectrum(0.7f, 0.5f, 0.3f));
    swarmGeometry->setOpacity(0.5f);
    swarmGeometry->setPointSize(1.0f);

    Body* asteroids = new Body();
    asteroids->setName(name.toUtf8().data());
    asteroids->setGeometry(swarmGeometry);
    asteroids->setVisible(false);

    vesta::Arc* arc = new vesta::Arc();
    arc->setDuration(daysToSeconds(365.25 * 200));
    arc->setCenter(sun);
    arc->setBodyFrame(InertialFrame::eclipticJ2000());
    asteroids->chronology()->addArc(arc);
    asteroids->chronology()->setBeginning(StartOfTime);

    return asteroids;
}


void UniverseView::initializeUniverse()
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
    Body* sun = createPlanet("Sun", ssb, 25.58 * 24.0);
    sun->chronology()->firstArc()->setTrajectory(new FixedPointTrajectory(Vector3d(50.0, 0.0, 0.0)));
    WorldGeometry* sunSphere = new WorldGeometry();
    sunSphere->setSphere(695000.0);
    sunSphere->setBaseMap(loadTexture("sun.jpg", PlanetTextureProperties()));
    sunSphere->setEmissive(true);
    sun->setGeometry(sunSphere);

    m_universe->addEntity(sun);

    Body* earth = createPlanet("Earth", sun, 23.934);
    {
        // Create a Keplerian orbit for the Earth
        OrbitalElements el;
        el.periapsisDistance = 1.5e8;
        el.meanMotion = toRadians(360.0) / daysToSeconds(365.25);
        earth->chronology()->firstArc()->setTrajectory(new KeplerianTrajectory(el));

        if (g_jplEph)
        {
            earth->chronology()->firstArc()->setTrajectory(g_jplEph->trajectory(JPLEphemeris::EarthMoonBarycenter));
        }

        UniformRotationModel* earthRotation = new UniformRotationModel(Vector3d::UnitZ(), toRadians(360.9856235) / 86400.0, toRadians(190.147 + 90.0));
        earth->chronology()->firstArc()->setRotationModel(earthRotation);
    }

    WorldGeometry* earthSphere = new WorldGeometry();
    earthSphere->setSphere(6378.0);
    earth->setGeometry(earthSphere);
    m_universe->addEntity(earth);

    Body* moon = createPlanet("Moon", earth, 23.934);
    moon->chronology()->firstArc()->setTrajectory(g_jplEph->trajectory(JPLEphemeris::Moon));
    moon->chronology()->firstArc()->setRotationModel(new MoonRotationModel);

    WorldGeometry* moonSphere = new WorldGeometry();
    moonSphere->setSphere(1737.1);
    moon->setGeometry(moonSphere);
    m_universe->addEntity(moon);

    //tex = loadTexture("textures/moon.dds", PlanetTextureProperties());
    //moonSphere->setBaseMap(tex);
    //WMSTiledMap* tiledMoonMap = new WMSTiledMap(m_textureLoader.ptr(), "wms:moon-lo,%1,%2,%3", 512, 10);
    //WMSTiledMap* tiledMoonMap = new WMSTiledMap(m_textureLoader.ptr(), "wms:moon-clementine", 512, 6);
    //moonSphere->setBaseMap(tiledMoonMap);

    /*
    TextureProperties compNormalMapProps = PlanetTextureProperties();
    compNormalMapProps.usage = TextureProperties::CompressedNormalMap;
    moonSphere->setNormalMap(loadTexture("textures/moon-normal.dds", compNormalMapProps));
    */

    UniformRotationModel* defaultRotation = new UniformRotationModel(Vector3d::UnitZ(), toRadians(360.0) / 86400.0, 0.0, 0.0);
    m_universe->addEntity(createPlanet("Mercury", sun, g_jplEph->trajectory(JPLEphemeris::Mercury), defaultRotation, 2439.7));
    m_universe->addEntity(createPlanet("Venus",   sun, g_jplEph->trajectory(JPLEphemeris::Venus),   defaultRotation, 6051.8));
    m_universe->addEntity(createPlanet("Mars",    sun, g_jplEph->trajectory(JPLEphemeris::Mars),    defaultRotation, 3389.5 / 3389));
    m_universe->addEntity(createPlanet("Jupiter", sun, g_jplEph->trajectory(JPLEphemeris::Jupiter), defaultRotation, 69911.0));
    m_universe->addEntity(createPlanet("Saturn",  sun, g_jplEph->trajectory(JPLEphemeris::Saturn),  defaultRotation, 58232.0));
    m_universe->addEntity(createPlanet("Uranus",  sun, g_jplEph->trajectory(JPLEphemeris::Uranus),  defaultRotation, 25362.0));
    m_universe->addEntity(createPlanet("Neptune", sun, g_jplEph->trajectory(JPLEphemeris::Neptune), defaultRotation, 24622));
    m_universe->addEntity(createPlanet("Pluto",   sun, g_jplEph->trajectory(JPLEphemeris::Pluto),   defaultRotation, 1195));

    //TextureMap* tex = loadTexture(EarthTextureSource, PlanetTextureProperties());
    //LocalTiledMap* tiledMap = new LocalTiledMap(m_textureLoader.ptr(), "/Users/chrislaurel/dev/maps/jmiiearth/level%1/tx_%2_%3.dds", true, 1024);
    setPlanetMap("Earth", new MultiWMSTiledMap(m_textureLoader.ptr(), "bmng-apr-nb", 7, "earth-global-mosaic", 13, 480));
    setPlanetMap("Moon", new WMSTiledMap(m_textureLoader.ptr(), "moon-clementine", 512, 6));
    setPlanetMap("Mars", new WMSTiledMap(m_textureLoader.ptr(), "mars-mdim-moc_na", 512, 10));
    //setPlanetMap("Mars", new WMSTiledMap(m_textureLoader.ptr(), "mars-viking", 512, 6));

    m_observer = new Observer(earth);
    m_observer->addRef();
    m_observer->setPosition(Vector3d(0.0, 0.0, 1.0e9));

    m_controller->setObserver(m_observer);

    {
        // Add mars map
    }

    QFile starFile("tycho2.stars");
    if (starFile.open(QFile::ReadOnly))
    {
        StarCatalog* stars = new StarCatalog();
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

    m_defaultSpacecraftMesh = loadMeshFile("models/jason.obj", m_textureLoader.ptr());
    if (m_defaultSpacecraftMesh.isValid())
    {
        m_defaultSpacecraftMesh->setMeshScale(0.004f / m_defaultSpacecraftMesh->boundingSphereRadius());
    }

    // Create the main asteroid belt
    if (1)
    {
        Body* mainBelt = CreateAsteroidGroup(sun, "Main Belt Asteroids");
        Body* hildaFamily = CreateAsteroidGroup(sun, "Hilda Asteroids");
        Body* jupiterTrojans = CreateAsteroidGroup(sun, "Jupiter Trojans");
        Body* kuiperBelt = CreateAsteroidGroup(sun, "Kuiper Belt");
        Body* nearEarthObjects = CreateAsteroidGroup(sun, "Near Earth Objects");

        QString asteroidOrbitsFileName = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/" + "astorb.dat";
        qDebug() << asteroidOrbitsFileName;

        QList<Body*> NEOs;
        NEOs = LoadAsteroidOrbits(dynamic_cast<KeplerianSwarm*>(mainBelt->geometry()),
                                  dynamic_cast<KeplerianSwarm*>(hildaFamily->geometry()),
                                  dynamic_cast<KeplerianSwarm*>(jupiterTrojans->geometry()),
                                  dynamic_cast<KeplerianSwarm*>(kuiperBelt->geometry()),
                                  dynamic_cast<KeplerianSwarm*>(nearEarthObjects->geometry()),
                                  asteroidOrbitsFileName,
                                  1000000);
        foreach (Body* neo, NEOs)
        {
            neo->chronology()->firstArc()->setCenter(sun);
            m_universe->addEntity(neo);

            Spectrum labelColor(0.7f, 0.5f, 0.3f);
            LabelGeometry* label = new LabelGeometry(neo->name(), m_labelFont.ptr(), labelColor, 6.0f);
            label->setIcon(m_spacecraftIcon.ptr());
            label->setIconColor(labelColor);
            neo->setVisualizer("label", new Visualizer(label));

            WorldGeometry* asteroidGeom = new WorldGeometry();
            asteroidGeom->setSphere(1.0f);
            neo->setGeometry(asteroidGeom);
        }

        m_universe->addEntity(mainBelt);
        m_universe->addEntity(hildaFamily);
        m_universe->addEntity(jupiterTrojans);
        m_universe->addEntity(kuiperBelt);
        m_universe->addEntity(nearEarthObjects);
    }
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
    milkyWayLayer->setVisibility(true);
    milkyWayLayer->setOpacity(0.3f);
    milkyWayLayer->setDrawOrder(-1);
    milkyWayLayer->setTexture(m_textureLoader->loadTexture("textures/milkyway.jpg", PlanetTextureProperties()));
    milkyWayLayer->setOrientation(InertialFrame::galactic()->orientation());
    m_universe->setLayer("milky way", milkyWayLayer);
    milkyWayLayer->setVisibility(false);
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

    m_realTime += dt;

    if (!isPaused())
    {
        m_simulationTime += dt * timeScale();
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

    repaint();
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
        frame = new TwoBodyRotatingFrame(center->chronology()->firstArc()->center(), center);
    }
    else
    {
        frame = InertialFrame::equatorJ2000();
    }

    m_observer->updateCenter(center, m_simulationTime);
    m_observer->updatePositionFrame(frame, m_simulationTime);
    m_observer->updatePointingFrame(frame, m_simulationTime);
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
    m_paused = paused;
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
    m_timeScale = timeScale;
}


void
UniverseView::setSimulationTime(double tsec)
{
    m_simulationTime = tsec;
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
UniverseView::toggleBodyAxes(bool /* checked */)
{
}


void
UniverseView::toggleFrameAxes(bool /* checked */)
{
}


void
UniverseView::toggleVelocityVector(bool /* checked */)
{
}


void
UniverseView::setCloudLayerVisibility(bool checked)
{
    Entity* earth = m_universe->findFirst("Earth");
    if (earth)
    {
        WorldGeometry* geom = dynamic_cast<WorldGeometry*>(earth->geometry());
        if (geom)
        {
            if (checked)
            {
                TextureProperties planetTexProperties;
                planetTexProperties.addressS = TextureProperties::Wrap;
                planetTexProperties.addressT = TextureProperties::Clamp;

                TextureMap* cloudTex = loadTexture(CloudTextureSource, planetTexProperties);
                geom->setCloudMap(cloudTex);
                geom->setCloudAltitude(7.0f);
            }
            else
            {
                geom->setCloudMap(NULL);
            }
        }
    }
}


void
UniverseView::setAsteroidVisibility(bool checked)
{
    unsigned int familyCount = sizeof(AsteroidFamilyNames) / sizeof(AsteroidFamilyNames[0]);

    for (unsigned int i = 0; i < familyCount; ++i)
    {
        Entity* asteroids = m_universe->findFirst(AsteroidFamilyNames[i]);
        if (asteroids)
        {
            asteroids->setVisible(checked);
        }
    }
}


void
UniverseView::highlightAsteroidFamily()
{
    unsigned int familyCount = sizeof(AsteroidFamilyNames) / sizeof(AsteroidFamilyNames[0]);

    Entity* asteroids = m_universe->findFirst(AsteroidFamilyNames[m_highlightedAsteroidFamily]);

    // Unhighlight the current group
    if (asteroids)
    {
        KeplerianSwarm* swarmGeometry = dynamic_cast<KeplerianSwarm*>(asteroids->geometry());
        if (swarmGeometry)
        {
            swarmGeometry->setColor(Spectrum(0.7f, 0.5f, 0.3f));
            swarmGeometry->setOpacity(0.15f);
            swarmGeometry->setPointSize(1.0f);
        }
    }

    m_highlightedAsteroidFamily = (m_highlightedAsteroidFamily + 1) % familyCount;

    if (m_highlightedAsteroidFamily != 0)
    {
        asteroids = m_universe->findFirst(AsteroidFamilyNames[m_highlightedAsteroidFamily]);
        if (asteroids)
        {
            KeplerianSwarm* swarmGeometry = dynamic_cast<KeplerianSwarm*>(asteroids->geometry());
            if (swarmGeometry)
            {
                swarmGeometry->setColor(Spectrum(1.0f, 0.2f, 0.1f));
                swarmGeometry->setOpacity(0.9f);
                swarmGeometry->setPointSize(3.0f);
            }
        }
    }
}


void
UniverseView::setMilkyWayVisibility(bool checked)
{
    SkyLayer* layer = m_universe->layer("milky way");
    if (layer)
    {
        layer->setVisibility(checked);
    }
}


void
UniverseView::setEquatorialGridVisibility(bool checked)
{
    SkyLayer* equatorialGrid = m_universe->layer("equatorial grid");
    if (equatorialGrid)
    {
        equatorialGrid->setVisibility(checked);
    }
}


void
UniverseView::setEclipticVisibility(bool checked)
{
    SkyLayer* ecliptic = m_universe->layer("ecliptic");
    if (ecliptic)
    {
        ecliptic->setVisibility(checked);
    }
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
UniverseView::setLabelMode(LabelMode /* mode */)
{
}


void
UniverseView::setAntennaLobeVisibility(bool /* enable */)
{
}


void
UniverseView::setTrajectoryVisibility(bool /* enable */)
{
}


void
UniverseView::plotTrajectory()
{
    if (m_selectedBody.isValid())
    {
        vesta::Arc* arc = m_selectedBody->chronology()->firstArc();
        string visName = string("traj - ") + m_selectedBody->name();
        Visualizer* vis = arc->center()->visualizer(visName);
        if (!vis)
        {
            TrajectoryGeometry* plot = new TrajectoryGeometry();
            Visualizer* visualizer = new Visualizer(plot);
            plot->setFrame(arc->trajectoryFrame());
            plot->setWindowDuration(arc->trajectory()->period());
            plot->setDisplayedPortion(TrajectoryGeometry::WindowBeforeCurrentTime);
            plot->setFadeFraction(0.25);
            plot->setColor(ObjectLabelColor(m_selectedBody->name().c_str()));
            arc->center()->setVisualizer(visName, visualizer);

            TrajectoryPlotEntry plotEntry;
            plotEntry.trajectory = arc->trajectory();
            plotEntry.visualizer = visualizer;
            plotEntry.generator = NULL;
            m_trajectoryPlots.push_back(plotEntry);
        }
    }
}


void
UniverseView::setPlanetOrbitsVisibility(bool enable)
{
    const char* planetNames[] = {
        "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Moon"
    };

    for (unsigned int i = 0; i < sizeof(planetNames) / sizeof(planetNames[0]); ++i)
    {
        Entity* planet = m_universe->findFirst(planetNames[i]);

        if (planet)
        {
            if (enable)
            {
                vesta::Arc* arc = planet->chronology()->firstArc();
                string visName = string("traj - ") + planet->name();
                Visualizer* vis = arc->center()->visualizer(visName);
                if (!vis)
                {
                    TrajectoryGeometry* plot = new TrajectoryGeometry();
                    Visualizer* visualizer = new Visualizer(plot);
                    plot->setFrame(arc->trajectoryFrame());
                    plot->setWindowDuration(arc->trajectory()->period());
                    plot->setDisplayedPortion(TrajectoryGeometry::WindowBeforeCurrentTime);
                    plot->setFadeFraction(0.25);
                    plot->setColor(ObjectLabelColor(planet->name().c_str()));
                    arc->center()->setVisualizer(visName, visualizer);

                    TrajectoryPlotEntry plotEntry;
                    plotEntry.trajectory = arc->trajectory();
                    plotEntry.visualizer = visualizer;
                    plotEntry.generator = NULL;
                    m_trajectoryPlots.push_back(plotEntry);
                }
            }
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
UniverseView::plotTrajectoryObserver()
{
    if (m_selectedBody.isValid())
    {
        Entity* center = m_observer->center();
        Frame* frame = m_observer->positionFrame();
        string visName = string("traj - ") + m_selectedBody->name();
        Visualizer* vis = center->visualizer(visName);
        if (!vis)
        {
            TrajectoryGeometry* plot = new TrajectoryGeometry();
            Visualizer* visualizer = new Visualizer(plot);
            plot->setFrame(frame);
            plot->setWindowDuration(daysToSeconds(3.0));
            plot->setDisplayedPortion(TrajectoryGeometry::WindowBeforeCurrentTime);
            plot->setFadeFraction(0.5);
            plot->setColor(ObjectLabelColor(m_selectedBody->name().c_str()));
            center->setVisualizer(visName, visualizer);

            TrajectoryPlotEntry plotEntry;
            plotEntry.trajectory = NULL;
            plotEntry.visualizer = visualizer;
            plotEntry.generator = new BodyPositionSampleGenerator(m_selectedBody.ptr(), center, frame);
            m_trajectoryPlots.push_back(plotEntry);
        }
    }
}


void
UniverseView::setNormalMaps(bool enable)
{
    Entity* body = m_universe->findFirst("Earth");
    if (body)
    {
        WorldGeometry* world = dynamic_cast<WorldGeometry*>(body->geometry());
        if (world)
        {
            if (enable)
            {
                TextureProperties planetTexProperties;
                planetTexProperties.addressS = TextureProperties::Wrap;
                planetTexProperties.addressT = TextureProperties::Clamp;
                world->setNormalMap(loadTexture("earth-normal.jpg", planetTexProperties));
            }
            else
            {
                world->setNormalMap(NULL);
            }
        }
    }
}


void
UniverseView::setShadows(bool enable)
{
    m_renderer->setShadowsEnabled(enable);
}


void
UniverseView::setReflections(bool enable)
{
    qDebug() << "reflections: " << enable;
    m_reflectionsEnabled = enable;
}


void
UniverseView::setAtmospheres(bool enable)
{
    Entity* body = m_universe->findFirst("Earth");
    if (body)
    {
        WorldGeometry* world = dynamic_cast<WorldGeometry*>(body->geometry());
        if (world)
        {
            if (enable)
            {
                if (!m_earthAtmosphere)
                {

                    QFile atmFile("earth.atmscat");
                    if (atmFile.open(QIODevice::ReadOnly))
                    {
                        QByteArray data = atmFile.readAll();
                        DataChunk chunk(data.data(), data.size());
                        m_earthAtmosphere = Atmosphere::LoadAtmScat(&chunk);
                    }

                    if (m_earthAtmosphere)
                    {
                        m_earthAtmosphere->generateTextures();
                        m_earthAtmosphere->addRef();
                    }
                }

                world->setAtmosphere(m_earthAtmosphere);
            }
            else
            {
                world->setAtmosphere(NULL);
            }
        }
    }
}


void
UniverseView::setAmbientLight(bool enable)
{
    float light = enable ? 0.2f : 0.0f;
    m_renderer->setAmbientLight(Spectrum(light, light, light));
}


void
UniverseView::setRealisticPlanets(bool /* enable */)
{
}


void
UniverseView::setAnaglyphStereo(bool enable)
{
    m_anaglyphEnabled = enable;
}


void
UniverseView::setInfoText(bool enable)
{
    m_infoTextVisible = enable;
}


void
UniverseView::addTleObject(const QString& name, const QString& line1, const QString& line2)
{
    TleTrajectory* tleTrajectory = TleTrajectory::Create(line1.toAscii().data(), line2.toAscii().data());
    if (!tleTrajectory)
    {
        qDebug() << "Failed: " << name;
        return;
    }

    string bodyName = name.toUtf8().data();

    Body* spacecraft = new Body();
    spacecraft->setName(bodyName);

    Entity* earth = m_universe->findFirst("Earth");
    double month = daysToSeconds(30.0);
    vesta::Arc* arc = new Arc();
    arc->setTrajectory(tleTrajectory);
    arc->setCenter(earth);
    arc->setDuration(month * 2.0);
    arc->setBodyFrame(new TwoBodyRotatingFrame(earth, spacecraft));

    spacecraft->chronology()->setBeginning(tleTrajectory->epoch() - month);
    spacecraft->chronology()->addArc(arc);

    string labelText = bodyName;
    Spectrum labelColor = ObjectLabelColor(name);

    LabelGeometry* label = new LabelGeometry(labelText, m_labelFont.ptr(), labelColor, 6.0f);
    label->setFadeSize(tleTrajectory->boundingSphereRadius());
    label->setFadeRange(new FadeRange(40.0f, 20.0f));
    label->setIcon(m_spacecraftIcon.ptr());
    label->setIconColor(labelColor);
    spacecraft->setVisualizer("label", new Visualizer(label));

    spacecraft->setGeometry(m_defaultSpacecraftMesh.ptr());

    qDebug() << labelText.c_str();

    m_universe->addEntity(spacecraft);
}


void
UniverseView::tleDataReceived(QNetworkReply* reply)
{
    qDebug() << "TLE data received";

    QVariant fromCache = reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute);
    qDebug() << "page from cache?" << fromCache.toBool();

    if (reply->open(QIODevice::ReadOnly))
    {
        QTextStream str(reply);

        QTextStream::Status status = QTextStream::Ok;
        while (status == QTextStream::Ok)
        {
            QString name = str.readLine();
            QString tleLine1 = str.readLine();
            QString tleLine2 = str.readLine();

            name = name.trimmed();
            status = str.status();
            if (status == QTextStream::Ok)
            {
                if (name.isEmpty())
                {
                    break;
                }
                else
                {
                    addTleObject(name, tleLine1, tleLine2);
                }
            }
        }
    }
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


void
UniverseView::setPlanetMap(const QString& planetName, vesta::TiledMap* tiledMap)
{
    Entity* planet = m_universe->findFirst(planetName.toUtf8().data());
    if (planet)
    {
        WorldGeometry* world = dynamic_cast<WorldGeometry*>(planet->geometry());
        if (world)
        {
            world->setBaseMap(tiledMap);
        }
    }
}


void
UniverseView::replaceEntity(Entity* entity)
{
    Entity* existingBody = m_universe->findFirst(entity->name());
    if (existingBody)
    {
        m_universe->removeEntity(existingBody);
    }

    m_universe->addEntity(entity);

    labelPlanet(entity, m_labelFont.ptr(), m_spacecraftIcon.ptr());
    qDebug() << "Body: " << entity->name().c_str() << ", " << entity->position(m_simulationTime).norm();
}

