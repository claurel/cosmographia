//
// $Revision: 603 $ $Date: 2011-04-13 17:06:42 -0700 (Wed, 13 Apr 2011) $
//
// Copyright by Astos Solutions GmbH, Germany
//  
// this file is published under the Astos Solutions Free Public License
// For details on copyright and terms of use see 
// http://www.astos.de/Astos_Solutions_Free_Public_License.html
//

%module(directors="1") vesta

%feature("ref")   Object "$this->addRef();"
%feature("unref") Object "$this->release();"

%include "std_string.i"
%include "std_vector.i"

#ifdef SWIGJAVA
%include "arrays_java.i"
%apply signed char[] {signed char *}

// Special handling for the two-argument pattern:
//    const signed char*, unsigned int byteArrayLength
// 
// We want to be able to pass data to VESTA as Java byte arrays. The following
// typemaps will cause Java methods with the signature f(byte[] data) to
// result in calls to C++ methods with a signed char pointer and length.
// This way, the Java caller doesn't need to explicitly pass the length,
// thereby making the interface more safe and convenient.
%typemap(in) (const signed char* byteArray, unsigned int byteArrayLength)
{
    $2 = jenv->GetArrayLength($input);
    $1 = jenv->GetByteArrayElements($input, NULL);
}

// Eliminate the code to copy values back to the Java array. In VESTA, byte
// arrays parameters are always input-only.
%typemap(argout) (const signed char* byteArray, unsigned int byteArrayLength)
{
}

// Release any memory allocated by GetByteArrayElements (which may or may
// not create a copy of the original data.)
%typemap(freearg) (const signed char* byteArray, unsigned int byteArrayLength)
{
     jenv->ReleaseByteArrayElements($input, $1, 0);
}

#endif

%{

#include "StateVector.h"
#include "OrbitalElements.h"
#include "Spectrum.h"

#include "Trajectory.h"
#include "RotationModel.h"
#include "Frame.h"
#include "Arc.h"
#include "Chronology.h"
#include "Entity.h"
#include "Body.h"
#include "Universe.h"
#include "Observer.h"
#include "StarCatalog.h"
#include "UniverseRenderer.h"
#include "GlareOverlay.h"
#include "Atmosphere.h"

// Utility classes
#include "GregorianDate.h"
#include "DataChunk.h"
#include "PlanarProjection.h"
#include "Viewport.h"
#include "LightingEnvironment.h"

#include "Framebuffer.h"
#include "CubeMapFramebuffer.h"

// Geometry classes
#include "Geometry.h"
#include "WorldGeometry.h"
#include "MeshGeometry.h"
#include "PlaneGeometry.h"
#include "LabelGeometry.h"
#include "ArrowGeometry.h"
#include "ConeGeometry.h"
#include "TrajectoryGeometry.h"
#include "ParticleSystemGeometry.h"
#include "BillboardGeometry.h"
#include "PlanetaryRings.h"

#include "MapLayer.h"
#include "WorldLayer.h"
#include "PlanetGridLayer.h"

#include "FixedPointTrajectory.h"
#include "KeplerianTrajectory.h"

#include "FixedRotationModel.h"
#include "UniformRotationModel.h"

#include "InertialFrame.h"
#include "BodyFixedFrame.h"
#include "TwoBodyRotatingFrame.h"

#include "TextureMapLoader.h"
#include "TextureFont.h"
#include "DDSLoader.h"

// Visualizer classes
#include "ArrowVisualizer.h"
#include "AxesVisualizer.h"
#include "VelocityVisualizer.h"
#include "NadirVisualizer.h"
#include "BodyDirectionVisualizer.h"
#include "PlaneVisualizer.h"
#include "SensorVisualizer.h"

// Sky layers
#include "CelestialCoordinateGrid.h"
#include "StarsLayer.h"
#include "SkyImageLayer.h"

#include "TiledMap.h"
#include "HierarchicalTiledMap.h"

#include "RenderContext.h"

#include "particlesys/ParticleEmitter.h"
#include "particlesys/InitialStateGenerator.h"
#include "particlesys/DiscGenerator.h"
#include "particlesys/BoxGenerator.h"
#include "particlesys/PointGenerator.h"

#include "interaction/ObserverController.h"

%}

%{

// Definition of callback classes for trajectories and rotation models.
// These are designed to be subclassed in the scripting language.
// 
// Note: we could allow the base Trajectory and RotationModel classes
// to be subclasses directly, but this may incur additional overhead
// for the native trajectories and rotation models.

namespace vesta
{

class CallbackTrajectory : public Trajectory
{
public:
    CallbackTrajectory() {}

    virtual StateVector state(double t) const
    {
        return StateVector();
    }

    virtual double boundingSphereRadius() const
    {
        return 0.0;
    }
};

class CallbackRotationModel : public RotationModel
{
public:
    CallbackRotationModel() {}

    virtual Eigen::Quaterniond orientation(double t) const
    {
        return Eigen::Quaterniond::Identity();
    }

    virtual Eigen::Vector3d angularVelocity(double t) const
    {
        return Eigen::Vector3d::Zero();
    }
};

}

%}

// Basic wrappers for Eigen objects used in the VESTA interface
namespace Eigen
{

class Vector3d
{
public:
    Vector3d();
    Vector3d(double x, double y, double z);
    double x() const;
    double y() const;
    double z() const;
    double norm() const;
    Eigen::Vector3d normalized() const;

    Vector3d operator+(const Vector3d& v);
    Vector3d operator-(const Vector3d& v);
    Vector3d operator*(double s);

    %extend {
    // For languages without operator overloading...

    Eigen::Vector3d add(const Vector3d& v) const
    {
        return *$self + v;
    }

    Eigen::Vector3d sub(const Vector3d& v) const
    {
        return *$self - v;
    }

    Eigen::Vector3d mul(double s) const
    {
        return *$self * s;
    }

    }

    %extend {
    
    double dot(const Vector3d& v) const
    {
        return $self->dot(v);
    }

    Vector3d cross(const Vector3d& v) const
    {
        return $self->cross(v);
    }

    }

};

class Vector3f
{
public:
    Vector3f();
    Vector3f(float x, float y, float z);
    float x() const;
    float y() const;
    float z() const;
    float norm() const;
    Eigen::Vector3f normalized() const;

    Vector3f operator+(const Vector3f& v);
    Vector3f operator-(const Vector3f& v);
    Vector3f operator*(float s);

    %extend {
    // For languages without operator overloading...

    Eigen::Vector3f add(const Vector3f& v) const
    {
        return *$self + v;
    }

    Eigen::Vector3f sub(const Vector3f& v) const
    {
        return *$self - v;
    }

    Eigen::Vector3f mul(float s) const
    {
        return *$self * s;
    }

    }

};


class Vector2d
{
public:
    Vector2d();
    Vector2d(double x, double y);
    double x() const;
    double y() const;
    double norm() const;
    Eigen::Vector2d normalized() const;

    Vector2d operator+(const Vector2d& v);
    Vector2d operator-(const Vector2d& v);
    Vector2d operator*(double s);

