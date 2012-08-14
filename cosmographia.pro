# Qt project file for Cosmographia

TEMPLATE = app
TARGET = Cosmographia
DESTDIR = build

QT += opengl
QT += network
QT += declarative


#### App sources ####

MAIN_PATH = src/main

APP_SOURCES = \
    $$MAIN_PATH/main.cpp \
    $$MAIN_PATH/Addon.cpp \
    $$MAIN_PATH/ConstellationInfo.cpp \
    $$MAIN_PATH/Cosmographia.cpp \
    $$MAIN_PATH/FileOpenEventFilter.cpp \
    $$MAIN_PATH/HelpCatalog.cpp \
    $$MAIN_PATH/UniverseView.cpp \
    $$MAIN_PATH/Viewpoint.cpp \
    $$MAIN_PATH/NetworkTextureLoader.cpp \
    $$MAIN_PATH/LocalImageLoader.cpp \
    $$MAIN_PATH/DateUtility.cpp \
    $$MAIN_PATH/RotationUtility.cpp \
    $$MAIN_PATH/ChebyshevPolyTrajectory.cpp \
    $$MAIN_PATH/GalleryView.cpp \
    $$MAIN_PATH/InterpolatedRotation.cpp \
    $$MAIN_PATH/InterpolatedStateTrajectory.cpp \
    $$MAIN_PATH/JPLEphemeris.cpp \
    $$MAIN_PATH/KeplerianSwarm.cpp \
    $$MAIN_PATH/LinearCombinationTrajectory.cpp \
    $$MAIN_PATH/MarkerLayer.cpp \
    $$MAIN_PATH/MultiLabelVisualizer.cpp \
    $$MAIN_PATH/NumberFormat.cpp \
    $$MAIN_PATH/ObserverAction.cpp \
    $$MAIN_PATH/SkyLabelLayer.cpp \
    $$MAIN_PATH/TleTrajectory.cpp \
    $$MAIN_PATH/TwoVectorFrame.cpp \
    $$MAIN_PATH/UnitConversion.cpp \
    $$MAIN_PATH/WMSRequester.cpp \
    $$MAIN_PATH/WMSTiledMap.cpp \
    $$MAIN_PATH/MultiWMSTiledMap.cpp \
    $$MAIN_PATH/astro/Constants.cpp \
    $$MAIN_PATH/astro/Gust86.cpp \
    $$MAIN_PATH/astro/IAULunarRotationModel.cpp \
    $$MAIN_PATH/astro/Nutation.cpp \
    $$MAIN_PATH/astro/OsculatingElements.cpp \
    $$MAIN_PATH/astro/Precession.cpp \
    $$MAIN_PATH/astro/L1.cpp \
    $$MAIN_PATH/astro/MarsSat.cpp \
    $$MAIN_PATH/astro/TASS17.cpp \
    $$MAIN_PATH/catalog/AstorbLoader.cpp \
    $$MAIN_PATH/catalog/BodyInfo.cpp \
    $$MAIN_PATH/catalog/ChebyshevPolyFileLoader.cpp \
    $$MAIN_PATH/catalog/UniverseCatalog.cpp \
    $$MAIN_PATH/catalog/UniverseLoader.cpp \
    $$MAIN_PATH/geometry/FeatureLabelSetGeometry.cpp \
    $$MAIN_PATH/geometry/MeshInstanceGeometry.cpp \
    $$MAIN_PATH/geometry/MultiLabelGeometry.cpp \
    $$MAIN_PATH/geometry/SimpleTrajectoryGeometry.cpp \
    $$MAIN_PATH/geometry/TimeSwitchedGeometry.cpp \
    $$MAIN_PATH/vext/CompositeTrajectory.cpp \
    $$MAIN_PATH/vext/LocalTiledMap.cpp \
    $$MAIN_PATH/vext/NameTemplateTiledMap.cpp \
    $$MAIN_PATH/vext/PathRelativeTextureLoader.cpp \
    $$MAIN_PATH/vext/SimpleRotationModel.cpp \
    $$MAIN_PATH/compatibility/CatalogParser.cpp \
    $$MAIN_PATH/compatibility/CelBodyFixedFrame.cpp \
    $$MAIN_PATH/compatibility/CmodLoader.cpp \
    $$MAIN_PATH/compatibility/Scanner.cpp \
    $$MAIN_PATH/compatibility/TransformCatalog.cpp \
    $$MAIN_PATH/qtwrapper/BodyObject.cpp \
    $$MAIN_PATH/qtwrapper/UniverseCatalogObject.cpp \
    $$MAIN_PATH/qtwrapper/VisualizerObject.cpp


