// This file is part of Cosmographia.
//
// Copyright (C) 2010-2012 Chris Laurel <claurel@gmail.com>
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

#ifndef _UNIVERSE_VIEW_H_
#define _UNIVERSE_VIEW_H_

#include "NetworkTextureLoader.h"
#include "catalog/UniverseCatalog.h"
#include "qtwrapper/BodyObject.h"
#include "qtwrapper/VisualizerObject.h"
#include <QDeclarativeView>
#include <QTimer>
#include <QDateTime>
#include <QGestureEvent>
#include <QUrl>
#include <vesta/Universe.h>
#include <vesta/Observer.h>
#include <vesta/TextureMapLoader.h>
#include <vesta/MeshGeometry.h>
#include <vesta/Visualizer.h>
#include <vesta/TiledMap.h>

class QVideoEncoder;
class ObserverAction;
class Viewpoint;
class MarkerLayer;
class GalleryView;

class QGraphicsScene;

class Leo3DState;

namespace vesta
{
    class UniverseRenderer;
    class TextureFont;
    class ConeGeometry;
    class Atmosphere;
    class CubeMapFramebuffer;
    class ObserverController;
    class Trajectory;
    class TrajectoryPlotGenerator;
    class GlareOverlay;
}

class UniverseView : public QDeclarativeView
{
    Q_OBJECT
    Q_PROPERTY(double realTime READ realTime);
    Q_PROPERTY(bool labelsVisible READ labelVisibility WRITE setLabelVisibility);
    Q_PROPERTY(bool surfaceFeatureLabelsVisible READ surfaceFeatureLabelVisibility WRITE setSurfaceFeatureLabelVisibility);
    Q_PROPERTY(bool centerIndicatorVisible READ centerIndicatorVisibility WRITE setCenterIndicatorVisibility NOTIFY centerIndicatorVisibilityChanged);
    Q_PROPERTY(bool constellationFiguresVisible READ constellationFigureVisibility WRITE setConstellationFigureVisibility);
    Q_PROPERTY(bool constellationNamesVisible READ constellationNameVisibility WRITE setConstellationNameVisibility);
    Q_PROPERTY(bool starNamesVisible READ starNameVisibility WRITE setStarNameVisibility);
    Q_PROPERTY(bool equatorialGridVisible READ equatorialGridVisibility WRITE setEquatorialGridVisibility NOTIFY equatorialGridVisibilityChanged);
    Q_PROPERTY(bool eclipticVisible READ eclipticVisibility WRITE setEclipticVisibility);

    Q_PROPERTY(bool shadows READ shadows WRITE setShadows);
    Q_PROPERTY(bool eclipseShadows READ eclipseShadows WRITE setEclipseShadows);
    Q_PROPERTY(bool reflections READ reflections WRITE setReflections);
    Q_PROPERTY(bool cloudsVisible READ cloudsVisible WRITE setCloudsVisible);
    Q_PROPERTY(bool atmospheresVisible READ atmospheresVisible WRITE setAtmospheresVisible);
    Q_PROPERTY(bool sunGlare READ sunGlare WRITE setSunGlare);
    Q_PROPERTY(bool diffractionSpikes READ diffractionSpikes WRITE setDiffractionSpikes);
    Q_PROPERTY(bool milkyWayVisible READ milkyWayVisible WRITE setMilkyWayVisible);
    Q_PROPERTY(bool planetOrbitsVisible READ planetOrbitsVisibility WRITE setPlanetOrbitsVisibility);

    Q_PROPERTY(int earthMapMonth READ earthMapMonth WRITE setEarthMapMonth);

