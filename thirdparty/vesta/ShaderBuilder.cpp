/*
 * $Revision: 546 $ $Date: 2010-10-21 12:37:00 -0700 (Thu, 21 Oct 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "ShaderBuilder.h"
#include "Debug.h"
#include <sstream>

using namespace vesta;
using namespace std;

#define DUMP_SHADER_SOURCE 0


ShaderBuilder ShaderBuilder::s_GLSLBuilder;


ShaderBuilder::ShaderBuilder()
{
}


ShaderBuilder::~ShaderBuilder()
{
}


/** Find the shader for the specified set of surface properties
  * and lighting state. Use a cached shader when possible. Returns
  * a default shader when shader creation is unsuccessful because
  * hardware register or instruction limits were exceeded.
  */
GLShaderProgram*
ShaderBuilder::getShader(const ShaderInfo& shaderInfo)
{
    ShaderCache::iterator pos = m_shaderCache.find(shaderInfo);
    if (pos != m_shaderCache.end())
    {
        // Shader was already in the cache
        return pos->second;
    }

    GLShaderProgram* shader = generateShader(shaderInfo);
    m_shaderCache[shaderInfo] = shader;

    return shader;
}


// Get the value of position appropriate for this shader: the normalized position value
// for spherical geometry, the interpolated vertex position otherwise.
static string position(const ShaderInfo& shaderInfo)
{
    if (shaderInfo.isSpherical())
    {
        return "nposition";
    }
    else
    {
        return "position";
    }
}


static void declareSamplers(ostream& out, unsigned int textureMask)
{
    if ((textureMask & ShaderInfo::DiffuseTexture) != 0)
    {
        out << "uniform sampler2D diffuseTex;" << endl;
    }

    if ((textureMask & ShaderInfo::SpecularTexture) != 0)
    {
        out << "uniform sampler2D specularTex;\n" << endl;
    }

    if ((textureMask & ShaderInfo::EmissiveTexture) != 0)
    {
        out << "uniform sampler2D emissiveTex;\n" << endl;
    }

    if ((textureMask & ShaderInfo::NormalTexture) != 0)
    {
        out << "uniform sampler2D normalTex;\n" << endl;
    }

    if ((textureMask & ShaderInfo::ReflectionTexture) != 0)
    {
        out << "uniform samplerCube reflectionTex;\n" << endl;
    }
}


static void declareShadowSamplers(ostream& out, const ShaderInfo& shaderInfo)
{
    for (unsigned int i = 0; i < shaderInfo.shadowCount(); ++i)
    {
        out << "uniform sampler2DShadow shadowTex" << i << ";\n";
    }

    for (unsigned int i = 0; i < shaderInfo.omniShadowCount(); ++i)
    {
        out << "uniform samplerCube shadowCubeMap" << i << ";\n";
    }
}

static void declareVarying(ostream& vertexOut, ostream& fragOut, const char* type, const char* name)
{
    vertexOut << "varying " << type << " " << name << ";" << endl;
    fragOut   << "varying " << type << " " << name << ";" << endl;
}

static void declareVaryingArray(ostream& vertexOut, ostream& fragOut, const char* type, const char* name, unsigned int count)
{
    vertexOut << "varying " << type << " " << name << "[" << count << "];" << endl;
    fragOut   << "varying " << type << " " << name << "[" << count << "];" << endl;
}

static void declareUniform(ostream& out, const char* type, const char* name)
{
    out << "uniform " << type << " " << name << ";" << endl;
}

static void declareUniformArray(ostream& out, const char* type, const char* name, unsigned int count)
{
    out << "uniform " << type << " " << name << "[" << count << "];" << endl;
}

static void declarePCFShadowFunc(ostream& out)
{
    const int KernelWidth = 4;
    float weight = 1.0f / (float) (KernelWidth * KernelWidth);

    // Box filter PCF with depth texture
    out << "float shadowPCF(sampler2DShadow shadowTex, vec4 shadowCoord)" << endl;
    out << "{" << endl;
    out << "    float light = 0.0;" << endl;

    float filterWidth = (float) KernelWidth - 1.0f;
    float firstSample = -filterWidth / 2.0f;
    float lastSample = firstSample + filterWidth;
    out << "    for (float y = " << firstSample << "; y <= " << lastSample << "; y += 1.0)" << endl;
    out << "        for (float x = " << firstSample << "; x <= " << lastSample << "; x += 1.0)" << endl;
    out << "            light += shadow2D(shadowTex, shadowCoord.xyz + vec3(x * shadowTexelSize, y * shadowTexelSize, 0.0005)).z;" << endl;
    out << "    return light * " << weight << ";" << endl;
    out << "}" << endl;
    out << endl;
}

static void declareCubeMapShadowFunc(ostream& out)
{
    out << "float omniShadow(samplerCube shadowTex, vec3 lightPos)" << endl;
    out << "{" << endl;
    out << "    lightPos.z = -lightPos.z;" << endl;
    out << "    float lightShadowDistance = textureCube(shadowTex, -lightPos).r;" << endl;
    out << "    float lightFragDistance = length(lightPos);" << endl;
    out << "    return lightFragDistance < lightShadowDistance ? 1.0 : 0.0;" << endl;
    out << "}" << endl;
    out << endl;
}