APP_HEADERS = \
    $$MAIN_PATH/Addon.h \
    $$MAIN_PATH/ConstellationInfo.h \
    $$MAIN_PATH/Cosmographia.h \
    $$MAIN_PATH/FileOpenEventFilter.h \
    $$MAIN_PATH/HelpCatalog.h \
    $$MAIN_PATH/UniverseView.h \
    $$MAIN_PATH/Viewpoint.h \
    $$MAIN_PATH/NetworkTextureLoader.h \
    $$MAIN_PATH/LocalImageLoader.h \
    $$MAIN_PATH/DateUtility.h \
    $$MAIN_PATH/RotationUtility.h \
    $$MAIN_PATH/ChebyshevPolyTrajectory.h \
    $$MAIN_PATH/GalleryView.h \
    $$MAIN_PATH/InterpolatedRotation.h \
    $$MAIN_PATH/InterpolatedStateTrajectory.h \
    $$MAIN_PATH/JPLEphemeris.h \
    $$MAIN_PATH/KeplerianSwarm.h \
    $$MAIN_PATH/LinearCombinationTrajectory.h \
    $$MAIN_PATH/MarkerLayer.h \
    $$MAIN_PATH/MultiLabelVisualizer.h \
    $$MAIN_PATH/NumberFormat.h \
    $$MAIN_PATH/ObserverAction.h \
    $$MAIN_PATH/SkyLabelLayer.h \
    $$MAIN_PATH/TleTrajectory.h \
    $$MAIN_PATH/TwoVectorFrame.h \
    $$MAIN_PATH/UnitConversion.h \
    $$MAIN_PATH/WMSRequester.h \
    $$MAIN_PATH/WMSTiledMap.h \
    $$MAIN_PATH/MultiWMSTiledMap.h \
    $$MAIN_PATH/astro/Constants.h \
    $$MAIN_PATH/astro/Gust86.h \
    $$MAIN_PATH/astro/IAULunarRotationModel.h \
    $$MAIN_PATH/astro/MarsSat.h \
    $$MAIN_PATH/astro/Nutation.h \
    $$MAIN_PATH/astro/OsculatingElements.h \
    $$MAIN_PATH/astro/Precession.h \
    $$MAIN_PATH/astro/Rotation.h \
    $$MAIN_PATH/astro/L1.h \
    $$MAIN_PATH/astro/TASS17.h \
    $$MAIN_PATH/catalog/AstorbLoader.h \
    $$MAIN_PATH/catalog/BodyInfo.h \
    $$MAIN_PATH/catalog/ChebyshevPolyFileLoader.h \
    $$MAIN_PATH/catalog/UniverseCatalog.h \
    $$MAIN_PATH/catalog/UniverseLoader.h \
    $$MAIN_PATH/geometry/FeatureLabelSetGeometry.h \
    $$MAIN_PATH/geometry/MeshInstanceGeometry.h \
    $$MAIN_PATH/geometry/MultiLabelGeometry.h \
    $$MAIN_PATH/geometry/SimpleTrajectoryGeometry.h \
    $$MAIN_PATH/geometry/TimeSwitchedGeometry.h \
    $$MAIN_PATH/vext/ArcStripParticleGenerator.h \
    $$MAIN_PATH/vext/CompositeTrajectory.h \
    $$MAIN_PATH/vext/LocalTiledMap.h \
    $$MAIN_PATH/vext/NameTemplateTiledMap.h \
    $$MAIN_PATH/vext/PathRelativeTextureLoader.h \
    $$MAIN_PATH/vext/SimpleRotationModel.h \
    $$MAIN_PATH/vext/StripParticleGenerator.h \
    $$MAIN_PATH/compatibility/CatalogParser.h \
    $$MAIN_PATH/compatibility/CelBodyFixedFrame.h \
    $$MAIN_PATH/compatibility/CmodLoader.h \
    $$MAIN_PATH/compatibility/Scanner.h \
    $$MAIN_PATH/compatibility/TransformCatalog.h \
    $$MAIN_PATH/qtwrapper/BodyObject.h \
    $$MAIN_PATH/qtwrapper/UniverseCatalogObject.h \
    $$MAIN_PATH/qtwrapper/VisualizerObject.h

VESTA_PATH = thirdparty/vesta
LIB3DS_PATH = thirdparty/lib3ds
GLEW_PATH = thirdparty/glew
NORADTLE_PATH = thirdparty/noradtle
QJSON_PATH = thirdparty/qjson
LUA_PATH = thirdparty/lua

VESTA_SOURCES = \
    $$VESTA_PATH/AlignedEllipsoid.cpp \
    $$VESTA_PATH/Arc.cpp \
    $$VESTA_PATH/ArrowGeometry.cpp \
    $$VESTA_PATH/ArrowVisualizer.cpp \
    $$VESTA_PATH/Atmosphere.cpp \
    $$VESTA_PATH/AxesVisualizer.cpp \
    $$VESTA_PATH/BillboardGeometry.cpp \
    $$VESTA_PATH/Body.cpp \
    $$VESTA_PATH/BodyDirectionVisualizer.cpp \
    $$VESTA_PATH/BodyFixedFrame.cpp \
    $$VESTA_PATH/CelestialCoordinateGrid.cpp \
    $$VESTA_PATH/Chronology.cpp \
    $$VESTA_PATH/ConeGeometry.cpp \
    $$VESTA_PATH/ConstellationsLayer.cpp \
    $$VESTA_PATH/CubeMapFramebuffer.cpp \
    $$VESTA_PATH/DataChunk.cpp \
    $$VESTA_PATH/DDSLoader.cpp \
    $$VESTA_PATH/Debug.cpp \
    $$VESTA_PATH/Entity.cpp \
    $$VESTA_PATH/FixedPointTrajectory.cpp \
    $$VESTA_PATH/FixedRotationModel.cpp \
    $$VESTA_PATH/Frame.cpp \
    $$VESTA_PATH/Framebuffer.cpp \
    $$VESTA_PATH/GeneralEllipse.cpp \
    $$VESTA_PATH/Geometry.cpp \
    $$VESTA_PATH/GeometryBuffer.cpp \
    $$VESTA_PATH/GlareOverlay.cpp \
    $$VESTA_PATH/GregorianDate.cpp \
    $$VESTA_PATH/HierarchicalTiledMap.cpp \
    $$VESTA_PATH/InertialFrame.cpp \
    $$VESTA_PATH/KeplerianTrajectory.cpp \
    $$VESTA_PATH/LabelGeometry.cpp \
    $$VESTA_PATH/LabelVisualizer.cpp \
    $$VESTA_PATH/LightSource.cpp \
    $$VESTA_PATH/LocalVisualizer.cpp \
    $$VESTA_PATH/MapLayer.cpp \
    $$VESTA_PATH/MeshGeometry.cpp \
    $$VESTA_PATH/NadirVisualizer.cpp \
    $$VESTA_PATH/Observer.cpp \
    $$VESTA_PATH/OrbitalElements.cpp \
    $$VESTA_PATH/ParticleSystemGeometry.cpp \
    $$VESTA_PATH/PickContext.cpp \
    $$VESTA_PATH/PlanarProjection.cpp \
    $$VESTA_PATH/PlaneGeometry.cpp \
    $$VESTA_PATH/PlanetaryRings.cpp \
    $$VESTA_PATH/PlanetGridLayer.cpp \
    $$VESTA_PATH/PlaneVisualizer.cpp \
    $$VESTA_PATH/PrimitiveBatch.cpp \
    $$VESTA_PATH/QuadtreeTile.cpp \
    $$VESTA_PATH/RenderContext.cpp \
    $$VESTA_PATH/LightingEnvironment.cpp \
    $$VESTA_PATH/SensorFrustumGeometry.cpp \
    $$VESTA_PATH/SensorVisualizer.cpp \
    $$VESTA_PATH/ShaderBuilder.cpp \
    $$VESTA_PATH/SkyImageLayer.cpp \
    $$VESTA_PATH/Spectrum.cpp \
    $$VESTA_PATH/StarCatalog.cpp \
    $$VESTA_PATH/StarsLayer.cpp \
    $$VESTA_PATH/Submesh.cpp \
    $$VESTA_PATH/TextureFont.cpp \
    $$VESTA_PATH/TextureMap.cpp \
    $$VESTA_PATH/TextureMapLoader.cpp \
    $$VESTA_PATH/TrajectoryGeometry.cpp \
    $$VESTA_PATH/TwoBodyRotatingFrame.cpp \
    $$VESTA_PATH/UniformRotationModel.cpp \
    $$VESTA_PATH/Universe.cpp \
    $$VESTA_PATH/UniverseRenderer.cpp \
    $$VESTA_PATH/VelocityVisualizer.cpp \
    $$VESTA_PATH/VertexArray.cpp \
    $$VESTA_PATH/VertexBuffer.cpp \
    $$VESTA_PATH/VertexPool.cpp \
    $$VESTA_PATH/VertexSpec.cpp \
    $$VESTA_PATH/Visualizer.cpp \
    $$VESTA_PATH/WorldGeometry.cpp \
    $$VESTA_PATH/interaction/ObserverController.cpp \
    $$VESTA_PATH/internal/DefaultFont.cpp \
    $$VESTA_PATH/internal/EclipseShadowVolumeSet.cpp \
    $$VESTA_PATH/internal/InputDataStream.cpp \
    $$VESTA_PATH/internal/OutputDataStream.cpp \
    $$VESTA_PATH/internal/ObjLoader.cpp