    Q_PROPERTY(double simulationTime READ simulationTime WRITE setSimulationTime);
    Q_PROPERTY(QString currentTimeString READ currentTimeString NOTIFY timeChanged);
    Q_PROPERTY(QDateTime simulationDateTime READ simulationDateTime WRITE setSimulationDateTime NOTIFY simulationDateTimeChanged);
    Q_PROPERTY(double timeScale READ timeScale WRITE setTimeScale NOTIFY timeScaleChanged);
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pauseStateChanged);

    Q_PROPERTY(double limitingMagnitude READ limitingMagnitude WRITE setLimitingMagnitude NOTIFY limitingMagnitudeChanged);
    Q_PROPERTY(double ambientLight READ ambientLight WRITE setAmbientLight NOTIFY ambientLightChanged);
    Q_PROPERTY(StereoMode stereoMode READ stereoMode WRITE setStereoMode);
    Q_PROPERTY(int antialiasingSamples READ antialiasingSamples WRITE setAntialiasingSamples);
    Q_PROPERTY(double gotoObjectTime READ gotoObjectTime WRITE setGotoObjectTime);

    Q_PROPERTY(bool galleryVisible READ isGalleryVisible WRITE setGalleryVisible);

    Q_PROPERTY(bool recordingVideo READ isRecordingVideo NOTIFY recordingVideoChanged)
    Q_PROPERTY(double recordedVideoLength READ recordedVideoLength NOTIFY recordedVideoLengthChanged)

    Q_ENUMS(StereoMode)

public:
    Q_INVOKABLE BodyObject* getSelectedBody() const;
    Q_INVOKABLE void setSelectedBody(BodyObject* body);
    Q_INVOKABLE BodyObject* getCentralBody() const;
    Q_INVOKABLE void setCentralBody(BodyObject* body);
    Q_INVOKABLE void setCentralBodyFixed(BodyObject* body);
    Q_INVOKABLE void trackBody(BodyObject* body);
    Q_INVOKABLE void trackBodyLevel(BodyObject* body);
    Q_INVOKABLE VisualizerObject* createBodyDirectionVisualizer(BodyObject* from, BodyObject* target);
    Q_INVOKABLE void plotTrajectory(QObject* body);
    Q_INVOKABLE void clearTrajectoryPlots(QObject* body);
    Q_INVOKABLE bool hasTrajectoryPlots(QObject* body) const;
    Q_INVOKABLE void setStateFromUrl(const QUrl& url);
    Q_INVOKABLE void setMouseClickEventProcessed(bool accepted);
    Q_INVOKABLE void setMouseMoveEventProcessed(bool accepted);

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    UniverseView(QWidget *parent, vesta::Universe* universe, UniverseCatalog* catalog);
    ~UniverseView();

    void initializeDeclarativeUi(const QString& qmlFileName);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    bool isPaused() const
    {
        return m_paused;
    }

    double timeScale() const
    {
        return m_timeScale;
    }

    /** Get the current simulation time as the number of seconds elapsed
      * since J2000 TDB.
      */
    double simulationTime() const
    {
        return m_simulationTime;
    }

    QDateTime simulationDateTime() const;

    void startVideoRecording(QVideoEncoder* encoder);
    void finishVideoRecording();
    bool isRecordingVideo() const
    {
        return m_videoEncoder != NULL;
    }

    double recordedVideoLength() const;

    QVideoEncoder* videoEncoder() const
    {
        return m_videoEncoder;
    }

    vesta::Universe* universe() const
    {
        return m_universe.ptr();
    }

    vesta::TextureMapLoader* textureLoader() const
    {
        return m_textureLoader.ptr();
    }

    vesta::Entity* selectedBody() const
    {
        return m_selectedBody.ptr();
    }

    double realTime() const
    {
        return m_realTime;
    }

    bool labelVisibility() const
    {
        return m_labelsVisible;
    }

    bool surfaceFeatureLabelVisibility() const
    {
        return m_surfaceFeatureLabelsVisible;
    }

    bool centerIndicatorVisibility() const
    {
        return m_centerIndicatorVisible;
    }

    bool constellationFigureVisibility() const;
    bool constellationNameVisibility() const;
    bool starNameVisibility() const;
    bool equatorialGridVisibility() const;
    bool eclipticVisibility() const;
    bool planetOrbitsVisibility() const;

    bool shadows() const;
    bool eclipseShadows() const;
    bool reflections() const;
    bool cloudsVisible() const;
    bool atmospheresVisible() const;
    bool sunGlare() const;
    bool diffractionSpikes() const;
    bool milkyWayVisible() const;

    double gotoObjectTime() const
    {
        return m_gotoObjectTime;
    }

    void setGotoObjectTime(double tsec)
    {
        m_gotoObjectTime = tsec;
    }

    double limitingMagnitude() const;
    double ambientLight() const;

    QString currentTimeString() const;

    enum TimeDisplayMode
    {
        TimeDisplay_UTC       = 0,
        TimeDisplay_Local     = 1,
        TimeDisplay_Multiple  = 2
    };

    TimeDisplayMode timeDisplay() const
    {
        return m_timeDisplay;
    }

    enum StereoMode
    {
        Mono               = 0,
        SideBySide         = 1,
        AnaglyphRedCyan    = 2,
        AnaglyphCyanRed    = 3
    };

    StereoMode stereoMode() const
    {
        return m_stereoMode;
    }

    int antialiasingSamples() const
    {
        return m_antialiasingSamples;
    }

    void setSelectedBody(vesta::Entity* body);
    QImage grabFrameBuffer(bool withAlpha = false);

    void replaceEntity(vesta::Entity* entity, const BodyInfo* info);

    void initializeGL();

    QUrl getStateUrl();

    enum FontRole { LabelFont, TitleFont, TextFont };
    vesta::TextureFont* font(FontRole role) const;

    int earthMapMonth() const
    {
        return m_earthMapMonth;
    }

    GalleryView* gallery() const
    {
        return m_galleryView;
    }

    bool isGalleryVisible() const;