    %extend {
    // For languages without operator overloading...

    Eigen::Vector2d add(const Vector2d& v) const
    {
        return *$self + v;
    }

    Eigen::Vector2d sub(const Vector2d& v) const
    {
        return *$self - v;
    }

    Eigen::Vector2d mul(double s) const
    {
        return *$self * s;
    }

    }

};

class Vector2f
{
public:
    Vector2f();
    Vector2f(float x, float y);
    float x() const;
    float y() const;
    float norm() const;
    Eigen::Vector2f normalized() const;

    Vector2f operator+(const Vector2f& v);
    Vector2f operator-(const Vector2f& v);
    Vector2f operator*(float s);

    %extend {
    // For languages without operator overloading...

    Eigen::Vector2f add(const Vector2f& v) const
    {
        return *$self + v;
    }

    Eigen::Vector2f sub(const Vector2f& v) const
    {
        return *$self - v;
    }

    Eigen::Vector2f mul(float s) const
    {
        return *$self * s;
    }

    }

};

class Quaterniond
{
public:
    Quaterniond();
    Quaterniond operator*(const Quaterniond& q);
    Quaterniond conjugate() const;
    Quaterniond normalized() const;
    void normalize();
    Quaterniond setFromTwoVectors(const Eigen::Vector3d& a, const Eigen::Vector3d& b);
    Quaterniond slerp(double t, Quaterniond other) const;

    %extend {
    // For languages without operator overloading...
    Quaterniond mul(const Quaterniond& q) const
    {
        return *$self * q;
    }
    Vector3d mul(const Vector3d& v) const
    {
        return *$self * v;
    }    
    }

    %extend {
    static Eigen::Quaterniond xRotation(double radians)
    {
        return Eigen::Quaterniond(Eigen::AngleAxisd(radians, Eigen::Vector3d::UnitX()));
    }

    static Eigen::Quaterniond yRotation(double radians)
    {
        return Eigen::Quaterniond(Eigen::AngleAxisd(radians, Eigen::Vector3d::UnitY()));
    }

    static Eigen::Quaterniond zRotation(double radians)
    {
        return Eigen::Quaterniond(Eigen::AngleAxisd(radians, Eigen::Vector3d::UnitZ()));
    }

    static Eigen::Quaterniond angleAxis(double angle, const Eigen::Vector3d& axis)
    {
        return Eigen::Quaterniond(Eigen::AngleAxisd(angle, axis));
    }
    }
};

}


namespace vesta
{
class Entity;
class LightSource;
class Geometry;
class TextureMap;
class Object;
class Visualizer;
class MapLayerBounds;
class SkyLayer;
class WorldLayer;

class StateVector
{
public:
    StateVector();            
    StateVector(const Eigen::Vector3d& position, const Eigen::Vector3d& velocity);
    Eigen::Vector3d position() const;
    Eigen::Vector3d velocity() const;
};


class OrbitalElements
{
public:
    OrbitalElements();

    double periapsisDistance;         // Distance in kilometers
    double eccentricity;
    double inclination;               // Angle in radians
    double longitudeOfAscendingNode;  // Angle in radians
    double argumentOfPeriapsis;       // Angle in radians
    double meanAnomalyAtEpoch;        // Angle in radians
    double meanMotion;                // Radians per second
    double epoch;                     // Time in seconds past J2000
};


class Spectrum
{
public:
    Spectrum();
    Spectrum(float r, float g, float b);

    %extend {
    // For languages without operator overloading...

    float red() const
    {
        return $self->data()[0];
    }

    float green() const
    {
        return $self->data()[1];
    }

    float blue() const
    {
        return $self->data()[2];
    }

    }
};


%nodefaultctor Object;

class Object
{
    int addRef();
    int release();
    int refCount() const;    
};


class Trajectory : public Object
{
public:
    Trajectory() {}
    virtual ~Trajectory() {}
    virtual StateVector state(double t) const = 0;
    virtual double boundingSphereRadius() const = 0;
    virtual Eigen::Vector3d position(double t) const;
    virtual Eigen::Vector3d velocity(double t) const;
    virtual bool isPeriodic() const;
    virtual double period() const;
    double startTime() const;
    double endTime() const;
    void setStartTime(double startTime);
    void setEndTime(double endTime);
    void setValidTimeRange(double startTime, double endTime);
};


class RotationModel : public Object
{
public:
    RotationModel() {}
    virtual ~RotationModel() {}
    virtual Eigen::Quaterniond orientation(double t) const = 0;
};


class Frame : public Object
{
public:
    virtual ~Frame() {}
    virtual Eigen::Quaterniond orientation(double t) const = 0;
};


class Arc : public Object
{
public:
    Entity* center() const;
    void setCenter(Entity* center);

    Frame* trajectoryFrame() const;
    void setTrajectoryFrame(Frame* f);

    Frame* bodyFrame() const;
    void setBodyFrame(Frame* f);

    Trajectory* trajectory() const;
    void setTrajectory(Trajectory* trajectory);

    RotationModel* rotationModel() const;
    void setRotationModel(RotationModel* rm);

    double duration() const;
    void setDuration(double t);
};


class Chronology : public Object
{
public:
    Chronology();
    ~Chronology();

    double beginning() const;
    void setBeginning(double t);
    double ending() const;
    double duration() const;

    Arc* activeArc(double t) const;
    Arc* firstArc() const;
    Arc* lastArc() const;
    Arc* arc(unsigned int index) const;
    unsigned int arcCount() const;

    void addArc(Arc* arc);
    bool empty() const;
    bool includesTime(double t) const;
};


class Entity : public Object
{
public:
    Entity();
    ~Entity();

    Eigen::Vector3d position(double t) const;
    Eigen::Quaterniond orientation(double t) const;
    virtual Geometry* geometry() const;

    LightSource* lightSource() const;
    void setLightSource(LightSource* lightSource);

    Chronology* chronology();
    bool isVisible() const;
    bool isVisible(double t) const;
    void setVisible(bool visible);

    std::string name() const;
    void setName(const std::string& name);
    
    void setVisualizer(const std::string& tag, Visualizer* visualizer);
    void removeVisualizer(const std::string& tag);
    Visualizer* visualizer(const std::string& tag) const;
    bool hasVisualizers() const;
};


class Body : public Entity
{
public:
    Geometry* geometry() const;
    void setGeometry(Geometry* geometry);
};


class Observer : public Object
{
public:
    Observer(Entity* center);
    virtual ~Observer();

    Entity* center() const;
    void setCenter(Entity* center);
    void updateCenter(Entity* center, double t);

    Frame* positionFrame() const;
    void setPositionFrame(Frame* f);
    void updatePositionFrame(Frame* positionFrame, double t);

    Frame* pointingFrame() const;
    void setPointingFrame(Frame* f);
    void updatePointingFrame(Frame* pointingFrame, double t);

    Eigen::Vector3d position() const;
    void setPosition(const Eigen::Vector3d& position);
    Eigen::Quaterniond orientation() const;
    void setOrientation(const Eigen::Quaterniond& orientation);

    void rotate(const Eigen::Quaterniond& rotation);
    void orbit(const Eigen::Quaterniond& rotation);
    void changeDistance(double factor);