static string arrayIndex(const string& arrayName, unsigned int index)
{
    ostringstream ostream;
    ostream << arrayName << "[" << index << "]";
    return ostream.str();
}


#define ANALYTIC_TRANSMITTANCE 1

static void declareOpticalDepthFunc(ostream& out)
{
    // Analytic calculation of optical depth
    // Based on approximation from E. Bruneton and F. Neyret, "Precomputed Atmospheric Scattering" (2008)
    //     - r is distance of eye from planet center
    //     - z is the cosine of angle between the zenith and view direction
    //     - pathLength is the distance that the ray travels through the atmosphere
    //     - H is the scale height
    out << "float opticalDepth(float r, float z, float pathLength, float H)" << endl;
    out << "{" << endl;
    out << "    float a = sqrt(r * (0.5 / H));" << endl;
    out << "    vec2 b = a * vec2(z, z + pathLength / r);" << endl;
    out << "    vec2 b2 = b * b;" << endl;
    out << "    vec2 signB = sign(b);" << endl;
    out << "    float x = signB.y > signB.x ? exp(b2.x) : 0.0;" << endl;
    out << "    vec2 y = signB / (2.3193 * abs(b) + sqrt(1.52 * b2 + 4.0)) * vec2(1.0, exp(-pathLength / H * (pathLength / (2.0 * r) + z)));" << endl;
    out << "    return sqrt((6.2831 * H) * r) * exp((planetRadius - r) / H) * (x + y.x - y.y);" << endl;
    out << "}" << endl;
}


static void declareTransmittanceFunc(ostream& out)
{
    // Integrate transmittance by ray stepping from point x to x0; both x and
    // x0 must be points within the spherical atmosphere volume.
    out << "float transmittance(vec3 x, vec3 x0)" << endl;
    out << "{" << endl;
    out << "    float T = 0.0;" << endl;
    out << "    float pathLength = length(x - x0);" << endl;
#if ANALYTIC_TRANSMITTANCE
    out << "    float r = length(x);" << endl;
    out << "    vec3 zenith = x / r;" << endl;
    out << "    vec3 path = x0 - x;" << endl;
    out << "    float mu = dot(zenith, path) / pathLength;" << endl;
    out << "    return opticalDepth(r, mu, pathLength, scaleHeight);" << endl;
#else

#if 1
    // Integrate by ray stepping
    unsigned int integrationSteps = 5;
    out << "    vec3 step = (x0 - x) * " << 1.0f / integrationSteps << ";" << endl;
    out << "    for (int i = 0; i < " << integrationSteps << "; ++i) {" << endl;
    out << "        float h = max(0.0, length(x) - planetRadius);" << endl;
    out << "        T += exp(-h / scaleHeight);" << endl;
    out << "        x += step;" << endl;
    out << "    }" << endl;
    out << "    return T * pathLength * " << 1.0f / integrationSteps << ";" << endl;
#else
    // 2-point approximation
    // Sample density at two points: the entry point, and the midpoint between the first
    // entry point and the end of the ray. This very crudely approximates the integral of
    // density over the enire ray path.
    out << "    vec3 mid = 0.5 * (x + x0);" << endl;
    out << "    float h = max(0.0, length(mid) - planetRadius);" << endl;
    out << "    T += exp(-h / scaleHeight);" << endl;
    out << "    h = max(0.0, length(x) - planetRadius);" << endl;
    out << "    T += exp(-h / scaleHeight);" << endl;
    out << "    return T * pathLength * 0.5;" << endl;
#endif

#endif
    out << "}" << endl;
}