signals:
    void timeChanged();
    void simulationDateTimeChanged();
    void timeScaleChanged(double);
    void pauseStateChanged(bool);
    void contextMenuTriggered(int x, int y, BodyObject* body);
    void limitingMagnitudeChanged(double);
    void ambientLightChanged(double);
    void equatorialGridVisibilityChanged(bool);
    void centerIndicatorVisibilityChanged(bool);
    void recordingVideoChanged();
    void recordedVideoLengthChanged(double);

public slots:
    void tick();
    void setPaused(bool paused);
    void setCurrentTime();
    void setTimeScale(double scale);
    void setSimulationTime(double tsec);
    void setSimulationDateTime(QDateTime dateTime);
    void inertialObserver(bool checked);
    void bodyFixedObserver(bool checked);
    void synodicObserver(bool checked);
    void lockedObserver(bool checked);
    void setObserverCenter();
    void setMilkyWayVisible(bool checked);
    void setPlanetOrbitsVisibility(bool checked);
    void setEquatorialGridVisibility(bool checked);
    void setEclipticVisibility(bool checked);
    void setEquatorialPlaneVisibility(bool checked);
    void setPlanetographicGridVisibility(bool checked);
    void setConstellationFigureVisibility(bool checked);
    void setConstellationNameVisibility(bool checked);
    void setStarNameVisibility(bool checked);
    void setLabelVisibility(bool enable);
    void setSurfaceFeatureLabelVisibility(bool enable);
    void setCenterIndicatorVisibility(bool enable);
    void setShadows(bool enable);
    void setEclipseShadows(bool enable);
    void setCloudsVisible(bool enable);
    void setAtmospheresVisible(bool enable);
    void setAmbientLight(bool enable);
    void setAmbientLight(double brightness);
    void setReflections(bool enable);
    void setStereoMode(StereoMode stereoMode);
    void setAntialiasingSamples(int samples);
    void setSunGlare(bool enable);
    void setDiffractionSpikes(bool enable);
    void setEarthMapMonth(int month);

    void setInfoText(bool enable);
    void plotTrajectory(vesta::Entity* body, const BodyInfo* info);
    void plotTrajectoryObserver(const BodyInfo* info);
    void clearTrajectoryPlots(vesta::Entity* body);
    bool hasTrajectoryPlots(vesta::Entity* body) const;
    void setSelectedBody(const QString& name);
    void gotoSelectedObject();
    void centerSelectedObject();
    void gotoHome();
    void setViewpoint(Viewpoint* viewpoint);
    void setTimeDisplay(TimeDisplayMode mode);
    void setLimitingMagnitude(double appMag);

    void setUpdateInterval(unsigned int msec);
    void findObject();

    void setGalleryVisible(bool visible);
    void toggleGallery();

    void copyNextFrameToClipboard(bool withAlpha = false);

    void setStatusMessage(const QString& message);

private slots:
    void setFOV(double fovY);

protected:
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);
    void paintEvent(QPaintEvent* event);
    void focusOutEvent(QFocusEvent* event);
    void focusInEvent(QFocusEvent* event);
    bool event(QEvent* event);