    Eigen::Vector3d absolutePosition(double t) const;
    Eigen::Quaterniond absoluteOrientation(double t) const;
};


enum TimeScale
{
    TimeScale_TDB  = 0, /**< Barycentric Dynamical Time */
    TimeScale_TT   = 1, /**< Terrestrial Time */
    TimeScale_TAI  = 2, /**< International Atomic Time */
    TimeScale_UTC  = 3, /**< Coordinated Universal Time */
};


class GregorianDate
{
public:
    enum Format
    {
        ISO8601_Combined = 0,
    };

    GregorianDate();
    GregorianDate(const GregorianDate& other);
    GregorianDate(int year, unsigned int month, unsigned int day,
                  unsigned int hour = 0, unsigned int minute = 0, unsigned int second = 0, unsigned int usec = 0,
                  TimeScale timeScale = TimeScale_UTC);

    GregorianDate& operator=(const GregorianDate& other);

    int year() const;
    unsigned int month() const;
    unsigned int day() const;
    unsigned int hour() const;
    unsigned int minute() const;
    unsigned int second() const;
    unsigned int usec() const;

    TimeScale timeScale() const;
    void setTimeScale(TimeScale timeScale);

    unsigned int dayOfWeek() const;
    unsigned int dayOfYear() const;
    unsigned int daysInMonth() const;
    int julianDay() const;

    bool isValid() const;

    bool isLeapYear() const;

    double toTDBJD() const;
    double toTAIJD() const;
    double toTTJD() const;
    double toTDBSec() const;

    std::string toString(Format format = ISO8601_Combined) const;

    static GregorianDate UTCDateFromTDBJD(double tdbjd);
    static GregorianDate TDBDateFromTDBJD(double tdbjd);
    static GregorianDate UTCDateFromTDBSec(double tdbsec);
    static GregorianDate TDBDateFromTDBSec(double tdbsec);
};


class DataChunk : public Object
{
public:

%extend
{
    DataChunk(const signed char* byteArray, unsigned int byteArrayLength)
    {
        vesta::DataChunk* data = new vesta::DataChunk((const char*) byteArray, byteArrayLength);
        return data;
    }
}

    ~DataChunk();
    
    unsigned int size() const;
};


class Viewport
{
public:
    Viewport(unsigned int width, unsigned int height);
    Viewport(int x, int y, unsigned int width, unsigned int height);

    int x() const;
    void setX(int x);
    int y() const;
    void setY(int y);
    void setOrigin(int x, int y);
    unsigned int width() const;
    void setWidth(unsigned int width);
    unsigned int height() const;
    void setHeight(unsigned int height);
    void setSize(unsigned int width, unsigned int height);
    float aspectRatio() const;
};


class PlanarProjection
{
public:
    enum ProjectionType
    {
        Perspective = 0,
        Orthographic = 1,
    };

    enum Chirality
    {
        LeftHanded  = 0,
        RightHanded = 1,
    };

    PlanarProjection(ProjectionType type, float left, float right, float bottom, float top, float nearDistance, float farDistance);

    ProjectionType type() const;

    float left() const;
    float right() const;
    float bottom() const;
    float top() const;
    float nearDistance() const;
    float farDistance() const;

    Chirality chirality() const;

    //Eigen::Matrix4f matrix() const;

    float fovY() const;
    float fovX() const;
    float fovDiagonal() const;
    float aspectRatio() const;

    PlanarProjection slice(float nearDistance, float farDistance) const;

    static PlanarProjection CreatePerspective(float fovY, float aspectRatio, float nearDistance, float farDistance);
    static PlanarProjection CreatePerspectiveLH(float fovY, float aspectRatio, float nearDistance, float farDistance);
    static PlanarProjection CreateOrthographic(float left, float right, float bottom, float top, float nearDistance, float farDistance);
    static PlanarProjection CreateOrthographic2D(float left, float right, float bottom, float top);
};


%nodefaultctor Framebuffer;

class Framebuffer : public Object
{
public:
    enum
    {
        ColorAttachment = 0x1,
        DepthAttachment = 0x2,
    };

    ~Framebuffer();

    unsigned int width() const;
    unsigned int height() const;

    bool isValid() const;

    TextureMap* colorTexture() const;
    TextureMap* depthTexture() const;

    void bind() const;
    static void unbind();

    bool resize(unsigned int width, unsigned int height);

    static bool supported();

    static Framebuffer* CreateFramebuffer(unsigned int width,
                                          unsigned int height,
                                          TextureMap::ImageFormat format,
                                          TextureMap::ImageFormat depthFormat = TextureMap::Depth24);
    static Framebuffer* CreateDepthOnlyFramebuffer(unsigned int width,
                                                   unsigned int height,
                                                   TextureMap::ImageFormat depthFormat = TextureMap::Depth24);
    static Framebuffer* CreateColorOnlyFramebuffer(unsigned int width,
                                                   unsigned int height,
                                                   TextureMap::ImageFormat format);
};


%nodefaultctor CubeMapFramebuffer;

class CubeMapFramebuffer : public Object
{
public:
    enum Face
    {
        PositiveX = 0,
        NegativeX = 1,
        PositiveY = 2,
        NegativeY = 3,
        PositiveZ = 4,
        NegativeZ = 5,
    };

    ~CubeMapFramebuffer();

    Framebuffer* face(Face faceIndex) const;
    unsigned int size() const;
    TextureMap* colorTexture() const;
    TextureMap* depthTexture() const;
    static bool supported();

    static CubeMapFramebuffer* CreateCubicShadowMap(unsigned int size);
    static CubeMapFramebuffer* CreateCubicReflectionMap(unsigned int size,
                                                        TextureMap::ImageFormat format);
};


class StarCatalog : public Object
{
public:
    StarCatalog();
    ~StarCatalog();

    unsigned int size() const;
    void addStar(unsigned int identifier, double ra, double dec, double vmag, double vmag);
};


class PickResult;

class Universe : public Object
{
public:
    Universe();
    ~Universe();

    const std::vector<Entity*> entities() const;
    void addEntity(Entity* entity);
    void removeEntity(Entity* entity);
    Entity* findFirst(const std::string& name);

    StarCatalog* starCatalog() const;
    void setStarCatalog(StarCatalog* starCatalog);
    bool pickObject(double t,
                    const Eigen::Vector3d& pickOrigin,
                    const Eigen::Vector3d& pickDirection,
                    double pixelAngle,
                    PickResult* result) const;
    %extend {
    PickResult* getPickObject(double t,
                    const Eigen::Vector3d& pickOrigin,
                    const Eigen::Vector3d& pickDirection,
                    double pixelAngle)
    {
        vesta::PickResult* pickResult = new vesta::PickResult();
        bool result;
        result = $self->pickObject(t,pickOrigin,pickDirection,pixelAngle, pickResult);
        if (result)
        {
            return pickResult;
        }
        else
        {
            delete pickResult;
            return NULL;
        }
    }
    }

    void setLayer(const std::string& tag, SkyLayer* layer);
    void removeLayer(const std::string& tag);
    SkyLayer* layer(const std::string& tag) const;
    bool hasLayers() const;
    void clearLayers();
};

class Atmosphere : public Object
{
public:
    Atmosphere();
    ~Atmosphere();

