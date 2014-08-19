// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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

#include "CmodLoader.h"
#include <QDataStream>
#include <QVector>

using namespace vesta;


CmodLoader::CmodLoader(QIODevice *in, TextureMapLoader* textureLoader) :
    m_inputStream(NULL),
    m_hasError(false),
    m_textureLoader(textureLoader)
{
    m_inputStream = new QDataStream(in);
    m_inputStream->setByteOrder(QDataStream::LittleEndian);
    m_inputStream->setFloatingPointPrecision(QDataStream::SinglePrecision);
}


CmodLoader::~CmodLoader()
{
    delete m_inputStream;
}


void
CmodLoader::setError(const QString& errorMessage)
{
    m_errorMessage = errorMessage;
    m_hasError = true;
}


vesta::Material*
CmodLoader::loadMaterial()
{
    Material* material = new Material();
    material->setDiffuse(Spectrum::Black());
    material->setSpecular(Spectrum::Black());
    material->setEmission(Spectrum::Black());
    material->setOpacity(1.0f);
    material->setBrdf(Material::Lambert);
    material->setPhongExponent(1.0f);

    bool done = false;
    while (!done && !error())
    {
        CmodToken token = readCmodToken();
        switch (token)
        {
        case CmodIlluminationModel:
        {
            quint16 illum = 0;
            *m_inputStream >> illum;

            switch (illum)
            {
            case 0:
                material->setBrdf(Material::Lambert);
                break;
            case 2:
                material->setBrdf(Material::BlinnPhongReflective);
                break;
            case 1:
                // Lunar-lambert model currently unsupported; just treat
                // it as purely Lambertian now
                material->setBrdf(Material::Lambert);
                break;
            default:
                setError("Unknown illumination model");
                break;
            }
        }
        break;

        case CmodDiffuse:
        {
            Spectrum diffuseColor;
            if (!readColorProperty(&diffuseColor))
            {
                setError("Error reading diffuse color in material");
            }
            else
            {
                material->setDiffuse(diffuseColor);
            }
        }
        break;

        case CmodSpecular:
        {
            Spectrum specularColor;
            if (!readColorProperty(&specularColor))
            {
                setError("Error reading specular color in material");
            }
            else
            {
                material->setSpecular(specularColor);
            }
        }
        break;

        case CmodSpecularPower:
        {
            float specularPower = 0.0f;
            if (!readFloat1Property(&specularPower))
            {
                setError("Error reading specular power");
            }
            else
            {
                material->setPhongExponent(specularPower);
            }
        }
        break;

        case CmodEmissive:
        {
            Spectrum emissiveColor;
            if (!readColorProperty(&emissiveColor))
            {
                setError("Error reading emissive color in material");
            }
            else
            {
                material->setEmission(emissiveColor);
            }
        }
        break;

        case CmodOpacity:
        {
            float opacity = 0.0f;
            if (!readFloat1Property(&opacity))
            {
                setError("Error reading material opacity");
            }
            else
            {
                material->setOpacity(opacity);
            }
        }
        break;

        case CmodTexture:
        {
            quint16 textureType = 0;
            *m_inputStream >> textureType;

            QString textureSource;
            if (!readStringProperty(&textureSource))
            {
                setError("Error reading texture source");
            }
            else
            {
                if (m_textureLoader.isValid())
                {
                    TextureProperties texProps(TextureProperties::Wrap);
                    if (textureType == 1)
                    {
                        if (textureSource.toLower().endsWith("dxt5nm") ||
                            textureSource.toLower().endsWith("dds"))
                        {
                            texProps.usage = TextureProperties::CompressedNormalMap;
                        }
                    }

                    counted_ptr<TextureMap> texture(m_textureLoader->loadTexture(textureSource.toUtf8().constData(), texProps));
                    switch (textureType)
                    {
                    case 0:
                        material->setBaseTexture(texture.ptr());
                        break;
                    case 1:
                        material->setNormalTexture(texture.ptr());
                        break;
                    case 2:
                        material->setSpecularTexture(texture.ptr());
                        break;
                    case 3:
                        // Emissive texture not supported
                        break;
                    default:
                        // Unsupported texture type
                        break;
                    }
                }
            }
        }
        break;

        case CmodBlend:
        {
            quint16 blendMode = 0;
            *m_inputStream >> blendMode;
            switch (blendMode)
            {
            case 0:
                material->setBlendMode(Material::AlphaBlend);
                break;
            case 1:
                material->setBlendMode(Material::AdditiveBlend);
                break;
            case 2:
                material->setBlendMode(Material::PremultipliedAlphaBlend);
                break;
            default:
                setError("Unknown blend mode");
                break;
            }
        }
        break;

        case CmodEndMaterial:
            done = true;
            break;

        default:
            setError(QString("Unrecognized token %1 in material").arg(int(token)));
            break;
        }
    }

    if (error())
    {
        delete material;
        material = NULL;
    }

    return material;
}


