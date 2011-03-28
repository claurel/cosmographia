#
# $Revision: 581 $ $Date: 2011-03-18 18:02:35 -0700 (Fri, 18 Mar 2011) $
#
# Copyright by Astos Solutions GmbH, Germany
#
# this file is published under the Astos Solutions Free Public License
# For details on copyright and terms of use see 
# http://www.astos.de/Astos_Solutions_Free_Public_License.html
#
#
# CMake file for building the vesta library

FIND_PACKAGE(SWIG)
INCLUDE(${SWIG_USE_FILE})

FIND_PACKAGE(JNI REQUIRED)
#FIND_PACKAGE(PythonLibs REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)

SET_SOURCE_FILES_PROPERTIES(vesta.swg PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(vesta.swg PROPERTIES SWIG_FLAGS "-includeall")

add_definitions(
     -DEIGEN_USE_NEW_STDVECTOR
     -DGLEW_STATIC
     -DLIB3DSAPI=
)

set (CMAKE_OSX_SYSROOT /Developer/SDKs/MacOSX10.6.sdk)

set (SRCS 
    AlignedEllipsoid.cpp
    Arc.cpp
    ArrowGeometry.cpp
    ArrowVisualizer.cpp
    Atmosphere.cpp
    AxesVisualizer.cpp
    BillboardGeometry.cpp
    Body.cpp
    BodyDirectionVisualizer.cpp
    BodyFixedFrame.cpp
    CelestialCoordinateGrid.cpp
    Chronology.cpp
    ConeGeometry.cpp
    ConstellationsLayer.cpp
    CubeMapFramebuffer.cpp
    DataChunk.cpp
    DDSLoader.cpp
    Debug.cpp
    Entity.cpp
    FixedPointTrajectory.cpp
    FixedRotationModel.cpp
    Frame.cpp
    Framebuffer.cpp
    GeneralEllipse.cpp
    Geometry.cpp
    GregorianDate.cpp
    HierarchicalTiledMap.cpp
    InertialFrame.cpp
    KeplerianTrajectory.cpp
    LabelGeometry.cpp
    LightingEnvironment.cpp
    LightSource.cpp
    MapLayer.cpp
    MeshGeometry.cpp
    NadirVisualizer.cpp
    Observer.cpp
    OrbitalElements.cpp
    ParticleSystemGeometry.cpp
    PlanarProjection.cpp
    PlaneGeometry.cpp
    PlanetaryRings.cpp
    PlanetGridLayer.cpp
    PlaneVisualizer.cpp
    PrimitiveBatch.cpp
    QuadtreeTile.cpp
    RenderContext.cpp
    SensorFrustumGeometry.cpp
    SensorVisualizer.cpp
    ShaderBuilder.cpp
    SkyImageLayer.cpp
    Spectrum.cpp
    StarCatalog.cpp
    StarsLayer.cpp
    Submesh.cpp
    TextureFont.cpp
    TextureMap.cpp
    TextureMapLoader.cpp
    TileBorderLayer.cpp
    TrajectoryGeometry.cpp
    TwoBodyRotatingFrame.cpp
    UniformRotationModel.cpp
    Universe.cpp
    UniverseRenderer.cpp
    VelocityVisualizer.cpp
    VertexArray.cpp
    VertexBuffer.cpp
    VertexPool.cpp
    VertexSpec.cpp
    Visualizer.cpp
    WorldGeometry.cpp
    internal/EclipseShadowVolumeSet.cpp
    internal/InputDataStream.cpp
    internal/OutputDataStream.cpp
    internal/ObjLoader.cpp
    particlesys/ParticleEmitter.cpp
    interaction/ObserverController.cpp
    glhelp/GLShader.cpp
    glhelp/GLShaderProgram.cpp
    glhelp/GLFramebuffer.cpp
    glhelp/GLBufferObject.cpp
    glhelp/GLVertexBuffer.cpp
    glhelp/GLElementBuffer.cpp
)

set (LIB3DSDIR ${VESTA_SOURCE_DIR}/libraries/src/lib3ds )

set (LIB3DS_SRCS 
    ${LIB3DSDIR}/lib3ds_atmosphere.c
    ${LIB3DSDIR}/lib3ds_background.c
    ${LIB3DSDIR}/lib3ds_camera.c
    ${LIB3DSDIR}/lib3ds_chunk.c
    ${LIB3DSDIR}/lib3ds_chunktable.c
    ${LIB3DSDIR}/lib3ds_file.c
    ${LIB3DSDIR}/lib3ds_io.c
    ${LIB3DSDIR}/lib3ds_light.c
    ${LIB3DSDIR}/lib3ds_material.c
    ${LIB3DSDIR}/lib3ds_math.c
    ${LIB3DSDIR}/lib3ds_matrix.c
    ${LIB3DSDIR}/lib3ds_mesh.c
    ${LIB3DSDIR}/lib3ds_node.c
    ${LIB3DSDIR}/lib3ds_quat.c
    ${LIB3DSDIR}/lib3ds_shadow.c
    ${LIB3DSDIR}/lib3ds_track.c
    ${LIB3DSDIR}/lib3ds_util.c
    ${LIB3DSDIR}/lib3ds_vector.c
    ${LIB3DSDIR}/lib3ds_viewport.c
)

set (GLEWDIR ${VESTA_SOURCE_DIR}/libraries/src/glew )

set (GLEW_SRCS
    ${GLEWDIR}/src/glew.c
)

set (CURVEPLOTDIR ${VESTA_SOURCE_DIR}/libraries/src/curveplot )

set (CURVEPLOT_SRCS
    ${CURVEPLOTDIR}/curveplot.cpp
)


INCLUDE_DIRECTORIES (${VESTA_SOURCE_DIR}/libraries/inc)
INCLUDE_DIRECTORIES (${VESTA_SOURCE_DIR}/libraries/src)
INCLUDE_DIRECTORIES (${VESTA_SOURCE_DIR}/libraries/src/glew/include)
INCLUDE_DIRECTORIES (${VESTA_SOURCE_DIR}/libraries/inc/curveplot)

#INCLUDE_DIRECTORIES (${PYTHON_INCLUDE_PATH})
INCLUDE_DIRECTORIES (${JNI_INCLUDE_DIRS})

set (LIBS ${LIBS} ${OPENGL_LIBRARIES})

add_library (vestacore STATIC ${SRCS} ${LIB3DS_SRCS} ${GLEW_SRCS} ${CURVEPLOT_SRCS})
target_link_libraries(vestacore ${LIBS})

set (CMAKE_SWIG_OUTDIR wrappers)
#SWIG_ADD_MODULE(vesta python vesta.swg vesta_wrap.cxx)
SWIG_ADD_MODULE(vesta java vesta.swg vesta_wrap.cxx)
SWIG_LINK_LIBRARIES(vesta vestacore
# ${PYTHON_LIBRARIES}
)