    float rayleighScaleHeight();
    void setRayleighScaleHeight(float height);
    float mieScaleHeight();
    void setMieScaleHeight(float height);
    Eigen::Vector3f rayleighScatteringCoeff() const;
    void setRayleighScatteringCoeff(const Eigen::Vector3f& coeff);
    void computeRayleighScatteringCoeff(double n, double N);
    Eigen::Vector3f absorptionCoeff() const;
    void setAbsorptionCoeff(const Eigen::Vector3f& coeff);
    float mieAsymmetry() const;
    void setMieAsymmetry(float g);

    TextureMap* transmittanceTexture() const;
    TextureMap* scatterTexture() const;

    Spectrum color(float distance) const;

    void computeScattering();
    void generateTextures();

    static Atmosphere* LoadAtmScat(const DataChunk* data);
};


/****** InitialStateGenerator classes *********/

%nodefaultctor InitialStateGenerator;

class InitialStateGenerator : public Object
{
public:
    virtual ~InitialStateGenerator();
    virtual float maxDistanceFromOrigin() const = 0;
    virtual float maxSpeed() const = 0;
};


class DiscGenerator : public InitialStateGenerator
{
public:
    DiscGenerator(float radius,
                  const Eigen::Vector3f& velocity = Eigen::Vector3f::Zero());
    virtual float maxDistanceFromOrigin() const;
    virtual float maxSpeed() const;
};


class BoxGenerator : public InitialStateGenerator
{
    BoxGenerator(const Eigen::Vector3f& sideLengths = Eigen::Vector3f::Ones(),
                 const Eigen::Vector3f& center = Eigen::Vector3f::Zero(),
                 const Eigen::Vector3f& velocity = Eigen::Vector3f::Zero());
    virtual float maxDistanceFromOrigin() const;
    virtual float maxSpeed() const;
};


class PointGenerator : public InitialStateGenerator
{
public:
    PointGenerator(const Eigen::Vector3f& position = Eigen::Vector3f::Zero(),
                   const Eigen::Vector3f& velocity = Eigen::Vector3f::Zero());
    virtual float maxDistanceFromOrigin() const;
    virtual float maxSpeed() const;
};

/****** End of generators ******/


class LightSource : public Object
{
public:
    LightSource();
    ~LightSource();

    float luminosity() const;
    void setLuminosity(float luminosity);
    Spectrum spectrum() const;
    void setSpectrum(const Spectrum& spectrum);
    float range() const;
    void setRange(float range);
    bool isShadowCaster() const;
    void setShadowCaster(bool castsShadows);
};


/***** Geometry classes *****/

class Material : public Object
{
public:
    enum BlendMode
    {
        Opaque                  = 0,
        AlphaBlend              = 1,
        AdditiveBlend           = 2,
        PremultipliedAlphaBlend = 3,
    };

    enum SpecularModifierSource
    {
        SpecularTextureRGB      = 0,
        DiffuseTextureAlpha     = 1,
    };
    
    enum BRDF
    {
        Lambert,
        BlinnPhong,
        BlinnPhongReflective,
        ParticulateVolume,
    };

    Material();
    Material(const Material& m);

    BRDF brdf() const;
    float opacity() const;
    Spectrum diffuse() const;
    Spectrum specular() const;
    float phongExponent() const;
    float fresnelReflectance() const;
    bool isReflective() const;
    SpecularModifierSource specularModifier() const;
    Spectrum emission() const;
    BlendMode blendMode() const;
    TextureMap* baseTexture() const;
    TextureMap* normalTexture() const;
    TextureMap* specularTexture() const;

    void setBrdf(BRDF brdf);
    void setOpacity(float opacity);
    void setDiffuse(const Spectrum& diffuse);
    void setSpecular(const Spectrum& specular);
    void setPhongExponent(float phongExponent);
    void setFresnelReflectance(float f);
    void setSpecularModifier(SpecularModifierSource source);
    void setEmission(const Spectrum& emission);
    void setBaseTexture(TextureMap* texture);
    void setNormalTexture(TextureMap* texture);
    void setSpecularTexture(TextureMap* texture);
    void setBlendMode(BlendMode blend);
};


class Geometry : public Object
{
public:
    virtual ~Geometry();

    virtual float boundingSphereRadius() const = 0;
};

class MapLayer : public Object
{
public:
    MapLayer();
    ~MapLayer();

    TextureMap* texture() const;
    void setTexture(TextureMap* texture);
    
    float opacity() const;
    void setOpacity(float opacity);

    MapLayerBounds box() const;
    void setBox(const MapLayerBounds& box);
};

class PlanetaryRings : public Geometry
{
public:
    PlanetaryRings(float innerRadius, float outerRadius);
    virtual ~PlanetaryRings();

    virtual float boundingSphereRadius() const;

    /** Get the radius of the inner edge of the ring system (in kilometers)
      */
    float innerRadius() const;
    void setInnerRadius(float innerRadius);
    float outerRadius() const;
    void setOuterRadius(float outerRadius);
    TextureMap* texture() const;
    void setTexture(TextureMap* texture);
};


class WorldGeometry : public Geometry
{
public:
    WorldGeometry();
    virtual ~WorldGeometry();

    virtual float boundingSphereRadius() const;

    Eigen::Vector3f ellipsoidAxes() const;
    void setSphere(float radius);
    void setSpheroid(float radius, float oblateness);
    void setEllipsoid(const Eigen::Vector3f& axes);

    TextureMap* baseMap() const;
    void setBaseMap(TextureMap* baseMap);
    TextureMap* normalMap() const;
    void setNormalMap(TextureMap* normalMap);

    void addLayer(MapLayer* layer);
    void removeLayer(unsigned int index);
    void removeLayer();
    MapLayer* layer(unsigned int index) const;
    unsigned int layerCount() const;    

    bool isEmissive() const;
    void setEmissive(bool emissive);
    
    Atmosphere* atmosphere() const;
    void setAtmosphere(Atmosphere* atmosphere);

    TextureMap* cloudMap() const;
    void setCloudMap(TextureMap* cloudMap);
    float cloudAltitude() const;
    void setCloudAltitude(float);

    PlanetaryRings* ringSystem() const;
    void setRingSystem(PlanetaryRings* rings);

    Spectrum specularReflectance() const;
    void setSpecularReflectance(const Spectrum& reflectance);
    float specularPower() const;
    void setSpecularPower(float specularPower);

    void setLayer(const std::string& tag, WorldLayer* layer);
    void removeLayer(const std::string& tag);
    WorldLayer* layer(const std::string& tag) const;
    bool hasLayers() const;
    void clearLayers();
    
    %extend {
    static WorldGeometry* cast(vesta::Geometry* geometry)
    {
        return dynamic_cast<vesta::WorldGeometry*>(geometry);
    }
    }    
};


class TextureMapLoader;

class MeshGeometry : public Geometry
{
public:
    MeshGeometry();
    virtual ~MeshGeometry();

    virtual float boundingSphereRadius() const;

    Eigen::Vector3f meshScale() const;
    void setMeshScale(const Eigen::Vector3f& scale);
    void setMeshScale(float scale);