static void declareScatteringFunc(ostream& out)
{
    out << "    uniform float scaleHeight;" << endl;
    out << "    uniform float Bs;" << endl;                // scattering coefficient
    // scattering coefficient ratios used to estimate color of Mie scattered light
    out << "    uniform vec3 scatterCoeffRatios;" << endl;
    out << "    uniform float mieG;" << endl;

#if ANALYTIC_TRANSMITTANCE
    declareOpticalDepthFunc(out);
#endif
    declareTransmittanceFunc(out);

    // Calculate atmospheric scattering
    //    P is the fragment position
    //    X is the eye position
    //    V is the view direction (normalized)
    //    S is the sun direction (normalized)
    //
    //    sc (output) inscatter
    //    ex (output) extinction

    //    All parameters are in model space.

    out << "void scattering(vec3 P, vec3 X, vec3 V, vec3 S, out vec3 sc, out vec3 sunAtten, out vec3 eyeAtten)" << endl;
    out << "{" << endl;
#if 0
    out << "    float f = 0.0;" << endl;
    out << "    float vc = dot(V, X);" << endl;

    // Solve a quadratic equation to find the intersection of the view ray and
    // the atmosphere shell. If the discriminant is negative there's no intersection,
    // and scattering makes no contribution to the pixel color.
    out << "    float disc = vc * vc - dot(X, X) + atmosphereRadius * atmosphereRadius;" << endl;
    out << "    if (disc > 0.0) {" << endl;
    out << "        float d = max(0.0, -vc - sqrt(disc));" << endl;
    out << "        vec3 atmEnter = X + d * V;" << endl;
    out << "        float scatter = transmittance(atmEnter, " << position(shaderInfo) << ");" << endl;
    out << "        f = exp(-Bs * scatter);" << endl;

    out << "        f = 1.0 - f;" << endl;

    // Self-shadowing
    out << "        float t = dot(" << position(shaderInfo) << ", S);" << endl;
    out << "        float u = length(" << position(shaderInfo) << " - S * t);" << endl;
    out << "        if (t < 0.0) f *= smoothstep(planetRadius * 0.97, planetRadius, u);" << endl;
    out << "    }" << endl;
    out << "    sc = f * atmosphereColor;" << endl;
#else
    out << "    sc = vec3(0.0, 0.0, 0.0);" << endl;
    out << "    sunAtten = vec3(1.0, 1.0, 1.0);" << endl;
    out << "    eyeAtten = vec3(1.0, 1.0, 1.0);" << endl;
    out << "    float vc = dot(V, X);" << endl;

    // Solve a quadratic equation to find the intersection of the view ray and
    // the atmosphere shell. If the discriminant is negative there's no intersection,
    // and scattering makes no contribution to the pixel color.
    out << "    float ar2 = atmosphereRadius * atmosphereRadius;" << endl;
    out << "    float disc = vc * vc - dot(X, X) + ar2;" << endl;
    out << "    if (disc > 0.0) {" << endl;
    out << "        float d = max(0.0, -vc - sqrt(disc));" << endl;
    out << "        vec3 atmEnter = X + d * V;" << endl;

    // Map input parameters to texture coordinates
    out << "        float r = length(atmEnter);" << endl;
    out << "        float h = max(0.0, r - planetRadius) / (atmosphereRadius - planetRadius);" << endl;
    out << "        float uv = dot(V, atmEnter) / r;" << endl;
    out << "        uv = 1.1 * (uv + 0.15) / (0.1 + abs(uv + 0.15)) * 0.5 + 0.5;" << endl;
    out << "        float muS = dot(S, atmEnter) / r;" << endl;
    out << "        float uMuS = (1.0 - exp(-2.0 * muS - 0.6)) / (1.0 - exp(-2.6));" << endl;
    out << "        eyeAtten = texture2D(transmittanceTex, vec2(uv, sqrt(h))).rgb;" << endl;
    out << "        vec4 inscatter = texture3D(scatterTex, vec3(uMuS, uv, sqrt(h)));" << endl;

    out << "        muS = dot(S, P) / length(P);" << endl;
    out << "        float uMuS2 = 1.1 * (muS + 0.15) / (0.1 + abs(muS + 0.15)) * 0.5 + 0.5;" << endl;
    out << "        sunAtten = texture2D(transmittanceTex, vec2(uMuS2, 0.0)).rgb * step(0.0, muS);" << endl;

    // Rayleigh and Mie phase functions.
    // Currently using Henyey-Greenstein approximation for Mie scattering
    // Rayleigh phase function should have a normalization factor of 3/(16pi) and
    // HG phase function should have a normalization factor of 1/(4pi). These are
    // currently omitted, since the normalization factor of 1/pi is omitted
    // throughout VESTA in reflectance calculations. We treat the 1/pi
    // factor as implicit, for the phase function normalization factors use the
    // ratio of the actual factor and 1/pi.
    out << "        float s = dot(V, S);" << endl;
    //out << "        float pR = 0.75 * (1.0 + s * s);" << endl;
    //out << "        float pM = (1.0 - mieG * mieG) * pow(1.0 + mieG * mieG - 2.0 * mieG * s, -1.5);" << endl;
    out << "        float pR = 0.1875 * (1.0 + s * s);" << endl;
    out << "        float pM = 0.25 * (1.0 - mieG * mieG) * pow(1.0 + mieG * mieG - 2.0 * mieG * s, -1.5);" << endl;

    out << "        vec3 mieRgb = inscatter.rgb * inscatter.a / max(inscatter.r, 1.0e-4) * scatterCoeffRatios;" << endl;
    out << "        sc = inscatter.rgb * pR + mieRgb * pM;" << endl;

    out << "    }" << endl;
#endif

    out << "}" << endl;
}


static string fresnelTerm(const string& cosIncidence)
{
    // Use the Schlick approximation to calculate the Fresnel reflectance
    // from the cosine of the incidence angle and the reflectance at normal
    // incidence.
    return string("mix(fresnelReflectance, 1.0, pow(1.0 - ") + cosIncidence + ", 5.0));";
}


