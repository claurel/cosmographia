// This file is part of Cosmographia.
//
// Copyright (C) 2011 Chris Laurel <claurel@gmail.com>
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

#ifndef _COMPATIBILITY_CMOD_LOADER_H_
#define _COMPATIBILITY_CMOD_LOADER_H_

#include <vesta/MeshGeometry.h>
#include <vesta/TextureMapLoader.h>
#include <QIODevice>

class QDataStream;


class CmodLoader
{
public:
    CmodLoader(QIODevice* in, vesta::TextureMapLoader* textureLoader);
    ~CmodLoader();

    bool error() const
    {
        return m_hasError;
    }

    QString errorMessage() const
    {
        return m_errorMessage;
    }

    vesta::MeshGeometry* loadMesh();

private:
    enum CmodToken
    {
        CmodMaterial       = 1001,
        CmodEndMaterial    = 1002,
        CmodDiffuse        = 1003,
        CmodSpecular       = 1004,
        CmodSpecularPower  = 1005,
        CmodOpacity        = 1006,
        CmodTexture        = 1007,
        CmodMesh           = 1009,
        CmodEndMesh        = 1010,
        CmodVertexDesc     = 1011,
        CmodEndVertexDesc  = 1012,
        CmodVertices       = 1013,
        CmodEmissive       = 1014,
        CmodBlend          = 1015,
        CmodIlluminationModel = 1016,
        CmodInvalidToken   = 0,
    };

    enum CmodDataType
    {
        CmodFloat1         = 1,
        CmodFloat2         = 2,
        CmodFloat3         = 3,
        CmodFloat4         = 4,
        CmodString         = 5,
        CmodUint32         = 6,
        CmodColor          = 7,
        CmodInvalidType    = 0,
    };

    enum CmodIlluminationModelType
    {
        CmodBlinnPhong     = 0,
        CmodLunarLambert   = 1,
        CmodReflective     = 2,
        CmodInvalidIlluminationModel = -1,
    };

    void setError(const QString& errorMessage);
    CmodToken readCmodToken();
    CmodDataType readCmodDataType();

    bool readFloat1Property(float* value);
    bool readColorProperty(vesta::Spectrum* value);
    bool readStringProperty(QString* value);

    vesta::Material* loadMaterial();
    vesta::Submesh* loadSubmesh();
    vesta::VertexSpec* loadVertexSpec();
    vesta::PrimitiveBatch* loadPrimitiveBatch(unsigned int primitiveTypeToken, unsigned int vertexCount, unsigned int* materialIndex);
    vesta::VertexArray* loadVertexArray(const vesta::VertexSpec& spec);

private:
    QDataStream* m_inputStream;
    QString m_errorMessage;
    bool m_hasError;
    vesta::counted_ptr<vesta::TextureMapLoader> m_textureLoader;
};

#endif // _COMPATIBILITY_CMOD_LOADER_H_