    static MeshGeometry* loadFromFile(const std::string& filename, TextureMapLoader* textureLoader);

    %extend {
    static MeshGeometry* cast(vesta::Geometry* geometry)
    {
        return dynamic_cast<vesta::MeshGeometry*>(geometry);
    }
    }    
};


class ArrowGeometry : public Geometry
{
public:
    enum
    {
        XAxis = 1,
        YAxis = 2,
        ZAxis = 4,
        AllAxes = 7
    };

    ArrowGeometry(float shaftLength,
                  float shaftRadius,
                  float headLength,
                  float headRadius);
    virtual ~ArrowGeometry();

    virtual float boundingSphereRadius() const;

    double scale() const;
    void setScale(double scale);

    unsigned int visibleArrows() const;
    void setVisibleArrows(unsigned int visibleArrows);

    Spectrum arrowColor(unsigned int which) const;
    void setArrowColor(unsigned int which, const Spectrum& color);

    float opacity() const;
    void setOpacity(float opacity);

    %extend {
    static ArrowGeometry* cast(vesta::Geometry* geometry)
    {
        return dynamic_cast<vesta::ArrowGeometry*>(geometry);
    }
    }    
};


class PlaneGeometry : public Geometry
{
public:
    PlaneGeometry();
    virtual ~PlaneGeometry();

    virtual float boundingSphereRadius() const;

    double scale() const;
    void setScale(double scale);

    Spectrum color() const;
    void setColor(const Spectrum& color);

    float opacity() const;
    void setOpacity(float opacity);

    double gridLineSpacing() const;
    void setGridLineSpacing(double gridLineSpacing);
};


class TrajectoryGeometry : public Geometry
{
public:
    enum TrajectoryPortion
    {
        Entire                  = 0,
        StartToCurrentTime      = 1,
        CurrentTimeToEnd        = 2,
        WindowBeforeCurrentTime = 3,
    };

    TrajectoryGeometry();
    virtual ~TrajectoryGeometry();

    virtual float boundingSphereRadius() const;

    Spectrum color() const;
    void setColor(const Spectrum& color);

    float opacity() const;
    void setOpacity(float opacity);

    TrajectoryPortion displayedPortion() const;
    void setDisplayedPortion(TrajectoryPortion portion);
    double windowDuration() const;
    void setWindowDuration(double duration);
    double windowLead() const;
    void setWindowLead(double duration);
    double fadeFraction() const;
    void setFadeFraction(double fadeFraction);
    float lineWidth() const;
    void setLineWidth(float width);

    void computeSamples(const Trajectory* trajectory, double startTime, double endTime, unsigned int steps);
    void updateSamples(const Trajectory* trajectory, double startTime, double endTime, unsigned int steps);
};

class TextureFont;
class RenderContext;

class FadeRange : public Object
{
public:
    FadeRange();
    FadeRange(float minPixels, float maxPixels, float minFadeExtent, float maxFadeExtent);
    FadeRange(float minPixels, float minFadeExtent);

    float minPixels() const;
    float maxPixels() const;
    float minFadeExtent() const;
    float maxFadeExtent() const;

    void setMinPixels(float minPixels);
    void setMaxPixels(float maxPixels);
    void setMinFadeExtent(float minFadeExtent);
    void setMaxFadeExtent(float maxFadeExtent);
    float opacity(float pixelSize);
};

class LabelGeometry : public Geometry
{
public:
    LabelGeometry(const std::string& text, TextureFont* font, const Spectrum& color, float iconSize);
    LabelGeometry(const std::string& text, TextureFont* font, const Spectrum& color);
    LabelGeometry();
    virtual ~LabelGeometry();

    float boundingSphereRadius() const;
    virtual bool isOpaque() const;
    virtual float apparentSize() const;
    std::string text() const;
    void setText(const std::string& text);
    const TextureFont* font() const;
    void setFont(TextureFont* font);
    Spectrum color() const;
    void setColor(const Spectrum& color);
    TextureMap* icon() const;
    void setIcon(TextureMap* icon);
    float iconSize() const;
    void setIconSize(float pixels);
    Spectrum iconColor() const;
    void setIconColor(const Spectrum& color);

    FadeRange* fadeRange() const;
    void setFadeRange(FadeRange* fadeRange);
    float fadeSize() const;
    void setFadeSize(float fadeSize);
};

class ParticleEmitter;
class TextureMap;

class ParticleSystemGeometry : public Geometry
{
public:
    ParticleSystemGeometry();
    virtual ~ParticleSystemGeometry();

    virtual float boundingSphereRadius() const;

    void addEmitter(ParticleEmitter* emitter, TextureMap* particleTexture);

    %extend {
    static ParticleSystemGeometry* cast(vesta::Geometry* geometry)
    {
        return dynamic_cast<vesta::ParticleSystemGeometry*>(geometry);
    }
    }    
};


class BillboardGeometry : public Geometry
{
public:
    BillboardGeometry();
    ~BillboardGeometry();

    float boundingSphereRadius() const;

    float size() const;
    void setSize(float size);

    TextureMap* texture() const;
    void setTexture(TextureMap* texture);
    Spectrum color() const;
    void setColor(const Spectrum& color);
    float opacity() const;
    void setOpacity(float opacity);
    bool hasFixedScreenSize() const;
    void setFixedScreenSize(bool enable);

    Material::BlendMode blendMode() const;
    void setBlendMode(Material::BlendMode blendMode);
};


/***** WorldGeometry-related classes *****/

%nodefaultctor WorldLayer;

class WorldLayer : public Object
{
public:
    bool isVisible() const;
    void setVisibility(bool visible);
    int drawOrder() const;
    void setDrawOrder(int order);
};


class PlanetGridLayer : public WorldLayer
{
public:
    PlanetGridLayer();
    ~PlanetGridLayer();

    Spectrum gridColor() const;
    void setGridColor(const Spectrum& color);
    float gridOpacity() const;
    void setGridOpacity(float opacity);

    TextureFont* labelFont() const;
    void setLabelFont(TextureFont* font);
};


%nodefaultctor TiledMap;

class TiledMap : public Object
{
public:
    struct TextureSubrect
    {
        TextureMap* texture;
        float u0;
        float v0;
        float u1;
        float v1;
    };

    virtual ~TiledMap();

    //virtual TextureSubrect tile(unsigned int level, unsigned int x, unsigned int y) = 0;
    virtual unsigned int tileSize() const;
};


class HierarchicalTiledMap : public TiledMap
{
public:
    HierarchicalTiledMap(TextureMapLoader* loader, unsigned int tileSize);
    virtual ~HierarchicalTiledMap();

    //virtual TextureSubrect tile(unsigned int level, unsigned int x, unsigned int y);
    virtual std::string tileResourceIdentifier(unsigned int level, unsigned int column, unsigned int row) = 0;
    virtual bool isValidTileAddress(unsigned int level, unsigned int column, unsigned int row) = 0;

    virtual bool tileResourceExists(const std::string& resourceId);
    TextureMapLoader* loader() const;
    virtual unsigned int tileSize() const;
};


/***** Trajectory subclasses *****/

class FixedPointTrajectory : public Trajectory
{
public:
    FixedPointTrajectory(const Eigen::Vector3d& point);
    FixedPointTrajectory(const StateVector& state);
    virtual StateVector state(double t) const;
    virtual double boundingSphereRadius() const;