static void declareHelperFunctions(ostream& fragment, const ShaderInfo& shaderInfo)
{
    // Declare all required helper functions
    if (shaderInfo.hasShadows())
    {
        fragment << "    uniform float shadowTexelSize;" << endl;
        declarePCFShadowFunc(fragment);
    }

    if (shaderInfo.hasOmniShadows())
    {
        declareCubeMapShadowFunc(fragment);
    }

    if (shaderInfo.hasScattering())
    {
        declareScatteringFunc(fragment);
    }
}


static void generateUnlitShader(ostream& vertex, ostream& fragment, const ShaderInfo& shaderInfo)
{
    vertex << "void main()" << endl;
    vertex << "{" << endl;
    if (shaderInfo.hasTextureCoord())
    {
        vertex << "    texCoord = gl_MultiTexCoord0.xy;" << endl;
    }
    if (shaderInfo.hasVertexColors() != 0)
    {
        vertex << "    vertexColor = gl_Color;" << endl;
    }
    vertex << "    gl_Position = ftransform();" << endl;
    vertex << "}" << endl;

    declareSamplers(fragment, shaderInfo.textures() & ShaderInfo::DiffuseTexture);
    declareUniform(fragment, "vec3", "color");
    declareUniform(fragment, "float", "opacity");

    fragment << "void main()" << endl;
    fragment << "{" << endl;
    fragment << "    vec4 fragColor = vec4(color, opacity);" << endl;
    if (shaderInfo.hasTexture(ShaderInfo::DiffuseTexture))
    {
        if (shaderInfo.hasAlphaTexture())
        {
            fragment << "    fragColor.a *= texture2D(diffuseTex, texCoord).a;" << endl;
        }
        else
        {
            fragment << "    fragColor *= texture2D(diffuseTex, texCoord);" << endl;
        }
    }
    if (shaderInfo.hasVertexColors())
    {
        fragment << "    fragColor *= vertexColor;" << endl;
    }
    fragment << "    gl_FragColor = fragColor;" << endl;
    fragment << "}" << endl;
}


