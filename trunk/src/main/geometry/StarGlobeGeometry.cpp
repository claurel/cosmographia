// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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

#include "StarGlobeGeometry.h"
#include <vesta/RenderContext.h>
#include <vesta/WorldGeometry.h>
#include <vesta/glhelp/GLShaderProgram.h>
#include <string>

using namespace vesta;
using namespace Eigen;


//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//
static const char* SimplexNoise3DSource =
        "#version 120                                    \n"
        "vec3 mod289(vec3 x) {\n"
        "    return x - floor(x * (1.0 / 289.0)) * 289.0;\n"
        "}\n"
        "\n"
        "vec4 mod289(vec4 x) {\n"
        "    return x - floor(x * (1.0 / 289.0)) * 289.0;\n"
        "}\n"
        "\n"
        "vec4 permute(vec4 x) {\n"
        "    return mod289(((x*34.0)+1.0)*x);\n"
        "}\n"
        "\n"
        "vec4 taylorInvSqrt(vec4 r)\n"
        "{\n"
        "    return 1.79284291400159 - 0.85373472095314 * r;\n"
        "}\n"
        "\n"
        "float snoise(vec3 v)\n"
        "{ \n"
        "    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;\n"
        "    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);\n"
        "\n"
        "// First corner\n"
        "    vec3 i  = floor(v + dot(v, C.yyy) );\n"
        "    vec3 x0 =   v - i + dot(i, C.xxx) ;\n"
        "\n"
        "// Other corners\n"
        "    vec3 g = step(x0.yzx, x0.xyz);\n"
        "    vec3 l = 1.0 - g;\n"
        "    vec3 i1 = min( g.xyz, l.zxy );\n"
        "    vec3 i2 = max( g.xyz, l.zxy );\n"
        "\n"
        "    //   x0 = x0 - 0.0 + 0.0 * C.xxx;\n"
        "    //   x1 = x0 - i1  + 1.0 * C.xxx;\n"
        "    //   x2 = x0 - i2  + 2.0 * C.xxx;\n"
        "    //   x3 = x0 - 1.0 + 3.0 * C.xxx;\n"
        "    vec3 x1 = x0 - i1 + C.xxx;\n"
        "    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y\n"
        "    vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y\n"
        "\n"
        "// Permutations\n"
        "    i = mod289(i); \n"
        "    vec4 p = permute( permute( permute( \n"
        "                                   i.z + vec4(0.0, i1.z, i2.z, 1.0 ))\n"
        "                               + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) \n"
        "                      + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));\n"
        "\n"
        "// Gradients: 7x7 points over a square, mapped onto an octahedron.\n"
        "// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)\n"
        "    float n_ = 0.142857142857; // 1.0/7.0\n"
        "    vec3  ns = n_ * D.wyz - D.xzx;\n"
        "\n"
        "    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)\n"
        "\n"
        "    vec4 x_ = floor(j * ns.z);\n"
        "    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)\n"
        "\n"
        "    vec4 x = x_ *ns.x + ns.yyyy;\n"
        "    vec4 y = y_ *ns.x + ns.yyyy;\n"
        "    vec4 h = 1.0 - abs(x) - abs(y);\n"
        "\n"
        "    vec4 b0 = vec4( x.xy, y.xy );\n"
        "    vec4 b1 = vec4( x.zw, y.zw );\n"
        "\n"
        "    //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;\n"
        "    //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;\n"
        "    vec4 s0 = floor(b0)*2.0 + 1.0;\n"
        "    vec4 s1 = floor(b1)*2.0 + 1.0;\n"
        "    vec4 sh = -step(h, vec4(0.0));\n"
        "\n"
        "    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;\n"
        "    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;\n"
        "\n"
        "    vec3 p0 = vec3(a0.xy,h.x);\n"
        "    vec3 p1 = vec3(a0.zw,h.y);\n"
        "    vec3 p2 = vec3(a1.xy,h.z);\n"
        "    vec3 p3 = vec3(a1.zw,h.w);\n"
        "\n"
        "//Normalise gradients\n"
        "    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));\n"
        "    p0 *= norm.x;\n"
        "    p1 *= norm.y;\n"
        "    p2 *= norm.z;\n"
        "    p3 *= norm.w;\n"
        "\n"
        "// Mix final noise value\n"
        "    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);\n"
        "    m = m * m;\n"
        "    return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), \n"
        "                                  dot(p2,x2), dot(p3,x3) ) );\n"
        "}\n";


static const char* SunVertexShaderSource =
"#version 120                     \n"
"uniform float time;              \n"
"uniform mat4 noiseTransform1;    \n"
"uniform mat4 noiseTransform2;    \n"
"varying vec3 position1;          \n"
"varying vec3 position2;          \n"
"\n"
"void main()                      \n"
"{                                \n"
"    position1 = (noiseTransform1 * gl_Vertex).xyz;    \n"
"    position2 = (noiseTransform2 * gl_Vertex).xyz;    \n"
"    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;                  \n"
"}                                                                            \n"
;

static const char* SunFragmentShaderSource =
"varying vec3 position1;                         \n"
"varying vec3 position2;                         \n"
"uniform vec3 color1;                            \n"
"uniform vec3 color2;                            \n"
"void main()                                     \n"
"{                                               \n"
        "    float a = snoise(position1 * 3.5) * 0.17 + abs(snoise(position1 * 20.0)) + abs(snoise(position2 * 43.0) * 0.5) + pow(snoise(position2 * 89.2), 2.0) * 0.25;\n"