    void setState(const StateVector& state);

    %extend {
    static FixedPointTrajectory* cast(vesta::Trajectory* trajectory)
    {
        return dynamic_cast<vesta::FixedPointTrajectory*>(trajectory);
    }
    }    
};

class KeplerianTrajectory : public Trajectory
{
public:
    KeplerianTrajectory(const OrbitalElements& elements);

    virtual StateVector state(double t) const;
    virtual double boundingSphereRadius() const;

    %extend {
    static KeplerianTrajectory* cast(vesta::Trajectory* trajectory)
    {
        return dynamic_cast<vesta::KeplerianTrajectory*>(trajectory);
    }
    }    
};


%feature("director") CallbackTrajectory;
%feature("nodirector") CallbackTrajectory::position;
%feature("nodirector") CallbackTrajectory::velocity;
class CallbackTrajectory : public Trajectory
{
public:
    CallbackTrajectory();

    virtual StateVector state(double t) const;
    virtual double boundingSphereRadius() const;
};



/***** RotationModel subclasses *****/

class FixedRotationModel : public RotationModel
{
public:
    FixedRotationModel(const Eigen::Quaterniond& orientation);
    Eigen::Quaterniond orientation(double t) const;
    void setOrientation(const Eigen::Quaterniond& orientation);
};

class UniformRotationModel : public RotationModel
{
public:
    UniformRotationModel(const Eigen::Vector3d& axis,
                         double rotationRate,
                         double meridianAngleAtEpoch,
                         double epoch = 0.0);
    virtual ~UniformRotationModel();

    virtual Eigen::Quaterniond orientation(double t) const;
    Eigen::Vector3d axis() const;
    double rotationRate() const;
    double meridianAngleAtEpoch() const;
    double epoch() const;
};


%feature("director") CallbackRotationModel;
class CallbackRotationModel : public RotationModel
{
public:
    CallbackRotationModel();

    virtual Eigen::Quaterniond orientation(double t) const;
    virtual Eigen::Vector3d angularVelocity(double t) const;
};


/***** Frame subclasses *****/

class InertialFrame : public Frame
{
public:
    InertialFrame(const Eigen::Quaterniond& q) : m_orientation(q) {}
    virtual ~InertialFrame() {}

    virtual Eigen::Quaterniond orientation(double t) const;
    virtual Eigen::Quaterniond orientation() const;

    static InertialFrame* eclipticJ2000();
    static InertialFrame* equatorJ2000();
    static InertialFrame* equatorB1950();
    static InertialFrame* galactic();
};

class BodyFixedFrame : public Frame
{
public:
    BodyFixedFrame(Entity* body);
    virtual ~BodyFixedFrame() {}

    virtual Eigen::Quaterniond orientation(double t) const;
    virtual Eigen::Vector3d angularVelocity(double t) const;
};

class TwoBodyRotatingFrame : public Frame
{
public:
    TwoBodyRotatingFrame(Entity* primary, Entity* secondary);
    virtual ~TwoBodyRotatingFrame();

    virtual Eigen::Quaterniond orientation(double t) const;
    virtual Eigen::Vector3d angularVelocity(double t) const;

    Entity* primary() const;
    Entity* secondary() const;
};

/**** Texture handling ****/

class TextureProperties
{
public:
    enum AddressMode
    {
        Wrap      = 0,
        Clamp     = 1,
    };

    TextureProperties();
    TextureProperties(AddressMode stAddress);

    AddressMode addressS;
    AddressMode addressT;
};

class TextureMap;

%feature("director") TextureMapLoader;
class TextureMapLoader : public Object
{
public:
    TextureMapLoader();
    virtual ~TextureMapLoader();

    TextureMap* loadTexture(const std::string& resourceName,
                            const TextureProperties& properties);
    virtual bool handleMakeResident(TextureMap* texture) = 0;

    // 64-bit integer types are problematic in SWIG; for now, we'll just
    // map them to double precision values.

    // v_uint64 evictTextures(v_uint64 desiredMemory,
    //                        v_int64 mostRecentAllowed);
    // v_uint64 textureMemoryUsed() const;
    // v_int64 frameCount() const;
    // v_int64 incrementFrameCount();

    %extend {
    double evictTextures(double desiredMemory,
                         double mostRecentAllowed)
    {
        return double($self->evictTextures(vesta::v_uint64(desiredMemory),
                                           vesta::v_int64(mostRecentAllowed)));
    }

    double textureMemoryUsed() const
    {
        return double($self->textureMemoryUsed());
    }

    double frameCount() const
    {
        return double($self->frameCount());
    }

    double incrementFrameCount()
    {
        return double($self->incrementFrameCount());
    }

    } // end extend block
};

class TextureMap : public Object
{
public:
    TextureMap(const std::string& name, TextureMapLoader* loader, const TextureProperties& properties);
    ~TextureMap();

    enum Status
    {
        Uninitialized  =  0,
        Loading        =  1,
        Ready          =  2,
        LoadingFailed  = -1,
    };

    enum ImageFormat
    {
        R8G8B8A8 =  0,
        B8G8R8A8 =  1,
        R8G8B8   =  2,
        B8G8R8   =  3,
        DXT1     =  4,
        DXT3     =  5,
        DXT5     =  6,
        RGB16F   =  7,
        RGBA16F  =  8,
        RGB32F   =  9,
        RGBA32F  = 10,
        R16F     = 11,
        R32F     = 12,
        RG16F    = 13,
        RG32F    = 14,
        Depth24  = 15,
        R8G8B8_sRGB   = 16,
        R8G8B8A8_sRGB = 17,
        DXT1_sRGB     = 18,
        DXT3_sRGB     = 19,
        DXT5_sRGB     = 20,
        Depth16       = 21,
        Depth32       = 22,
        Depth32F      = 23,
        FormatCount   = 24,
        InvalidFormat = -1,
    };

    unsigned int id() const;
    const std::string name() const;
    bool isResident() const;
    bool makeResident();

    Status status() const;
    void setStatus(Status status);

    unsigned int memoryUsage() const;

    // Map 64-bit integers to doubles because many scripting languages
    // only support 32-bit integers.
    // v_int64 lastUsed() const;
    // void setLastUsed(v_int64 lastUsed);
    %extend {
        double lastUsed() const
        {
            return double($self->lastUsed());
        }

        void setLastUsed(double lastUsed)
        {
            $self->setLastUsed(vesta::v_int64(lastUsed));
        }
    }

    void evict();

#ifdef SWIGJAVA
    bool generate(const signed char imageData[],
                  unsigned int imageDataSize,
                  unsigned int width,
                  unsigned int height,
                  ImageFormat format)
    {
    }
#else
    bool generate(const unsigned char imageData[],
                  unsigned int imageDataSize,
                  unsigned int width,
                  unsigned int height,
                  ImageFormat format);
#endif
};


class Visualizer : public Object
{
public:
    Visualizer(Geometry* geometry);
    virtual ~Visualizer();

    bool isVisible() const;
    void setVisibility(bool visible);