static void generateBlinnPhongShader(ostream& vertex, ostream& fragment, const ShaderInfo& shaderInfo)
{
    bool phong = shaderInfo.reflectanceModel() == ShaderInfo::BlinnPhong;
    bool hasTangents = shaderInfo.hasTexture(ShaderInfo::NormalTexture);
    bool hasLocalLightSources = shaderInfo.lightCount() > 1;
    bool hasEnvironmentMap = shaderInfo.hasTexture(ShaderInfo::ReflectionTexture);

    // View dependent is set to true when atmospheric scattering is enabled or
    // when the reflectance model is view-dependent (i.e. almost anything but
    // a purely Lambertian surface.)
    bool isViewDependent = false;
    if (shaderInfo.reflectanceModel() == ShaderInfo::BlinnPhong ||
        hasEnvironmentMap ||
        shaderInfo.hasScattering())
    {
        isViewDependent = true;
    }

    bool usesPosition = isViewDependent || hasLocalLightSources;

    // Interpolated variables
    declareVarying(vertex, fragment, "vec3", "normal");   // surface normal
    if (usesPosition)
    {
        declareVarying(vertex, fragment, "vec3", "position"); // position in local space
        if (shaderInfo.isSpherical())
        {
            fragment << "vec3 nposition = normalize(position);" << endl;
        }
    }
    if (hasTangents)
    {
        vertex << "attribute vec3 vtxTangent;" << endl;
        declareVarying(vertex, fragment, "vec3", "tangent");   // surface tangent
    }

    // Fragment shader constants
    declareSamplers(fragment, shaderInfo.textures());
    declareUniform(fragment, "vec3", "color");
    declareUniform(fragment, "float", "opacity");
    declareUniform(fragment, "vec3", "ambientLight");

    if (shaderInfo.hasShadows())
    {
        unsigned int count = shaderInfo.shadowCount();
        declareUniformArray(vertex, "mat4", "shadowMatrix", count);
        declareVaryingArray(vertex, fragment, "vec4", "shadowCoord", count);
        declareShadowSamplers(fragment, shaderInfo);
    }

    if (shaderInfo.hasScattering())
    {
        fragment << "uniform sampler2D transmittanceTex;" << endl;
        fragment << "uniform sampler3D scatterTex;" << endl;
    }

    if (isViewDependent)
    {
        declareUniform(fragment, "vec3", "eyePosition");
    }

    if (phong)
    {
        // These values aren't required for purely Lambertian reflectance
        declareUniform(fragment, "vec3", "specularColor");
        declareUniform(fragment, "float", "phongExponent");
    }

    if (shaderInfo.hasScattering())
    {
        declareUniform(fragment, "float", "atmosphereRadius");
        declareUniform(fragment, "float", "planetRadius");
        declareUniform(fragment, "vec3", "atmosphereColor");
    }

    if (shaderInfo.hasFresnelFalloff())
    {
        declareUniform(fragment, "float", "fresnelReflectance");
    }

    // Light position in local space
    declareUniformArray(fragment, "vec3", "lightPosition", shaderInfo.lightCount());
    declareUniformArray(fragment, "vec3", "lightColor", shaderInfo.lightCount());
    declareUniformArray(fragment, "float", "lightAttenuation", shaderInfo.lightCount());

    if (hasEnvironmentMap || shaderInfo.hasOmniShadows())
    {
        declareUniform(fragment, "mat3", "objToWorldMat");
    }

    declareHelperFunctions(fragment, shaderInfo);

    vertex << "void main()" << endl;
    vertex << "{" << endl;
    if (shaderInfo.hasTextureCoord())
    {
        vertex << "    texCoord = gl_MultiTexCoord0.xy;" << endl;
    }
    if (shaderInfo.hasVertexColors() != 0)
    {
        vertex << "    vertexColor = gl_Color;" << endl;
    }
    if (hasTangents)
    {
        vertex << "    tangent = vtxTangent;" << endl;
    }
    vertex << "    normal = gl_Normal;" << endl;
    if (usesPosition)
    {
        // Note that this is the model space position
        vertex << "    position = gl_Vertex.xyz;" << endl;
    }

    // Output shadow coordinates for shaders that have shadows
    if (shaderInfo.hasShadows())
    {
        for (unsigned int i = 0; i < shaderInfo.shadowCount(); ++i)
        {
            vertex << "    shadowCoord[" << i << "] = shadowMatrix[" << i << "] * gl_Vertex;" << endl;
        }
    }

    // Position is always required
    vertex << "    gl_Position = ftransform();" << endl;

    vertex << "}" << endl;

    // Values used in fragment shader light calculation:
    //    N - surface normal (model space)
    //    V - view vector (model space)
    //    T - surface tangent
    //    B - surface bitangent
    //    Nsurf - the geometric surface normal (i.e. unperturbed by the
    //            normal map.)
    fragment << "void main()" << endl;
    fragment << "{" << endl;
    if (hasTangents)
    {
        fragment << "    vec3 Ngeom = normalize(normal);" << endl;
        fragment << "    vec3 T = normalize(tangent);" << endl;
        fragment << "    vec3 B = cross(T, Ngeom);" << endl;

        // Retrieve the normal from the normal texture
        if (shaderInfo.hasCompressedNormalMap())
        {
            // Compressed normal maps have the normal x and y stored in the alpha
            // and green channels; z is computed from as sqrt(1-x^2-y^2)
            fragment << "    vec2 mxy = texture2D(normalTex, texCoord).ag * 2.0 - 1.0;" << endl;
            fragment << "    vec3 m = vec3(mxy, sqrt(1.0 - dot(mxy, mxy)));" << endl;
        }
        else
        {
            fragment << "    vec3 m = normalize(texture2D(normalTex, texCoord).xyz * 2.0 - 1.0);" << endl;
        }

        // Map the normal from surface local space to model space
        fragment << "    vec3 N = m.x * T + m.y * B + m.z * Ngeom;" << endl;
    }
    else
    {
        fragment << "    vec3 N = normalize(normal);" << endl;
    }

    fragment << "    vec3 diffLight = ambientLight;" << endl;
    if (isViewDependent)
    {
        fragment << "   vec3 V = normalize(eyePosition - " << position(shaderInfo) << ");" << endl;
    }
    if (phong)
    {
        fragment << "   vec3 specLight = vec3(0.0);" << endl;
    }

    if (shaderInfo.hasScattering())
    {
        fragment << "vec3 sc;" << endl;  // scattering
        fragment << "vec3 sunAttenuation;" << endl;  // extinction
        fragment << "vec3 eyeAttenuation;" << endl;  // extinction
        fragment << "scattering(" << position(shaderInfo) << ", eyePosition, -V, lightPosition[0], sc, sunAttenuation, eyeAttenuation);" << endl;
    }

    // Loop over the light sources and accumulate the specular and diffuse
    // contributions from each.
    for (unsigned int light = 0; light < shaderInfo.lightCount(); ++light)
    {
        bool isPointLight = light > 0;

        fragment << "    {" << endl;
        string lightDirection;
        string lightPosition;
        if (!isPointLight)
        {
            // Light source zero is directional (i.e. effectively an infinite distance from the object)
            lightDirection = arrayIndex("lightPosition", light);
            lightPosition = lightDirection;
        }
        else
        {
            // Light source is a point source
            fragment << "        vec3 lightPos = " << arrayIndex("lightPosition", light) << " - " << position(shaderInfo) << ";" << endl;
            fragment << "        float dist2 = dot(lightPos, lightPos);" << endl;
            fragment << "        vec3 lightDir = lightPos / sqrt(dist2);" << endl;
            lightDirection = "lightDir";
            lightPosition = "lightPos";
            fragment << "        float lightIntensity = 1.0 / max(1.0, dist2 * " << arrayIndex("lightAttenuation", light) << ");" << endl;
        }

        fragment << "        float d = max(0.0, dot(N, " << lightDirection << "));" << endl;

        // Presently, a maximum of one directional shadow and three omnidirectional shadows are supported.
        if (!isPointLight && shaderInfo.shadowCount() != 0)
        {
            unsigned int shadowIndex = 0;
            fragment << "        float shadow = shadowPCF(shadowTex" << shadowIndex << ", shadowCoord[" << shadowIndex << "]);" << endl;
        }
        else if (isPointLight && light <= shaderInfo.omniShadowCount())
        {
            unsigned int shadowIndex = light - 1;
            fragment << "        float shadow = omniShadow(shadowCubeMap" << shadowIndex << ", objToWorldMat * " << lightPosition << ");" << endl;
        }
        else if (hasTangents)
        {
            // Compute the self-shadowing term to prevent steep areas of
            // a normal map from being lit when they should be shadowed by
            // the geometry. If shadows are enabled, this is handled by the
            // shadow map.
            fragment << "        float shadow = clamp(dot(Ngeom, " << lightDirection << ") * 8.0, 0.0, 1.0);\n";
        }
        else
        {
            // No need for shadow term
            fragment << "        float shadow = 1.0;" << endl;
        }

        // Fold light intensity into shadow term
        if (isPointLight)
        {
            fragment << "        shadow *= lightIntensity;" << endl;
        }

        string lightColor = arrayIndex("lightColor", light);
        if (shaderInfo.hasScattering() && light == 0)
        {
            lightColor = lightColor + " * sunAttenuation";
        }
        fragment << "        diffLight += (shadow * d) * " << lightColor << ";" << endl;

        if (phong)
        {
            // Compute the half angle vector
            fragment << "        vec3 H = normalize(" << lightDirection << " + V);" << endl;
            fragment << "        float s = pow(max(0.0, dot(H, N)), phongExponent);" << endl;

            // Self-shadowing term necessary to prevent the Phong highlight from bleeding
            // onto geometry that's facing away from the light source.
            if (!hasTangents)
            {
                fragment << "        s *= clamp(d * 8.0, 0.0, 1.0);" << endl;
            }

            if (shaderInfo.hasFresnelFalloff())
            {
                fragment << "        s *= " << fresnelTerm("dot(H, V)") << ";" << endl;
            }

            fragment << "       specLight += (shadow * s) * " << lightColor << ";" << endl;
        }

        fragment << "    }" << endl;
    }

    fragment << "    vec4 diffuse = vec4(color, opacity);" << endl;

    if (phong)
    {
        fragment << "    vec3 specular = specularColor;" << endl;
    }

    // The specular modifier (if present) is either a color from the specular texture or the
    // alpha channel of the diffuse texture.
    bool hasSpecularModifier = false;
    if (shaderInfo.hasTexture(ShaderInfo::SpecularTexture) && phong)
    {
        fragment << "    vec3 specularModifier = texture2D(specularTex, texCoord).rgb;" << endl;
        hasSpecularModifier = true;
    }

    if (shaderInfo.hasTexture(ShaderInfo::DiffuseTexture))
    {
        if (shaderInfo.hasAlphaTexture())
        {
            fragment << "    diffuse.a *= texture2D(diffuseTex, texCoord).a;" << endl;
        }
        else
        {
            if (phong && shaderInfo.hasSpecularMaskInDiffuseAlpha())
            {
                fragment << "    vec4 texColor = texture2D(diffuseTex, texCoord);" << endl;
                fragment << "    diffuse.rgb *= texColor.rgb;" << endl;
                fragment << "    float specularModifier = texColor.a;" << endl;
                hasSpecularModifier = true;
            }
            else
            {
                fragment << "    diffuse *= texture2D(diffuseTex, texCoord);" << endl;
            }
        }
    }

    if (hasSpecularModifier)
    {
        fragment << "    specular *= specularModifier;" << endl;
    }

    if (shaderInfo.hasVertexColors())
    {
        fragment << "    diffuse *= vertexColor;" << endl;
    }

    string colorSum = "diffuse.rgb * diffLight";
    if (phong)
    {
        colorSum += " + specular * specLight";
    }

    if (shaderInfo.hasTexture(ShaderInfo::ReflectionTexture))
    {
        // TODO: Eliminate this matrix multiply by doing lighting in world coordinates
        // instead of object coordinates.
        fragment << "    vec3 R = objToWorldMat * reflect(-V, N); R.z = -R.z;" << endl;
        colorSum += " + textureCube(reflectionTex, R).rgb";
        if (phong)
        {
            colorSum += " * specular";
        }

        if (shaderInfo.hasFresnelFalloff())
        {
            fragment << "    float f = " << fresnelTerm("dot(V, N)") << ";" << endl;
            colorSum += " * f";
        }
    }

    string alphaSum = "diffuse.a";
    if (shaderInfo.hasScattering())
    {
        colorSum = "(" + colorSum + ") * eyeAttenuation * 1.0 + sc * 2.0";
        alphaSum = "diffuse.a + (1.0 - eyeAttenuation.g)";
    }

    fragment << "    gl_FragColor = vec4(" << colorSum << ", " << alphaSum << ");" << endl;

    fragment << "}" << endl;
}