VESTA_HEADERS = \
    $$VESTA_PATH/AlignedEllipsoid.h \
    $$VESTA_PATH/Arc.h \
    $$VESTA_PATH/ArrowGeometry.h \
    $$VESTA_PATH/ArrowVisualizer.h \
    $$VESTA_PATH/Atmosphere.h \
    $$VESTA_PATH/AxesVisualizer.h \
    $$VESTA_PATH/BillboardGeometry.h \
    $$VESTA_PATH/Body.h \
    $$VESTA_PATH/BodyDirectionVisualizer.h \
    $$VESTA_PATH/BodyFixedFrame.h \
    $$VESTA_PATH/BoundingBox.h \
    $$VESTA_PATH/BoundingSphere.h \
    $$VESTA_PATH/CelestialCoordinateGrid.h \
    $$VESTA_PATH/Chronology.h \
    $$VESTA_PATH/ConeGeometry.h \
    $$VESTA_PATH/ConstellationsLayer.h \
    $$VESTA_PATH/CubeMapFramebuffer.h \
    $$VESTA_PATH/DataChunk.h \
    $$VESTA_PATH/Debug.h \
    $$VESTA_PATH/DDSLoader.h \
    $$VESTA_PATH/Entity.h \
    $$VESTA_PATH/FadeRange.h \
    $$VESTA_PATH/Frame.h \
    $$VESTA_PATH/Framebuffer.h \
    $$VESTA_PATH/Frustum.h \
    $$VESTA_PATH/FixedPointTrajectory.h \
    $$VESTA_PATH/FixedRotationModel.h \
    $$VESTA_PATH/Geometry.h \
    $$VESTA_PATH/GeometryBuffer.h \
    $$VESTA_PATH/GeneralEllipse.h \
    $$VESTA_PATH/GlareOverlay.h \
    $$VESTA_PATH/GregorianDate.h \
    $$VESTA_PATH/HierarchicalTiledMap.h \
    $$VESTA_PATH/InertialFrame.h \
    $$VESTA_PATH/IntegerTypes.h \
    $$VESTA_PATH/Intersect.h \
    $$VESTA_PATH/JavaCallbackTrajectory.h \
    $$VESTA_PATH/KeplerianTrajectory.h \
    $$VESTA_PATH/LabelGeometry.h \
    $$VESTA_PATH/LabelVisualizer.h \
    $$VESTA_PATH/LightSource.h \
    $$VESTA_PATH/LocalVisualizer.h \
    $$VESTA_PATH/MapLayer.h \
    $$VESTA_PATH/Material.h \
    $$VESTA_PATH/MeshGeometry.h \
    $$VESTA_PATH/NadirVisualizer.h \
    $$VESTA_PATH/Object.h \
    $$VESTA_PATH/Observer.h \
    $$VESTA_PATH/OGLHeaders.h \
    $$VESTA_PATH/OrbitalElements.h \
    $$VESTA_PATH/ParticleSystemGeometry.h \
    $$VESTA_PATH/PickContext.h \
    $$VESTA_PATH/PickResult.h \
    $$VESTA_PATH/PlanarProjection.h \
    $$VESTA_PATH/PlaneGeometry.h \
    $$VESTA_PATH/PlanetaryRings.h \
    $$VESTA_PATH/PlanetGridLayer.h \
    $$VESTA_PATH/PlanetographicCoord.h \
    $$VESTA_PATH/PlaneVisualizer.h \
    $$VESTA_PATH/PrimitiveBatch.h \
    $$VESTA_PATH/QuadtreeTile.h \
    $$VESTA_PATH/RenderContext.h \
    $$VESTA_PATH/LightingEnvironment.h \
    $$VESTA_PATH/RotationModel.h \
    $$VESTA_PATH/SensorFrustumGeometry.h \
    $$VESTA_PATH/SensorVisualizer.h \
    $$VESTA_PATH/ShaderBuilder.h \
    $$VESTA_PATH/ShaderInfo.h \
    $$VESTA_PATH/SingleTextureTiledMap.h \
    $$VESTA_PATH/SkyImageLayer.h \
    $$VESTA_PATH/SkyLayer.h \
    $$VESTA_PATH/Spectrum.h \
    $$VESTA_PATH/StarCatalog.h \
    $$VESTA_PATH/StarsLayer.h \
    $$VESTA_PATH/StateVector.h \
    $$VESTA_PATH/Submesh.h \
    $$VESTA_PATH/TextureFont.h \
    $$VESTA_PATH/TextureMap.h \
    $$VESTA_PATH/TextureMapLoader.h \
    $$VESTA_PATH/TiledMap.h \
    $$VESTA_PATH/Trajectory.h \
    $$VESTA_PATH/TrajectoryGeometry.h \
    $$VESTA_PATH/TwoBodyRotatingFrame.h \
    $$VESTA_PATH/UniformRotationModel.h \
    $$VESTA_PATH/Units.h \
    $$VESTA_PATH/Universe.h \
    $$VESTA_PATH/UniverseRenderer.h \
    $$VESTA_PATH/VelocityVisualizer.h \
    $$VESTA_PATH/VertexArray.h \
    $$VESTA_PATH/VertexAttribute.h \
    $$VESTA_PATH/VertexBuffer.h \
    $$VESTA_PATH/VertexPool.h \
    $$VESTA_PATH/VertexSpec.h \
    $$VESTA_PATH/Viewport.h \
    $$VESTA_PATH/Visualizer.h \
    $$VESTA_PATH/WorldGeometry.h \
    $$VESTA_PATH/interaction/ObserverController.h \
    $$VESTA_PATH/internal/AtomicInt.h \
    $$VESTA_PATH/internal/DefaultFont.h \
    $$VESTA_PATH/internal/EclipseShadowVolumeSet.h \
    $$VESTA_PATH/internal/InputDataStream.h \
    $$VESTA_PATH/internal/OutputDataStream.h \
    $$VESTA_PATH/internal/ObjLoader.h