    virtual Eigen::Quaterniond orientation(const Entity* parent, double t) const;
    Geometry* geometry() const;

    enum DepthAdjustment
    {
        NoAdjustment   = 0,
        AdjustToFront  = 1,
        AdjustToBack   = 2,
    };

    DepthAdjustment depthAdjustment() const;
    void setDepthAdjustment(DepthAdjustment adjustment);
};


class ArrowVisualizer : public Visualizer
{
public:
    ArrowVisualizer(double size);
    virtual ~ArrowVisualizer();

    Eigen::Quaterniond orientation(const Entity* parent, double t) const;
    virtual Eigen::Vector3d direction(const Entity* parent, double t) const = 0;

    Spectrum color() const;
    void setColor(const Spectrum& color);

    %extend {
    static ArrowVisualizer* cast(vesta::Visualizer* visualizer)
    {
        return dynamic_cast<vesta::ArrowVisualizer*>(visualizer);
    }
    }    
};


class VelocityVisualizer : public ArrowVisualizer
{
public:
    VelocityVisualizer(double size);
    virtual ~VelocityVisualizer();

    virtual Eigen::Vector3d direction(const Entity* parent, double t) const;

    %extend {
    static VelocityVisualizer* cast(vesta::Visualizer* visualizer)
    {
        return dynamic_cast<vesta::VelocityVisualizer*>(visualizer);
    }
    }    
};


class AxesVisualizer : public Visualizer
{
public:
    enum AxesType
    {
        FrameAxes = 1,
        BodyAxes  = 2,
    };

    AxesVisualizer(AxesType axesType, double size);
    virtual ~AxesVisualizer();
    
    ArrowGeometry* arrows();

    %extend {
    static AxesVisualizer* cast(vesta::Visualizer* visualizer)
    {
        return dynamic_cast<vesta::AxesVisualizer*>(visualizer);
    }
    }    
};


class NadirVisualizer : public ArrowVisualizer
{
public:
    NadirVisualizer(double size);
    virtual ~NadirVisualizer();

    virtual Eigen::Vector3d direction(const Entity* parent, double t) const;

    %extend {
    static NadirVisualizer* cast(vesta::Visualizer* visualizer)
    {
        return dynamic_cast<vesta::NadirVisualizer*>(visualizer);
    }
    }    
};


class BodyDirectionVisualizer : public ArrowVisualizer
{
public:
    BodyDirectionVisualizer(double size, Entity* body);
    virtual ~BodyDirectionVisualizer();

    virtual Eigen::Vector3d direction(const Entity* parent, double t) const;
    Entity* body() const;
    void setBody(Entity* body);

    %extend {
    static BodyDirectionVisualizer* cast(vesta::Visualizer* visualizer)
    {
        return dynamic_cast<vesta::BodyDirectionVisualizer*>(visualizer);
    }
    }    
};


class PlaneVisualizer : public Visualizer
{
public:
    PlaneVisualizer(double size);
    virtual ~PlaneVisualizer();

    PlaneGeometry* plane() const;
    Frame* frame() const;
    void setFrame(Frame* frame);

    %extend {
    static PlaneVisualizer* cast(vesta::Visualizer* visualizer)
    {
        return dynamic_cast<vesta::PlaneVisualizer*>(visualizer);
    }
    }    
};

class SensorVisualizer : public Visualizer
{
public:
    enum FrustumShape
    {
        Elliptical = 0,
        Rectangular = 1,
    };

    SensorVisualizer();
    virtual ~SensorVisualizer();

    virtual Eigen::Quaterniond orientation(const Entity* parent, double t) const;

    Entity* source() const;
    void setSource(Entity* source);
    Entity* target() const;
    void setTarget(Entity* target);
    Spectrum color() const;
    void setColor(const Spectrum& color);
    float opacity() const;
    void setOpacity(float opacity);

    Eigen::Quaterniond sensorOrientation() const;
    void setSensorOrientation(const Eigen::Quaterniond& orientation);

    FrustumShape frustumShape() const;
    void setFrustumShape(SensorVisualizer::FrustumShape shape);
    void setFrustumAngles(double horizontal, double vertical);

    double range() const;
    void setRange(double range);
};

// The following is necessary because the abstract methods of the SkyLayer
// base class are not exposed in the scripting interface.
%nodefaultctor SkyLayer;

class SkyLayer : public Object
{
public:
    virtual ~SkyLayer();

    bool isVisible() const;
    void setVisibility(bool visible);
    int drawOrder() const;
    void setDrawOrder(int order);
};

class CelestialCoordinateGrid : public SkyLayer
{
public:
    enum GridFrame
    {
        InertialFrame   = 0,
        HorizontalFrame = 1,
    };

    enum LongitudeUnits
    {
        Degrees   = 0,
        Hours     = 1,
    };

    enum GridStyle
    {
        LabeledGrid   = 0,
        UnlabeledGrid = 1,
        EquatorOnly   = 2,
    };

    CelestialCoordinateGrid();
    ~CelestialCoordinateGrid();

    GridFrame frame() const;
    void setFrame(GridFrame frame);

    Eigen::Quaterniond orientation() const;
    void setOrientation(const Eigen::Quaterniond& orientation);

    LongitudeUnits longitudeUnits() const;
    void setLongitudeUnits(LongitudeUnits units);

    Spectrum color() const;
    void setColor(const Spectrum& color);

    GridStyle gridStyle() const;
    void setGridStyle(GridStyle style);

    %extend {
    static CelestialCoordinateGrid* cast(vesta::SkyLayer* layer)
    {
        return dynamic_cast<vesta::CelestialCoordinateGrid*>(layer);
    }
    }    
};


class StarsLayer : public SkyLayer
{
public:
    StarsLayer();
    explicit StarsLayer(StarCatalog* starCatalog);
    ~StarsLayer();

    StarCatalog* starCatalog() const;
    void setStarCatalog(StarCatalog* starCatalog);

    enum StarStyle
    {
        PointStars    = 0,
        GaussianStars = 1,
    };

    StarStyle style() const;
    void setStyle(StarStyle style);

    float limitingMagnitude() const;
    void setLimitingMagnitude(float limitingMagnitude);
    float diffractionSpikeBrightness();
    void setDiffractionSpikeBrightness(float brightness);

    %extend {
    static StarsLayer* cast(vesta::SkyLayer* layer)
    {
        return dynamic_cast<vesta::StarsLayer*>(layer);
    }
    }
};


class SkyImageLayer : public SkyLayer
{
public:
    SkyImageLayer();
    ~SkyImageLayer();

    Eigen::Quaterniond orientation() const;
    void setOrientation(const Eigen::Quaterniond& orientation);
    float opacity() const;
    void setOpacity(float opacity);
    TextureMap* texture() const;
    void setTexture(TextureMap* texture);

    %extend {
    static SkyImageLayer* cast(vesta::SkyLayer* layer)
    {
        return dynamic_cast<vesta::SkyImageLayer*>(layer);
    }
    }
};


class LightingEnvironment
{
public:
    LightingEnvironment();
    ~LightingEnvironment();
    void reset();