VertexArray*
CmodLoader::loadVertexArray(const VertexSpec& spec)
{
    if (readCmodToken() != CmodVertices)
    {
        setError("Vertex data missing");
        return NULL;
    }

    quint32 vertexCount = 0;
    *m_inputStream >> vertexCount;

    if (vertexCount == 0)
    {
        setError("Vertex data block has zero vertices");
        return NULL;
    }

    unsigned int dataSize = vertexCount * spec.size();
    char* vertexData = new char[dataSize];
    if (!vertexData)
    {
        setError("Error allocating vertex data for mesh (out of memory)");
        return NULL;
    }

    unsigned int vertexOffset = 0;
    unsigned int vertexSize = spec.size();

    for (unsigned int vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex)
    {
        for (unsigned int attributeIndex = 0; attributeIndex < spec.attributeCount(); ++attributeIndex)
        {
            unsigned int offset = vertexOffset + spec.attributeOffset(attributeIndex);
            switch (spec.attribute(attributeIndex).format())
            {
            case VertexAttribute::Float1:
                *m_inputStream >> *(reinterpret_cast<float*>(vertexData + offset));
                break;
            case VertexAttribute::Float2:
                *m_inputStream >> *(reinterpret_cast<float*>(vertexData + offset));
                *m_inputStream >> *(reinterpret_cast<float*>(vertexData + offset + 4));
                break;
            case VertexAttribute::Float3:
                *m_inputStream >> *(reinterpret_cast<float*>(vertexData + offset));
                *m_inputStream >> *(reinterpret_cast<float*>(vertexData + offset + 4));
                *m_inputStream >> *(reinterpret_cast<float*>(vertexData + offset + 8));
                break;
            case VertexAttribute::Float4:
                *m_inputStream >> *(reinterpret_cast<float*>(vertexData + offset));
                *m_inputStream >> *(reinterpret_cast<float*>(vertexData + offset + 4));
                *m_inputStream >> *(reinterpret_cast<float*>(vertexData + offset + 8));
                *m_inputStream >> *(reinterpret_cast<float*>(vertexData + offset + 12));
                break;
            case VertexAttribute::UByte4:
                m_inputStream->readRawData(vertexData + offset, 4);
                break;
            default:
                break;
            }
        }

        vertexOffset += vertexSize;
    }

    return new VertexArray(vertexData, vertexCount, spec);
}


Submesh*
CmodLoader::loadSubmesh()
{
    VertexSpec* vertexSpec = loadVertexSpec();
    if (!vertexSpec)
    {
        return NULL;
    }

    VertexArray* vertexArray = loadVertexArray(*vertexSpec);
    if (!vertexArray)
    {
        delete vertexSpec;
        return NULL;
    }

    Submesh* submesh = new Submesh(vertexArray);

    bool done = false;
    while (!done && !error())
    {
        quint16 token = 0;
        *m_inputStream >> token;

        if (token == CmodEndMesh)
        {
            done = true;
        }
        else
        {
            unsigned int materialIndex = 0;
            PrimitiveBatch* primitives = loadPrimitiveBatch(token, vertexArray->count(), &materialIndex);
            if (primitives)
            {
                submesh->addPrimitiveBatch(primitives, materialIndex);
            }
        }
    }

    if (error())
    {
        delete submesh;
        submesh = NULL;
    }

    return submesh;
}