private:
    QString bodyName(const vesta::Entity* body) const;
    void drawInfoOverlay();
    void drawFrame(float width, float height);
    void begin2DDrawing();
    void end2DDrawing();
    void drawFovReticle(float brightness);
    bool skyLayerVisible(const std::string& layerName) const;
    void setSkyLayerVisible(const std::string& layerName, bool enable);

    enum FrameType
    {
        Frame_Inertial,
        Frame_BodyFixed,
        Frame_Synodic,
        Frame_Locked,
        Frame_LockedLevel
    };

    void setCenterAndFrame(vesta::Entity* center, FrameType f, const Eigen::Vector3d& refVector = Eigen::Vector3d::UnitY());
    void initializeSkyLayers();
    void initializeObserver();
    double secondsFromBaseTime() const;
    vesta::TextureMap* loadTexture(const QString& location, const vesta::TextureProperties& texProps);

    void initNetwork();
    bool initPlanetEphemeris();

    void updateTrajectoryPlots();
    bool gestureEvent(QGestureEvent* event);

    vesta::Entity* pickObject(const QPoint& point);
    void constrainViewerPosition(double maxRange);

private:
    int m_mouseMovement;
    QPoint m_mouseDownPosition;
    QPoint m_lastMousePosition;
    double m_lastDoubleClickTime;
    bool m_mouseClickEventProcessed;
    bool m_mouseMoveEventProcessed;
    bool m_sceneHadFocus;
    bool m_glInitialized;

    vesta::counted_ptr<vesta::Universe> m_universe;
    UniverseCatalog* m_catalog;
    vesta::counted_ptr<vesta::Observer> m_observer;
    vesta::counted_ptr<vesta::ObserverController> m_controller;
    vesta::UniverseRenderer* m_renderer;
    vesta::counted_ptr<vesta::GlareOverlay> m_glareOverlay;
    FrameType m_observerFrame;
    double m_fovY;

    bool m_rollLeft;
    bool m_rollRight;
    bool m_pitchDown;
    bool m_pitchUp;

    QTimer* m_timer;
    double m_realTime;
    double m_simulationTime;

    QDateTime m_baseTime;
    bool m_firstTick;
    double m_lastTickTime;

    double m_timeScale;
    bool m_paused;

    vesta::counted_ptr<vesta::TextureFont> m_titleFont;
    vesta::counted_ptr<vesta::TextureFont> m_textFont;
    vesta::counted_ptr<vesta::TextureFont> m_labelFont;
    vesta::counted_ptr<vesta::TextureMap> m_spacecraftIcon;

    unsigned int m_frameCount;
    double m_frameCountStartTime;
    double m_framesPerSecond;

    vesta::counted_ptr<vesta::Entity> m_selectedBody;

    vesta::counted_ptr<NetworkTextureLoader> m_textureLoader;
    vesta::counted_ptr<vesta::CubeMapFramebuffer> m_reflectionMap;
    vesta::counted_ptr<vesta::MeshGeometry> m_defaultSpacecraftMesh;

    bool m_reflectionsEnabled;
    StereoMode m_stereoMode;
    int m_antialiasingSamples;
    bool m_sunGlareEnabled;

    struct TrajectoryPlotEntry
    {
        TrajectoryPlotEntry();
        vesta::counted_ptr<vesta::Visualizer> visualizer;
        vesta::counted_ptr<vesta::Trajectory> trajectory;
        vesta::TrajectoryPlotGenerator* generator;
        unsigned int sampleCount;
        double leadDuration;
    };
    std::vector<TrajectoryPlotEntry> m_trajectoryPlots;

    bool m_planetOrbitsVisible;
    bool m_infoTextVisible;
    bool m_labelsVisible;
    bool m_surfaceFeatureLabelsVisible;
    bool m_centerIndicatorVisible;
    double m_gotoObjectTime;

    vesta::counted_ptr<ObserverAction> m_observerAction;

    QVideoEncoder* m_videoEncoder;
    double m_videoRecordingStartTime;

    TimeDisplayMode m_timeDisplay;
    bool m_wireframe;
    bool m_captureNextImage;

    double m_reticleUpdateTime;
    double m_statusUpdateTime;
    QString m_statusMessage;

    MarkerLayer* m_markers;
    GalleryView* m_galleryView;

    int m_earthMapMonth;

    Leo3DState* m_leoState;
};

#endif // _UNIVERSE_VIEW_H_