    %extend
    {
    void addReflectionRegion(const Eigen::Vector3f& center,
                             float radius,
                             TextureMap* cubeMap)
    {
        vesta::ReflectionRegion r;
        r.region = BoundingSphere<float>(center, radius);
        r.cubeMap = cubeMap;
        $self->reflectionRegions().push_back(r);
    }
    }
};


%nodefaultctor GlareOverlay;

class GlareOverlay
{
public:
    ~GlareOverlay();

    float adaptationRate() const;
    void setAdaptationRate(float rate);
    float glareSize();
    void setGlareSize(float radiusInPixels);
    void adjustBrightness();
};


class UniverseRenderer
{
public:
    UniverseRenderer();
    ~UniverseRenderer();

    enum RenderStatus
    {
        RenderOk,
        RenderNoViewSet,
        RenderViewSetAlreadyStarted,
        RendererUninitialized,
    };

    RenderStatus beginViewSet(const Universe* universe, double t);
    RenderStatus endViewSet();

    RenderStatus renderView(const Observer* observer,
                            double fieldOfView,
                            int viewportWidth,
                            int viewportHeight);
    RenderStatus renderView(const LightingEnvironment* lighting,
                            const Observer* observer,
                            double fieldOfView,
                            const Viewport& viewport,
                            Framebuffer* renderSurface = 0);
    RenderStatus renderView(const LightingEnvironment* lighting,
                            const Eigen::Vector3d& cameraPosition,
                            const Eigen::Quaterniond& cameraOrientation,
                            const PlanarProjection& projection,
                            const Viewport& viewport,
                            Framebuffer* renderSurface = 0);
    RenderStatus renderCubeMap(const LightingEnvironment* lighting,
                               const Eigen::Vector3d& cameraPosition,
                               CubeMapFramebuffer* cubeMap,
                               double nearDistance = MinimumNearDistance,
                               double farDistance = MaximumFarDistance,
                               const Eigen::Quaterniond& rotation = Eigen::Quaterniond::Identity());
    RenderStatus renderShadowCubeMap(const LightingEnvironment* lighting,
                                     const Eigen::Vector3d& cameraPosition,
                                     CubeMapFramebuffer* cubeMap);

    RenderStatus renderLightGlare(GlareOverlay* glareOverlay);
    GlareOverlay* createGlareOverlay();


    Spectrum ambientLight() const;
    void setAmbientLight(const Spectrum& spectrum);

    bool visualizersEnabled() const;
    void setVisualizersEnabled(bool enable);
    bool skyLayersEnabled() const;
    void setSkyLayersEnabled(bool enable);

    bool initializeGraphics();
    bool initializeShadowMaps(unsigned int shadowMapSize = 1024,
                              unsigned int maxShadowMaps = 1);
    bool initializeOmniShadowMaps(unsigned int shadowMapSize = 1024,
                                  unsigned int maxShadowMaps = 1);

    bool shadowsEnabled() const;
    void setShadowsEnabled(bool enable);
    void setEclipseShadowsEnabled(bool enable);
    bool shadowsSupported() const;    
    bool omniShadowsSupported() const;
};

class ParticleEmitter : public Object
{
public:
    ParticleEmitter();
    ~ParticleEmitter();

    double particleLifetime() const;
    void setParticleLifetime(double particleLifetime);
    double spawnRate() const;
    void setSpawnRate(double spawnRate);
    double startTime() const;
    double endTime() const;
    void setTimeRange(double startTime, double endTime);
    void setStartTime(double startTime);
    void setEndTime(double endTime);
    float startSize() const;
    float endSize() const;
    void setSizeRange(float startSize, float endSize);
    Eigen::Vector3f force() const;
    void setForce(const Eigen::Vector3f& force);
    void setColor(unsigned int index, const Spectrum& color, float opacity);
    unsigned int colorCount() const;
    void setColorCount(unsigned int count);
    InitialStateGenerator* generator() const;
    void setGenerator(InitialStateGenerator* generator);
    float velocityVariation() const;
    void setVelocityVariation(float variation);

    float boundingRadius() const;
};

class PickResult
{
public:
    PickResult();
    PickResult(const PickResult& other);
    PickResult& operator=(const PickResult& other);
    ~PickResult();
    
    bool hit() const;
    Entity* hitObject() const;
    Eigen::Vector3d intersectionPoint() const;
    double distance() const;
    void setHit(Entity* hitObject, double distance, const Eigen::Vector3d& intersectionPoint);

    void setMiss();

};

class TextureFont : public Object
{
public:
    struct Glyph
    {
        unsigned int characterId;
        Eigen::Vector2f textureCoords[4];

        Eigen::Vector2f offset;
        Eigen::Vector2f size;

        float advance;
    };
    %extend {
    static vesta::TextureFont::Glyph *createGlyph()
    {
        vesta::TextureFont::Glyph *glyph = (vesta::TextureFont::Glyph *) malloc(sizeof(vesta::TextureFont::Glyph));
        return glyph;
    }
    static void setCharacterId(vesta::TextureFont::Glyph& glyph, unsigned int characterId) {
        glyph.characterId = characterId;      
    }
    static void setTextureCoords(vesta::TextureFont::Glyph& glyph, Eigen::Vector2f textureCoords, int index) {
        glyph.textureCoords[index] = textureCoords;      
    }
    static void setOffset(vesta::TextureFont::Glyph& glyph, Eigen::Vector2f offset) {
        glyph.offset = offset;      
    }
    static void setSize(vesta::TextureFont::Glyph& glyph, Eigen::Vector2f size) {
        glyph.size = size;      
    }    
    static void setAdvance(vesta::TextureFont::Glyph& glyph, float advance) {
        glyph.advance = advance;      
    }    
    }

    TextureFont();
    ~TextureFont();
    float textWidth(const std::string& text) const;
    const vesta::TextureFont::Glyph* lookupGlyph(wchar_t ch) const;
    Eigen::Vector2f render(const std::string& text,
                           const Eigen::Vector2f& startPosition) const;
    void addGlyph(const vesta::TextureFont::Glyph& glyph);
    void buildCharacterSet();
    bool buildFontTexture(unsigned int width,
                          unsigned int height,
                          unsigned char* pixels);
    %ignore pixels;
    %extend {
    bool buildFontTexture(unsigned int width,
                          unsigned int height,
                          signed char* pixels) {
        return $self->buildFontTexture(width, height, (unsigned char*) pixels); 
    }
    }

    void bind() const;
    TextureMap* glyphTexture() const;

    static TextureFont* LoadTxf(const DataChunk* txfData);
};


class DDSLoader : public Object
{
public:
    DDSLoader();
    ~DDSLoader();

    bool load(TextureMap* tex, const DataChunk* data);

    std::string errorMessage() const;
};


class ObserverController : public Object
{
public:
    ObserverController();
    ~ObserverController();

    void tick(double dt);

    Observer* observer() const;
    void setObserver(Observer* observer);

    void applyTorque(const Eigen::Vector3d& torque);
    void roll(double f);
    void pitch(double f);
    void yaw(double f);
    void applyOrbitTorque(const Eigen::Vector3d& torque);
    void dolly(double factor);
    void stop();

    double rotationDampingFactor() const;
    void setRotationDampingFactor(double rotationDampingFactor);
};

}