PrimitiveBatch*
CmodLoader::loadPrimitiveBatch(unsigned int primitiveTypeToken, unsigned int vertexCount, unsigned int* materialIndex)
{
    PrimitiveBatch::PrimitiveType primType = PrimitiveBatch::Triangles;
    unsigned int minIndexCount = 1;

    switch (primitiveTypeToken)
    {
    case 0:
        primType = PrimitiveBatch::Triangles;
        minIndexCount = 3;
        break;
    case 1:
        primType = PrimitiveBatch::TriangleStrip;
        minIndexCount = 3;
        break;
    case 2:
        primType = PrimitiveBatch::TriangleFan;
        minIndexCount = 3;
        break;
    case 3:
        primType = PrimitiveBatch::Lines;
        minIndexCount = 2;
        break;
    case 4:
        primType = PrimitiveBatch::LineStrip;
        minIndexCount = 2;
        break;
    case 5:
        primType = PrimitiveBatch::Points;
        minIndexCount = 1;
        break;
    case 6:
        primType = PrimitiveBatch::Points;
        minIndexCount = 1;
        break;
    default:
        setError("Unrecognized primitive group");
        break;
    }

    quint32 materialIndex32 = 0;
    quint32 indexCount = 0;
    *m_inputStream >> materialIndex32 >> indexCount;

    // In a CMOD file, the material index -1 (0xffffffff unsigned) indicates that the default
    // material should be used. The loader always places the default material at index 0. All
    // material indices therefore need to be incremented by one.
    *materialIndex = materialIndex32 + 1;

    if (indexCount < minIndexCount ||
        (primType == PrimitiveBatch::Triangles && indexCount % 3 != 0) ||
        (primType == PrimitiveBatch::Lines && indexCount % 2 != 0))
    {
        setError("Bad number of indices for primitive type");
    }

    v_uint32 primitiveCount = 0;
    switch (primType)
    {
    case PrimitiveBatch::Triangles:
        primitiveCount = indexCount / 3;
        break;
    case PrimitiveBatch::TriangleStrip:
        primitiveCount = indexCount - 2;
        break;
    case PrimitiveBatch::TriangleFan:
        primitiveCount = indexCount - 2;
        break;
    case PrimitiveBatch::Lines:
        primitiveCount = indexCount / 2;
        break;
    case PrimitiveBatch::LineStrip:
        primitiveCount = indexCount - 1;
        break;
    case PrimitiveBatch::Points:
        primitiveCount = indexCount;
        break;
    default:
        break;
    }

    v_uint32* indices = NULL;
    if (!error())
    {
        indices = new v_uint32[indexCount];
        if (!indices)
        {
            setError("Error allocating index data for mesh (out of memory)");
        }
    }

    if (!error())
    {
        for (unsigned int i = 0; i < indexCount; ++i)
        {
            *m_inputStream >> indices[i];
            if (indices[i] >= vertexCount)
            {
                setError(QString("Vertex index out of range (index %1, vertex count %2)").arg(indices[i], vertexCount));
                delete[] indices;
                indices = NULL;
                break;
            }
        }
    }

    PrimitiveBatch* primitives = NULL;
    if (!error())
    {
        primitives = new PrimitiveBatch(primType, indices, primitiveCount);
    }
    else
    {
        delete primitives;
        primitives = NULL;
    }

    delete[] indices;

    return primitives;
}


VertexSpec*
CmodLoader::loadVertexSpec()
{
    if (readCmodToken() != CmodVertexDesc)
    {
        setError("Missing vertex description in submesh");
        return NULL;
    }

    QVector<VertexAttribute> attributes;
    bool done = false;

    while (!done && !error())
    {
        quint16 token = 0;
        *m_inputStream >> token;
        if (token == CmodEndVertexDesc)
        {
            done = true;
        }
        else
        {
            VertexAttribute::Semantic semantic = VertexAttribute::InvalidAttributeSemantic;
            switch (token)
            {
            case 0:
                semantic = VertexAttribute::Position;
                break;
            case 1:
                semantic = VertexAttribute::Color;
                break;
            case 2:
                setError("color1 is not a supported vertex attribute");
                break;
            case 3:
                semantic = VertexAttribute::Normal;
                break;
            case 4:
                semantic = VertexAttribute::Tangent;
                break;
            case 5:
                semantic = VertexAttribute::TextureCoord;
                break;
            case 6:
                setError("texture1 is not a supported vertex attribute");
                break;
            case 7:
                setError("texture2 is not a supported vertex attribute");
                break;
            case 8:
                setError("texture3 is not a supported vertex attribute");
                break;
            case 9:
                semantic = VertexAttribute::PointSize;
                break;
            default:
                setError("Unrecognized vertex attribute semantic");
                break;
            }

            VertexAttribute::Format format = VertexAttribute::InvalidAttributeFormat;
            quint16 formatIndex = 0;
            *m_inputStream >> formatIndex;
            switch (formatIndex)
            {
            case 0:
                format = VertexAttribute::Float1;
                break;
            case 1:
                format = VertexAttribute::Float2;
                break;
            case 2:
                format = VertexAttribute::Float3;
                break;
            case 3:
                format = VertexAttribute::Float4;
                break;
            case 4:
                format = VertexAttribute::UByte4;
                break;
            default:
                setError("Unrecognized vertex attribute format");
                break;
            }

            if (format != VertexAttribute::InvalidAttributeFormat &&
                semantic != VertexAttribute::InvalidAttributeSemantic)
            {
                attributes << VertexAttribute(semantic, format);
            }
        }
    }

    if (attributes.isEmpty())
    {
        setError("Submesh has an empty vertex description");
        return NULL;
    }

    return new VertexSpec(attributes.size(), attributes.data());
}