// Generate a shader for a volume of partculates that scatters light. This is currently
// used only for rendering planetary rings.
static void generateParticulateShader(ostream& vertex, ostream& fragment, const ShaderInfo& shaderInfo)
{
    // Interpolated variables
    declareVarying(vertex, fragment, "vec3", "position"); // position in local space

    // Fragment shader constants
    declareSamplers(fragment, shaderInfo.textures());
    declareUniform(fragment, "vec3", "color");
    declareUniform(fragment, "float", "opacity");
    declareUniform(fragment, "vec3", "ambientLight");

    if (shaderInfo.hasShadows())
    {
        unsigned int count = shaderInfo.shadowCount();
        declareUniformArray(vertex, "mat4", "shadowMatrix", count);
        declareVaryingArray(vertex, fragment, "vec4", "shadowCoord", count);
        declareShadowSamplers(fragment, shaderInfo);
    }

    if (shaderInfo.hasScattering())
    {
        fragment << "uniform sampler2D transmittanceTex;" << endl;
        fragment << "uniform sampler3D scatterTex;" << endl;
    }

    declareUniform(fragment, "vec3", "eyePosition");

    if (shaderInfo.hasScattering())
    {
        declareUniform(fragment, "float", "atmosphereRadius");
        declareUniform(fragment, "float", "planetRadius");
        declareUniform(fragment, "vec3", "atmosphereColor");
    }

    // Light position in local space
    declareUniformArray(fragment, "vec3", "lightPosition", shaderInfo.lightCount());
    declareUniformArray(fragment, "vec3", "lightColor", shaderInfo.lightCount());

    declareHelperFunctions(fragment, shaderInfo);

    vertex << "void main()" << endl;
    vertex << "{" << endl;
    if (shaderInfo.hasTextureCoord())
    {
        vertex << "    texCoord = gl_MultiTexCoord0.xy;" << endl;
    }
    if (shaderInfo.hasVertexColors() != 0)
    {
        vertex << "    vertexColor = gl_Color;" << endl;
    }

    // Note that this is the model space position
    vertex << "    position = gl_Vertex.xyz;" << endl;

    // Output shadow coordinates for shaders that have shadows
    if (shaderInfo.hasShadows())
    {
        for (unsigned int i = 0; i < shaderInfo.shadowCount(); ++i)
        {
            vertex << "    shadowCoord[" << i << "] = shadowMatrix[" << i << "] * gl_Vertex;" << endl;
        }
    }

    // Position is always required
    vertex << "    gl_Position = ftransform();" << endl;

    vertex << "}" << endl;

    // Values used in fragment shader light calculation:
    //    V - view vector (model space)
    fragment << "void main()" << endl;
    fragment << "{" << endl;

    fragment << "    vec3 diffLight = ambientLight;" << endl;
    fragment << "    vec3 V = normalize(eyePosition - " << position(shaderInfo) << ");" << endl;

    if (shaderInfo.hasScattering())
    {
        fragment << "vec3 sc;" << endl;  // scattering
        fragment << "vec3 sunAttenuation;" << endl;  // extinction
        fragment << "vec3 eyeAttenuation;" << endl;  // extinction
        fragment << "scattering(" << position(shaderInfo) << ", eyePosition, -V, lightPosition[0], sc, sunAttenuation, eyeAttenuation);" << endl;
    }

    // Loop over the light sources and accumulate the contributions from each.
    for (unsigned int light = 0; light < shaderInfo.lightCount(); ++light)
    {
        fragment << "    {" << endl;
        string lightPosition;
        if (light == 0)
        {
            // Light source zero is directional (i.e. effectively an infinite distance from the object)
            lightPosition = arrayIndex("lightPosition", light);
        }
        else
        {
            // Light source is a point source
            lightPosition = "lightPos";
            fragment << "        vec3 lightPos = " << arrayIndex("lightPosition", light) << " - " << position(shaderInfo) << ";" << endl;
            fragment << "        lightPos = normalize(lightPos);" << endl;
        }

        if (shaderInfo.shadowCount() > light)
        {
             fragment << "        float shadow = shadowPCF(shadowTex" << light << ", shadowCoord[" << light << "]);" << endl;
        }
        else
        {
            // No need for shadow term
            fragment << "        float shadow = 1.0;" << endl;
        }

        // Henyey-Greenstein phase function; g should be a shader parameter
        fragment << "        float cosLV = dot(" << lightPosition << ", V);\n";
        fragment << "        float g = 0.3;\n";
        fragment << "        float ph = 0.25 * (1.0 - g * g) * pow(1.0 + g * g - 2.0 * g * cosLV, -1.5);" << endl;

        string lightColor = arrayIndex("lightColor", light);
        if (shaderInfo.hasScattering() && light == 0)
        {
            lightColor = lightColor + " * sunAttenuation";
        }
        fragment << "        diffLight += shadow * ph * " << lightColor << ";" << endl;

        fragment << "    }" << endl;
    }

    fragment << "    vec4 diffuse = vec4(color, opacity);" << endl;

    if (shaderInfo.hasTexture(ShaderInfo::DiffuseTexture))
    {
        if (shaderInfo.hasAlphaTexture())
        {
            fragment << "    diffuse.a *= texture2D(diffuseTex, texCoord).a;" << endl;
        }
        else
        {
            fragment << "    diffuse *= texture2D(diffuseTex, texCoord);" << endl;
        }
    }

    if (shaderInfo.hasVertexColors())
    {
        fragment << "    diffuse *= vertexColor;" << endl;
    }

    string colorSum = "diffuse.rgb * diffLight";
    string alphaSum = "diffuse.a";
    if (shaderInfo.hasScattering())
    {
        colorSum = "(" + colorSum + ") * eyeAttenuation * 1.0 + sc * 2.0";
        alphaSum = "diffuse.a + (1.0 - eyeAttenuation.g)";
    }

    fragment << "    gl_FragColor = vec4(" << colorSum << ", " << alphaSum << ");" << endl;

    fragment << "}" << endl;
}