"    gl_FragColor = vec4(mix(color1, color2, a), 1.0);                             \n"
"}                                               \n"
;

//        "    float a = abs(snoise(position1 * 20.0)) + abs(snoise(position2 * 43.0) * 0.5) + pow(snoise(position2 * 89.2), 2.0) * 0.25;\n"

bool StarGlobeGeometry::ms_shaderCompiled = false;
counted_ptr<GLShaderProgram> StarGlobeGeometry::ms_starShader;


StarGlobeGeometry::StarGlobeGeometry()
{
    m_globe = new WorldGeometry();
    m_globe->setEmissive(true);
}


StarGlobeGeometry::~StarGlobeGeometry()
{
}


float
StarGlobeGeometry::boundingSphereRadius() const
{
    return m_globe->boundingSphereRadius();
}


void
StarGlobeGeometry::setEllipsoidAxes(const Eigen::Vector3f& axes)
{
    m_globe->setEllipsoid(axes);
}


Eigen::Vector3f
StarGlobeGeometry::ellipsoidAxes() const
{
    return m_globe->ellipsoidAxes();
}


AlignedEllipsoid
StarGlobeGeometry::ellipsoid() const
{
    return m_globe->ellipsoid();
}


void
StarGlobeGeometry::render(RenderContext& rc, double clock) const
{
    if (rc.pass() == RenderContext::TranslucentPass)
    {
        return;
    }

    if (rc.shaderCapability() != RenderContext::FixedFunction)
    {
        // Create the star shaders if they haven't already been compiled
        if (!ms_shaderCompiled)
        {
            std::string fragmentShaderSource = std::string(SimplexNoise3DSource) + SunFragmentShaderSource;
            ms_starShader = GLShaderProgram::CreateShaderProgram(SunVertexShaderSource, fragmentShaderSource);
            ms_shaderCompiled = true;
#ifdef VESTA_OGLES2
            // TODO: Need to make VESTA bind these automatically
            if (m_swarmShader.isValid())
            {
                m_swarmShader->bindAttribute(ShaderBuilder::PositionAttribute, ShaderBuilder::PositionAttributeLocation);
                m_swarmShader->link();
            }
#endif
        }
    }

    if (ms_starShader.isValid())
    {
        rc.enableCustomShader(ms_starShader.ptr());
        ms_starShader->bind();

        Transform3f noiseTransform1(Transform3f::Identity());
        Transform3f noiseTransform2(Transform3f::Identity());

        double t = clock * 0.0001;
        noiseTransform1.translate(Vector3f((float) (10.0 * sin(t * 1.1 + 3.4)),
                                           (float) (13.0 * sin(t * 1.3 + 1.91)),
                                           (float) ( 8.3 * sin(t * 2.25 + 0.44))));
        //noiseTransform1.scale(1.0 + sin(clock * 0.019 + 5.32));
        noiseTransform2.translate(Vector3f((float) ( 9.0 * sin(t * 1.9 + 0.4)),
                                           (float) (15.0 * sin(t * 4.3 + 0.91)),
                                           (float) (10.3 * sin(t * 3.4 + 2.44))));
        //noiseTransform2.scale(1.0 + sin(clock * 0.0039 + 2.08));
        ms_starShader->setConstant("noiseTransform1", noiseTransform1.matrix());
        ms_starShader->setConstant("noiseTransform2", noiseTransform2.matrix());
        ms_starShader->setConstant("color1", Vector3f(0.6f, 0.25f, 0.13f));
        ms_starShader->setConstant("color2", Vector3f(1.6f, 1.05f, 0.73f));
        m_globe->render(rc, clock);

        rc.disableCustomShader();
    }

#if 0
    Transform3f invModelView = Transform3f(rc.modelview().inverse());
    Vector3f cameraPos = invModelView * Vector3f::Zero();

    float horizonDistance = 0.0f;
    if (approxAltitude > 0.0f)
    {
        float r = m_ellipsoidAxes.maxCoeff() * 0.5f;
        horizonDistance = sqrt((2 * r + approxAltitude) * approxAltitude);
    }

    Frustum viewFrustum = rc.frustum();
    float farDistance = max(viewFrustum.nearZ, min(horizonDistance, viewFrustum.farZ));
    CullingPlaneSet frustumPlanes;
    for (unsigned int i = 0; i < 4; ++i)
    {
        frustumPlanes.planes[i] = Hyperplane<float, 3>(viewFrustum.planeNormals[i].cast<float>(), 0.0f);
        frustumPlanes.planes[i].coeffs() = rc.modelview().matrix().transpose() * cullingPlanes.planes[i].coeffs();
    }
    frustumPlanes.planes[4].coeffs() = modelviewTranspose * Vector4f(0.0f, 0.0f, -1.0f, -viewFrustum.nearZ);
    frustumPlanes.planes[5].coeffs() = modelviewTranspose * Vector4f(0.0f, 0.0f,  1.0f, farDistance);

    rc.pushModelView();
    rc.scaleModelView(m_ellipsoidAxes * 0.5f);
#endif
}