MeshGeometry*
CmodLoader::loadMesh()
{
    char headerData[16];
    if (m_inputStream->readRawData(headerData, sizeof(headerData)) != sizeof(headerData))
    {
        setError("Error reading header");
        return NULL;
    }

    QString header = QString::fromLatin1(headerData, sizeof(headerData));
    if (header == "#celmodel__ascii")
    {
        setError("ASCII cmod files are not supported");
        return NULL;
    }
    else if (header != "#celmodel_binary")
    {
        setError("Wrong header; file is not a cmod mesh file");
        return NULL;
    }

    MeshGeometry* mesh = new MeshGeometry();

    // Add the default material
    Material* defaultMaterial = new Material();
    defaultMaterial->setDiffuse(Spectrum::Flat(0.8f));
    defaultMaterial->setBrdf(Material::Lambert);
    mesh->addMaterial(defaultMaterial);

    bool done = false;
    unsigned int meshCount = 0;

    while (!done && !error())
    {
        CmodToken token = readCmodToken();
        if (m_inputStream->atEnd())
        {
            done = true;
        }
        else if (token == CmodMaterial)
        {
            if (meshCount > 0)
            {
                setError("Materials must be appear before any meshes");
            }

            Material* material = loadMaterial();
            if (material)
            {
                mesh->addMaterial(material);
            }
        }
        else if (token == CmodMesh)
        {
            Submesh* submesh = loadSubmesh();
            if (submesh)
            {
                mesh->addSubmesh(submesh);
            }
        }
        else
        {
            setError("Unrecognized block in cmod (not a mesh or material)");
        }
    }

    if (error())
    {
        delete mesh;
        mesh = NULL;
    }

    return mesh;
}


bool
CmodLoader::readFloat1Property(float* value)
{
    CmodDataType type = readCmodDataType();
    if (type != CmodFloat1)
    {
        return false;
    }
    else
    {
        *m_inputStream >> *value;
        return true;
    }
}


bool
CmodLoader::readColorProperty(vesta::Spectrum* value)
{
    CmodDataType type = readCmodDataType();
    if (type != CmodColor)
    {
        return false;
    }
    else
    {
        float red = 0.0f, green = 0.0f, blue = 0.0f;
        *m_inputStream >> red >> green >> blue;
        *value = Spectrum(red, green, blue);
        return true;
    }
}


bool
CmodLoader::readStringProperty(QString* value)
{
    if (readCmodDataType() != CmodString)
    {
        return false;
    }
    else
    {
        quint16 stringLength = 0;
        *m_inputStream >> stringLength;
        char* data = new char[stringLength];
        if (m_inputStream->readRawData(data, stringLength) != stringLength)
        {
            delete data;
            return false;
        }
        else
        {
            *value = QString::fromUtf8(data, stringLength);
            delete data;
            return true;
        }
    }
}


CmodLoader::CmodToken
CmodLoader::readCmodToken()
{
    quint16 tokenValue = 0;
    *m_inputStream >> tokenValue;
    return CmodToken(tokenValue);
}


CmodLoader::CmodDataType
CmodLoader::readCmodDataType()
{
    quint16 typeValue = 0;
    *m_inputStream >> typeValue;
    return CmodDataType(typeValue);
}