GLShaderProgram*
ShaderBuilder::generateShader(const ShaderInfo& shaderInfo) const
{
    ostringstream vertex;
    ostringstream fragment;

    // Version header (disabled for now)
    // fragment << "#version 120" << endl;

    if (shaderInfo.hasTextureCoord())
    {
        declareVarying(vertex, fragment, "vec2", "texCoord");
    }

    if (shaderInfo.hasVertexColors())
    {
        declareVarying(vertex, fragment, "vec4", "vertexColor");
    }

    if (shaderInfo.lightCount() == 0 || shaderInfo.reflectanceModel() == ShaderInfo::Emissive)
    {
        generateUnlitShader(vertex, fragment, shaderInfo);
    }
    else if (shaderInfo.reflectanceModel() == ShaderInfo::Particulate)
    {
        generateParticulateShader(vertex, fragment, shaderInfo);
    }
    else
    {
        generateBlinnPhongShader(vertex, fragment, shaderInfo);
    }

    VESTA_LOG("Creating shader:  model: %u, textures 0x%x, lights: %u, shadows: %u/%u, scattering: %d, fresnel: %d, vertexColors: %d",
              (int) shaderInfo.reflectanceModel(),
              shaderInfo.textures(),
              shaderInfo.lightCount(),
              shaderInfo.shadowCount(), shaderInfo.omniShadowCount(),
              shaderInfo.hasScattering() ? 1 : 0,
              shaderInfo.hasFresnelFalloff() ? 1 : 0,
              shaderInfo.hasVertexColors() ? 1 : 0);

    // Compile the vertex shader
    GLShader* vertexShader = new GLShader(GLShader::VertexStage);
    if (!vertexShader->compile(vertex.str()))
    {
        VESTA_WARNING("Error creating vertex shader:");
        VESTA_WARNING("Error message(s):\n%s", vertexShader->compileLog().c_str());
        VESTA_WARNING("Shader source:\n%s", vertex.str().c_str());
        delete vertexShader;
        return NULL;
    }
    else if (!vertexShader->compileLog().empty())
    {
        VESTA_LOG("Vertex shader compile messages:\n%s", vertexShader->compileLog().c_str());
    }

    // Compile the fragment shader
    GLShader* fragmentShader = new GLShader(GLShader::FragmentStage);
    if (!fragmentShader->compile(fragment.str()))
    {
        VESTA_WARNING("Error creating fragment shader:");
        VESTA_WARNING("Error message(s):\n%s", fragmentShader->compileLog().c_str());
        VESTA_WARNING("Shader source:\n%s", fragment.str().c_str());
        delete vertexShader;
        delete fragmentShader;
        return NULL;
    }
    else if (!fragmentShader->compileLog().empty())
    {
        VESTA_LOG("Vertex shader compile messages:\n%s", fragmentShader->compileLog().c_str());
    }

#if DUMP_SHADER_SOURCE
    VESTA_LOG("Vertex shader source:\n%s", vertex.str().c_str());
    VESTA_LOG("Fragment shader source:\n%s", fragment.str().c_str());
#endif

    // Attach the vertex and fragment shaders
    GLShaderProgram* shaderProgram = new GLShaderProgram();
    shaderProgram->addShader(vertexShader);
    shaderProgram->addShader(fragmentShader);

    // Bind vertex attributes
    if (shaderInfo.hasTexture(ShaderInfo::NormalTexture))
    {
        shaderProgram->bindAttribute("vtxTangent", TangentAttributeLocation);
    }

    // Link the shader program
    if (!shaderProgram->link())
    {
        VESTA_WARNING("Error linking shader program:");
        VESTA_WARNING("Error message(s):\n%s", shaderProgram->log().c_str());
        delete shaderProgram;
        // vertex and fragment shaders automatically deleted along with program
        return NULL;
    }
    else if (!shaderProgram->log().empty())
    {
        VESTA_LOG("Shader program link messages:\n%s", shaderProgram->log().c_str());
    }

    return shaderProgram;
}