### particle system module ###

VESTA_SOURCES += \
    $$VESTA_PATH/particlesys/ParticleEmitter.cpp

VESTA_HEADERS += \
    $$VESTA_PATH/particlesys/ParticleEmitter.h \
    $$VESTA_PATH/particlesys/ParticleRenderer.h \
    $$VESTA_PATH/particlesys/PseudorandomGenerator.h \
    $$VESTA_PATH/particlesys/InitialStateGenerator.h \
    $$VESTA_PATH/particlesys/BoxGenerator.h \
    $$VESTA_PATH/particlesys/DiscGenerator.h \
    $$VESTA_PATH/particlesys/PointGenerator.h


### glhelp module ###

VESTA_SOURCES += \
    $$VESTA_PATH/glhelp/GLFramebuffer.cpp \
    $$VESTA_PATH/glhelp/GLShader.cpp \
    $$VESTA_PATH/glhelp/GLShaderProgram.cpp \
    $$VESTA_PATH/glhelp/GLBufferObject.cpp \
    $$VESTA_PATH/glhelp/GLElementBuffer.cpp \
    $$VESTA_PATH/glhelp/GLVertexBuffer.cpp

VESTA_HEADERS += \
    $$VESTA_PATH/glhelp/GLFramebuffer.h \
    $$VESTA_PATH/glhelp/GLShader.h \
    $$VESTA_PATH/glhelp/GLShaderProgram.h \
    $$VESTA_PATH/glhelp/GLBufferObject.h \
    $$VESTA_PATH/glhelp/GLElementBuffer.h \
    $$VESTA_PATH/glhelp/GLVertexBuffer.h


### TLE support

NORADTLE_HEADERS += \
    $$NORADTLE_PATH/norad.h

NORADTLE_SOURCES += \
    $$NORADTLE_PATH/basics.cpp \
    $$NORADTLE_PATH/common.cpp \
    $$NORADTLE_PATH/deep.cpp \
    $$NORADTLE_PATH/get_el.cpp \
    $$NORADTLE_PATH/sdp4.cpp \
    $$NORADTLE_PATH/sdp8.cpp \
    $$NORADTLE_PATH/sgp.cpp \
    $$NORADTLE_PATH/sgp4.cpp \
    $$NORADTLE_PATH/sgp8.cpp


### lib3ds ###

LIB3DS_SOURCES = \
    $$LIB3DS_PATH/lib3ds_atmosphere.c \
    $$LIB3DS_PATH/lib3ds_background.c \
    $$LIB3DS_PATH/lib3ds_camera.c \
    $$LIB3DS_PATH/lib3ds_chunk.c \
    $$LIB3DS_PATH/lib3ds_chunktable.c \
    $$LIB3DS_PATH/lib3ds_file.c \
    $$LIB3DS_PATH/lib3ds_io.c \
    $$LIB3DS_PATH/lib3ds_light.c \
    $$LIB3DS_PATH/lib3ds_material.c \
    $$LIB3DS_PATH/lib3ds_math.c \
    $$LIB3DS_PATH/lib3ds_matrix.c \
    $$LIB3DS_PATH/lib3ds_mesh.c \
    $$LIB3DS_PATH/lib3ds_node.c \
    $$LIB3DS_PATH/lib3ds_quat.c \
    $$LIB3DS_PATH/lib3ds_shadow.c \
    $$LIB3DS_PATH/lib3ds_track.c \
    $$LIB3DS_PATH/lib3ds_util.c \
    $$LIB3DS_PATH/lib3ds_vector.c \
    $$LIB3DS_PATH/lib3ds_viewport.c

LIB3DS_HEADERS = \
    $$LIB3DS_PATH/lib3ds.h \
    $$LIB3DS_PATH/lib3ds_impl.h


### GL extension wrangler ###

GLEW_SOURCES = \
    $$GLEW_PATH/glew.c

GLEW_HEADERS = \
    $$GLEW_PATH/GL/glew.h \
    $$GLEW_PATH/GL/glxew.h \
    $$GLEW_PATH/GL/wglew.h

DEFINES += GLEW_STATIC


### CurvePlot sources ###

CURVEPLOT_SOURCES = \
    thirdparty/curveplot/curveplot.cpp

CURVEPLOT_HEADERS = \
    thirdparty/curveplot/curveplot.h


### QJSON sources ###

QJSON_SOURCES = \
    $$QJSON_PATH/json_parser.cc \
    $$QJSON_PATH/json_scanner.cpp \
    $$QJSON_PATH/parser.cpp \
    $$QJSON_PATH/parserrunnable.cpp \
    $$QJSON_PATH/qobjecthelper.cpp \
    $$QJSON_PATH/serializer.cpp \
    $$QJSON_PATH/serializerrunnable.cpp

QJSON_HEADERS = \
    $$QJSON_PATH/parser.h \
    $$QJSON_PATH/parserrunnable.h \
    $$QJSON_PATH/qobjecthelper.h \
    $$QJSON_PATH/serializer.h \
    $$QJSON_PATH/serializerrunnable.h \
    $$QJSON_PATH/qjson_export.h


### Optional Lua scripting ###

LUA_SOURCES = \
    $$LUA_PATH/lapi.c \
    $$LUA_PATH/lcode.c \
    $$LUA_PATH/lctype.c \
    $$LUA_PATH/ldebug.c \
    $$LUA_PATH/ldo.c \
    $$LUA_PATH/ldump.c \
    $$LUA_PATH/lfunc.c \
    $$LUA_PATH/lgc.c \
    $$LUA_PATH/llex.c \
    $$LUA_PATH/lmem.c \
    $$LUA_PATH/lobject.c \
    $$LUA_PATH/lopcodes.c \
    $$LUA_PATH/lparser.c \
    $$LUA_PATH/lstate.c \
    $$LUA_PATH/lstring.c \
    $$LUA_PATH/ltable.c \
    $$LUA_PATH/ltm.c \
    $$LUA_PATH/lundump.c \
    $$LUA_PATH/lvm.c \
    $$LUA_PATH/lzio.c \
    $$LUA_PATH/lauxlib.c \
    $$LUA_PATH/lbaselib.c \
    $$LUA_PATH/lbitlib.c \
    $$LUA_PATH/lcorolib.c \
    $$LUA_PATH/ldblib.c \
    $$LUA_PATH/liolib.c \
    $$LUA_PATH/lmathlib.c \
    $$LUA_PATH/loslib.c \
    $$LUA_PATH/lstrlib.c \
    $$LUA_PATH/ltablib.c \
    $$LUA_PATH/loadlib.c \
    $$LUA_PATH/linit.c

LUA_HEADERS = \
    $$LUA_PATH/lua.hpp \
    $$LUA_PATH/lapi.h \
    $$LUA_PATH/lualib.h \
    $$LUA_PATH/lauxlib.h \
    $$LUA_PATH/luaconf.h


SOURCES = \
    $$VESTA_SOURCES \
    $$NORADTLE_SOURCES \
    $$LIB3DS_SOURCES \
    $$GLEW_SOURCES \
    $$CURVEPLOT_SOURCES \
    $$QJSON_SOURCES \
    $$APP_SOURCES

HEADERS = \
    $$VESTA_HEADERS \
    $$NORADTLE_HEADERS \
    $$LIB3DS_HEADERS \
    $$GLEW_HEADERS \
    $$CURVEPLOT_HEADERS \
    $$QJSON_HEADERS \
    $$APP_HEADERS

RESOURCES = resources/icons.qrc

INCLUDEPATH += thirdparty/glew thirdparty/curveplot thirdparty

#CONFIG += ffmpeg
#CONFIG += nomenu
#CONFIG += storedeploy
#CONFIG += lua

lua {
    message("Building with Lua scripting support")
    SOURCES += $$LUA_SOURCES
    HEADERS += $$LUA_HEADERS
}

macx {
    # Always enable QTKit support on Mac
    message("Building with QTKit support for video")

    DEFINES += QTKIT_SUPPORT=1
    QMAKE_LFLAGS += -framework QTKit -framework Cocoa

    HEADERS += \
        src/video/VideoEncoder.h

    OBJECTIVE_SOURCES += \
        src/video/VideoEncoder.mm
}

storedeploy {
    QMAKE_CXXFLAGS += -gdwarf-2
    QMAKE_CXXFLAGS += -mmacosx-version-min=10.6
    QMAKE_CFLAGS += -gdwarf-2
    QMAKE_CFLAGS += -mmacosx-version-min=10.6
    QMAKE_INFO_PLIST = resources/Info.plist
    QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.6.sdk
    DEFINES += MAS_DEPLOY
}

nomenu {
    DEFINES += NOMENUBAR=1
}

ffmpeg {
    message("Building with FFMPEG for video")

    # ##############################################################################
    # ##############################################################################
    # FFMPEG: START OF CONFIGURATION BELOW ->
    # Copy these lines into your own project
    # Make sure to set the path variables for:
    # 1) QTFFmpegWrapper sources (i.e. where the QVideoEncoder.cpp and QVideoDecoder.cpp lie),
    # 2) FFMPEG include path (i.e. where the directories libavcodec, libavutil, etc. lie),
    # 3) the binary FFMPEG libraries (that must be compiled separately).
    # Under Linux path 2 and 3 may not need to be set as these are usually in the standard include and lib path.
    # Under Windows, path 2 and 3 must be set to the location where you placed the FFMPEG includes and compiled binaries
    # Note that the FFMPEG dynamic librairies (i.e. the .dll files) must be in the PATH
    # ##############################################################################
    # ##############################################################################
    # ##############################################################################
    # Modify here: set FFMPEG_LIBRARY_PATH and FFMPEG_INCLUDE_PATH
    # ##############################################################################
    # Set QTFFMPEGWRAPPER_SOURCE_PATH to point to the directory containing the QTFFmpegWrapper sources
    QTFFMPEGWRAPPER_SOURCE_PATH = thirdparty/QTFFmpegWrapper/QTFFmpegWrapper

    # Set FFMPEG_LIBRARY_PATH to point to the directory containing the FFmpeg import libraries (if needed - typically for Windows), i.e. the dll.a files
    FFMPEG_LIBRARY_PATH = ../trunk/lib/macosx/ffmpeg

    # Set FFMPEG_INCLUDE_PATH to point to the directory containing the FFMPEG includes (if needed - typically for Windows)
    FFMPEG_INCLUDE_PATH = thirdparty/QTFFmpegWrapper/QTFFmpegWrapper

    # ##############################################################################
    # Do not modify: FFMPEG default settings
    # ##############################################################################
    # Sources for QT wrapper
    SOURCES += $$QTFFMPEGWRAPPER_SOURCE_PATH/QVideoEncoder.cpp \
        $$QTFFMPEGWRAPPER_SOURCE_PATH/QVideoDecoder.cpp
    HEADERS += $$QTFFMPEGWRAPPER_SOURCE_PATH/QVideoEncoder.h \
        $$QTFFMPEGWRAPPER_SOURCE_PATH/QVideoDecoder.h

    # Set list of required FFmpeg libraries
    LIBS += \
        -lavutil \
        -lavcodec \
        -lavformat \
        -lswscale \
        -lz \
        -lbz2

    # Add the path
    LIBS += -L$$FFMPEG_LIBRARY_PATH
    INCLUDEPATH += QVideoEncoder
    INCLUDEPATH += $$FFMPEG_INCLUDE_PATH

    DEFINES += FFMPEG_SUPPORT=1

    # ##############################################################################
    # FFMPEG: END OF CONFIGURATION
    # ##############################################################################
}

DEFINES += EIGEN_USE_NEW_STDVECTOR
DEFINES += QJSON_EXPORT=

win32-g++ {
    # Work around alignment problems with MinGW. Fixed-size Eigen matrices
    # are sometimes allocated on the stack at unaligned addresses even though
    # __alignof e.g. Vector4d is 16.
    DEFINES += EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT
}

win32 {
    DEFINES += NOMINMAX
}

win32-msvc2008|win32-msvc2010 {
    # Disable MSVC's warnings about some standard C and C++ library
    # functions.
    DEFINES += _SCL_SECURE_NO_WARNINGS _CRT_SECURE_NO_WARNINGS

    # Necessary to avoid linker warnings when not building lib3ds as a library
    DEFINES += LIB3DSAPI=" "
}

macx {
    ICON = resources/cosmographia.icns

    # Media files for the Mac bundle
    TEXTURES.path = Contents/Resources/data/textures
    TEXTURES.files = \
        data/textures/README.html \
        data/textures/flare.png \
        data/textures/gaussian.jpg \
        data/textures/milkyway.jpg \
        data/textures/moon-clementine_0_0_0.jpg \
        data/textures/moon-clementine_0_1_0.jpg \
        data/textures/mercury.dds \
        data/textures/venus.dds \
        data/textures/venus-clouds.jpg \
        data/textures/jupiter.dds \
        data/textures/saturn.jpg \
        data/textures/uranus.jpg \
        data/textures/neptune.jpg \
        data/textures/sun.jpg \
        data/textures/saturn-rings.png \
        data/textures/uranus-rings.png \
        data/textures/io.dds \
        data/textures/europa.dds \
        data/textures/ganymede.dds \
        data/textures/callisto.dds \
        data/textures/mimas.dds \
        data/textures/enceladus.dds \
        data/textures/dione.dds \
        data/textures/tethys.dds \
        data/textures/rhea.dds \
        data/textures/titan.dds \
        data/textures/iapetus.dds \
        data/textures/miranda.dds \
        data/textures/ariel.dds \
        data/textures/umbriel.dds \
        data/textures/titania.dds \
        data/textures/oberon.dds \
        data/textures/triton.dds \
        data/textures/pluto.png \
        data/textures/charon.png \
        data/textures/ceres.png \
        data/textures/earth-clouds.dds

    QMAKE_BUNDLE_DATA += TEXTURES

    EARTHTEXTURES.path = Contents/Resources/data/textures/earth
    EARTHTEXTURES.files = \
        data/textures/earth/bmng-jan-nb_0_0_0.jpg \
        data/textures/earth/bmng-jan-nb_0_1_0.jpg \
        data/textures/earth/bmng-feb-nb_0_0_0.jpg \
        data/textures/earth/bmng-feb-nb_0_1_0.jpg \
        data/textures/earth/bmng-mar-nb_0_0_0.jpg \
        data/textures/earth/bmng-mar-nb_0_1_0.jpg \
        data/textures/earth/bmng-apr-nb_0_0_0.jpg \
        data/textures/earth/bmng-apr-nb_0_1_0.jpg \
        data/textures/earth/bmng-may-nb_0_0_0.jpg \
        data/textures/earth/bmng-may-nb_0_1_0.jpg \
        data/textures/earth/bmng-jun-nb_0_0_0.jpg \
        data/textures/earth/bmng-jun-nb_0_1_0.jpg \
        data/textures/earth/bmng-jul-nb_0_0_0.jpg \
        data/textures/earth/bmng-jul-nb_0_1_0.jpg \
        data/textures/earth/bmng-aug-nb_0_0_0.jpg \
        data/textures/earth/bmng-aug-nb_0_1_0.jpg \
        data/textures/earth/bmng-sep-nb_0_0_0.jpg \
        data/textures/earth/bmng-sep-nb_0_1_0.jpg \
        data/textures/earth/bmng-oct-nb_0_0_0.jpg \
        data/textures/earth/bmng-oct-nb_0_1_0.jpg \
        data/textures/earth/bmng-nov-nb_0_0_0.jpg \
        data/textures/earth/bmng-nov-nb_0_1_0.jpg \
        data/textures/earth/bmng-dec-nb_0_0_0.jpg \
        data/textures/earth/bmng-dec-nb_0_1_0.jpg

    QMAKE_BUNDLE_DATA += EARTHTEXTURES

    MODELS.path = Contents/Resources/data/models
    MODELS.files = \
        data/models/README.html \
        data/models/phobos.cmod \
        data/models/phobos-normals.dds \
        data/models/deimos.cmod \
        data/models/deimos.dds \
        data/models/2pallas.cmod \
        data/models/3juno.cmod \
        data/models/4vesta.cmod \
        data/models/vesta-normals.dds \
        data/models/5astraea.cmod \
        data/models/8flora.cmod \
        data/models/10hygiea.cmod \
        data/models/21lutetia.cmod \
        data/models/243ida.cmod \
        data/models/ida.dds \
        data/models/433eros.cmod \
        data/models/eros-normals.dds \
        data/models/511davida.cmod \
        data/models/25143itokawa.cmod \
        data/models/itokawa-normals.dds \
        data/models/amalthea.cmod \
        data/models/hyperion.cmod \
        data/models/phoebe.cmod \
        data/models/phoebe-normals.dds \
        data/models/proteus.cmod \
        data/models/jason.obj \
        data/models/jason.mtl \
        data/models/jas_solr.png \
        data/models/jas_brsh.png \
        data/models/cassini.cmod \
        data/models/huygens.cmod \
        data/models/foil_n_2.dds \
        data/models/cassini-dish.dds \
        data/models/cassini-tex-1.dds \
        data/models/galileo.cmod \
        data/models/galileo_tex_01.dds \
        data/models/galileo_tex_02.dds \
        data/models/voyager.cmod \
        data/models/voyager-tex1.dds \
        data/models/voyager-tex2.dds \
        data/models/pioneer10.cmod \
        data/models/pioneer-box_n.dds \
        data/models/pioneer-dish.dds \
        data/models/pioneer-tex-1.dds \
        data/models/pioneer-tex-2.dds \
        data/models/pioneer-tex-black.dds \
        data/models/near.cmod \
        data/models/near-tex-01.dds \
        data/models/foil_n.dds \
        data/models/hst.cmod \
        data/models/hbltel_1.jpg \
        data/models/hbltel_2.jpg \
        data/models/hbltel_3.jpg \
        data/models/hbltel_4.jpg \
        data/models/hbltel_w.jpg \
        data/models/msl-cruise.cmod \
        data/models/msl-solar.dds

    QMAKE_BUNDLE_DATA += MODELS

    ISSFILES.path = Contents/Resources/data/models/iss
    ISSFILES.files = \
        data/models/iss/iss.cmod \
        data/models/iss/d_ring.jpg \
        data/models/iss/graple.jpg \
        data/models/iss/iss_dc.jpg \
        data/models/iss/iss_dcs.jpg \
        data/models/iss/issb.jpg \
        data/models/iss/issb2.jpg \
        data/models/iss/issb3.jpg \
        data/models/iss/issb4.jpg \
        data/models/iss/issbso.jpg \
        data/models/iss/issbso2.jpg \
        data/models/iss/isscov2.jpg \
        data/models/iss/isscover.jpg \
        data/models/iss/isscup.jpg \
        data/models/iss/issdish.jpg \
        data/models/iss/issdot.jpg \
        data/models/iss/isshand.jpg \
        data/models/iss/issins.jpg \
        data/models/iss/issku.jpg \
        data/models/iss/issku1.jpg \
        data/models/iss/issku2.jpg \
        data/models/iss/issmod.jpg \
        data/models/iss/issp2.jpg \
        data/models/iss/isspanel.jpg \
        data/models/iss/issrad.jpg \
        data/models/iss/issred.jpg \
        data/models/iss/isssolar.jpg \
        data/models/iss/isssolar.png \
        data/models/iss/isszmod.jpg \
        data/models/iss/metalcon.jpg \
        data/models/iss/questcov.jpg

    QMAKE_BUNDLE_DATA += ISSFILES

    DAWNFILES.path = Contents/Resources/data/models/dawn
    DAWNFILES.files = \
        data/models/dawn/dawn.cmod \
        data/models/dawn/awsup.jpg \
        data/models/dawn/dawn_ant.jpg \
        data/models/dawn/dawn_foil.jpg \
        data/models/dawn/dawn_louvre.jpg \
        data/models/dawn/pb1.jpg \
        data/models/dawn/cv2.jpg \
        data/models/dawn/dawn_arm.jpg \
        data/models/dawn/dawn_gold.jpg \
        data/models/dawn/dawn_silver.jpg \
        data/models/dawn/dawn_eng.jpg \
        data/models/dawn/dawn_grid.jpg \
        data/models/dawn/dawn_sp.jpg

    QMAKE_BUNDLE_DATA += DAWNFILES

    TRAJECTORIES.path = Contents/Resources/data/trajectories
    TRAJECTORIES.files = \
        data/trajectories/cassini-cruise.xyzv \
        data/trajectories/cassini-orbit.xyzv \
        data/trajectories/cassini-solstice.xyzv \
        data/trajectories/dawn-cruise1.xyzv \
        data/trajectories/dawn-vesta-orbit.xyzv \
        data/trajectories/dawn-cruise2.xyzv \
        data/trajectories/galileo-cruise.xyzv \
        data/trajectories/galileo-orbit.xyzv \
        data/trajectories/voyager1.xyzv \
        data/trajectories/voyager2.xyzv \
        data/trajectories/pioneer10.xyzv \
        data/trajectories/pioneer11.xyzv \
        data/trajectories/near-cruise.xyzv \
        data/trajectories/near-eros-orbit.xyzv \
        data/trajectories/msl-cruise.xyzv \
        data/trajectories/msl-edl.xyzv

    QMAKE_BUNDLE_DATA += TRAJECTORIES

    DATA.path = Contents/Resources/data
    DATA.files = \
        data/tycho2.stars \
        data/starnames.json \
        data/solarsys.json \
        data/sun.json \
        data/mercury.json \
        data/venus.json \
        data/earth.json \
        data/mars.json \
        data/jupiter.json \
        data/saturn.json \
        data/uranus.json \
        data/neptune.json \
        data/pluto.json \
        data/mainbelt.json \
        data/nearearth.json \
        data/kuiperbelt.json \
        data/scattereddisc.json \
        data/cassini.json \
        data/galileo.json \
        data/voyager.json \
        data/pioneer.json \
        data/dawn.json \
        data/near.json \
        data/earthorbiting.json \
        data/iss.json \
        data/hst.json \
        data/msl.json \
        data/mercury-features.json \
        data/venus-features.json \
        data/moon-features.json \
        data/mars-features.json \
        data/jupiter-features.json \
        data/saturn-features.json \
        data/uranus-features.json \
        data/neptune-features.json \
        data/start-viewpoints.json \
        data/sans-12.txf \
        data/sans-24.txf \
        data/csans-14.txf \
        data/csans-16.txf \
        data/csans-28.txf \
        data/sans-light-24.txf \
        data/de406_1800-2100.dat \
        data/enceladus.cheb \
        data/dione.cheb \
        data/phoebe.cheb \
        data/saturn.cheb \
        data/earth.atmscat \
        data/mars.atmscat \
        data/titan.atmscat \
        data/addons.js

    QMAKE_BUNDLE_DATA += DATA

    GUI.path = Contents/Resources/data/qml
    GUI.files = \
        data/qml/main.qml \
        data/qml/AddonManager.qml \
        data/qml/Button.qml \
        data/qml/CheckBox.qml \
        data/qml/ChoiceBox.qml \
        data/qml/ContextMenu.qml \
        data/qml/DistancePanel.qml \
        data/qml/FindObjectPanel.qml \
        data/qml/HelpBrowser.qml \
        data/qml/InfoText.qml \
        data/qml/PanelRectangle.qml \
        data/qml/PanelText.qml \
        data/qml/ScrollablePane.qml \
        data/qml/SettingsPanel.qml \
        data/qml/Slider.qml \
        data/qml/SpinBox.qml \
        data/qml/TabWidget.qml \
        data/qml/TextButton.qml \
        data/qml/TextPanel.qml \
        data/qml/TextToggle.qml \
        data/qml/TimePanel.qml \
        data/qml/TimeRateControl.qml \
        data/qml/LinkStack.js

    QMAKE_BUNDLE_DATA += GUI

    HELPFILES.path = Contents/Resources/data/help
    HELPFILES.files = \
        data/help/help.html \
        data/help/intro.html \
        data/help/keyboard.html \
        data/help/navigation.html \
        data/help/settings.html \
        data/help/properties.html \
        data/help/solarsysguide.html \
        data/help/how.html \
        data/help/code.html \
        data/help/datasources.html \
        data/help/announcement.html \
        data/help/sun.html \
        data/help/mercury.html \
        data/help/venus.html \
        data/help/earth.html \
        data/help/moon.html \
        data/help/mars.html \
        data/help/phobos.html \
        data/help/deimos.html \
        data/help/jupiter.html \
        data/help/amalthea.html \
        data/help/io.html \
        data/help/europa.html \
        data/help/galileo.html \
        data/help/ganymede.html \
        data/help/callisto.html \
        data/help/saturn.html \
        data/help/mimas.html \
        data/help/enceladus.html \
        data/help/tethys.html \
        data/help/dione.html \
        data/help/rhea.html \
        data/help/hyperion.html \
        data/help/titan.html \
        data/help/iapetus.html \
        data/help/phoebe.html \
        data/help/uranus.html \
        data/help/miranda.html \
        data/help/ariel.html \
        data/help/umbriel.html \
        data/help/titania.html \
        data/help/oberon.html \
        data/help/neptune.html \
        data/help/triton.html \
        data/help/proteus.html \
        data/help/nereid.html \
        data/help/ceres.html \
        data/help/pallas.html \
        data/help/juno.html \
        data/help/vesta.html \
        data/help/astraea.html \
        data/help/flora.html \
        data/help/hygiea.html \
        data/help/lutetia.html \
        data/help/ida.html \
        data/help/dactyl.html \
        data/help/eros.html \
        data/help/davida.html \
        data/help/itokawa.html \
        data/help/pluto.html \
        data/help/charon.html \
        data/help/nix.html \
        data/help/hydra.html \
        data/help/p4.html \
        data/help/eris.html \
        data/help/haumea.html \
        data/help/orcus.html \
        data/help/varuna.html \
        data/help/ixion.html \
        data/help/makemake.html \
        data/help/cassini.html \
        data/help/voyager1.html \
        data/help/voyager2.html \
        data/help/pioneer10.html \
        data/help/pioneer11.html \
        data/help/dawn.html \
        data/help/near.html \
        data/help/hubble.html \
        data/help/iss.html \
        data/help/galileo.html \
        data/help/msl.html \
        data/help/jupiter.jpg \
        data/help/PIA01971.jpg \
        data/help/hyperion.jpg \
        data/help/iapetus-global.jpg \
        data/help/miranda.jpg \
        data/help/vesta_dawn_20110714.jpg \
        data/help/lutetia.jpg

    QMAKE_BUNDLE_DATA += HELPFILES

    ADDONS.path = Contents/Resources/data/addons
    ADDONS.files = \
        data/addons/aurora.json \
        data/addons/gaussian.jpg \
        data/addons/tvashtar.json \
        data/addons/enceladus-jets.json \
        data/addons/asteroids.json \
        data/addons/astorb_5000.dat \
        data/addons/allasteroids.json \
        data/addons/allasteroids.dat \
        data/addons/hildas.json \
        data/addons/hildas.dat \
        data/addons/gps.json \
        data/addons/2012DA14.json \
        data/addons/2012DA14.xyzv \
        data/addons/2012da14.html \
        data/addons/1998wt24.cmod \
        data/addons/2011MD.json \
        data/addons/2011MD.xyzv \
        data/addons/2008TC3.json \
        data/addons/2008TC3.xyzv \
        data/addons/2008tc3.html \
        data/addons/sciencesat.json \
        data/addons/sorce.obj \
        data/addons/sorce.mtl \
        data/addons/solar_panel.png \
        data/addons/debris.json \
        data/addons/iridium33.kep \
        data/addons/cosmos2251.kep

    QMAKE_BUNDLE_DATA += ADDONS

    GALLERY.path = Contents/Resources/data/gallery
    GALLERY.files = \
        data/gallery/gallery.json \
        data/gallery/sun.png \
        data/gallery/mercury.png \
        data/gallery/venus.png \
        data/gallery/earth.png \
        data/gallery/moon.png \
        data/gallery/mars.png \
        data/gallery/phobos.png \
        data/gallery/deimos.png \
        data/gallery/jupiter.png \
        data/gallery/io.png \
        data/gallery/europa.png \
        data/gallery/ganymede.png \
        data/gallery/callisto.png \
        data/gallery/amalthea.png \
        data/gallery/saturn.png \
        data/gallery/mimas.png \
        data/gallery/enceladus.png \
        data/gallery/tethys.png \
        data/gallery/dione.png \
        data/gallery/rhea.png \
        data/gallery/hyperion.png \
        data/gallery/titan.png \
        data/gallery/iapetus.png \
        data/gallery/phoebe.png \
        data/gallery/uranus.png \
        data/gallery/miranda.png \
        data/gallery/ariel.png \
        data/gallery/umbriel.png \
        data/gallery/titania.png \
        data/gallery/oberon.png \
        data/gallery/neptune.png \
        data/gallery/triton.png \
        data/gallery/proteus.png \
        data/gallery/pluto.png \
        data/gallery/ceres.png \
        data/gallery/vesta.png \
        data/gallery/lutetia.png \
        data/gallery/ida.png \
        data/gallery/eros.png \
        data/gallery/itokawa.png

    QMAKE_BUNDLE_DATA += GALLERY

    # Scan directories for files for Mac bundle
    MARS_TILES_DIR = data/textures/mars
    FILES = $$system(ls $$MARS_TILES_DIR)
    MARS_TILES = $$join(FILES, " $$MARS_TILES_DIR/", $$MARS_TILES_DIR/)

    MARS_TEXTURE.path = Contents/Resources/data/textures/mars
    MARS_TEXTURE.files = $$MARS_TILES

    # Add all texture tiles
    QMAKE_BUNDLE_DATA += \
        MARS_TEXTURE

    QMAKE_LFLAGS += -framework CoreFoundation
}
