/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#ifndef _VESTA_INTERNAL_OBJ_LOADER_H_
#define _VESTA_INTERNAL_OBJ_LOADER_H_

#include "../MeshGeometry.h"
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <cstdio>
#include <Eigen/Core>


namespace vesta
{

/** Loader for Wavefront OBJ format meshes.
  */
class ObjLoader
{
public:
    ObjLoader();
    ~ObjLoader();

    MeshGeometry* loadModel(std::istream& in);

    /** Get the name of the most recently used material library.
      */
    std::string materialLibrary() const
    {
        return m_materialLibrary;
    }

    /** Get the list of all materials used in the mesh.
      */
    const std::vector<std::string>& materials() const
    {
        return m_materials;
    }

    /** Get the error message from an unsuccessful call to loadModel(). If no error
      * occurred, the message string will be empty.
      */
    std::string errorMessage() const
    {
        return m_errorMessage;
    }

public:
    struct ObjVertex
    {
        ObjVertex() :
            positionIndex(0),
            normalIndex(0),
            texCoordIndex(0)
        {
        }

        int positionIndex;
        int normalIndex;
        int texCoordIndex;
    };

    struct ObjTriangle
    {
        ObjVertex vertices[3];
    };

    struct ObjMaterialGroup
    {
        unsigned int materialIndex;
        unsigned int firstTriangle;
        unsigned int triangleCount;
    };

    enum ObjVertexType
    {
        InvalidVertex,
        PositionVertex,
        PositionTexVertex,
        PositionNormalVertex,
        PositionTexNormalVertex,
    };

private:
    void reportError(const std::string& message);
    void finishVertexGroup();
    void finishMaterialGroup();
    int convertVertexIndex(int objIndex, int maxIndex);
    bool convertTriangleIndices(ObjTriangle& tri);
    unsigned int useMaterial(const std::string& materialName);

private:
    MeshGeometry* m_mesh;

    std::vector<Eigen::Vector3f> m_positions;
    std::vector<Eigen::Vector3f> m_normals;
    std::vector<Eigen::Vector2f> m_texCoords;
    std::vector<ObjTriangle> m_triangles;
    std::vector<ObjMaterialGroup> m_materialGroups;

    std::map<std::string, unsigned int> m_materialTable;
    std::vector<std::string> m_materials;

    std::string m_materialLibrary;

    unsigned int m_currentMaterial;
    unsigned int m_materialGroupStart;
    unsigned int m_lineNumber;
    std::string m_errorMessage;

    ObjVertexType m_currentVertexType;
    bool m_firstGroupFace;
};


class ObjMaterial
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    ObjMaterial();
    ObjMaterial(const ObjMaterial& other);
    ObjMaterial& operator=(const ObjMaterial& other);

    Material* convert(TextureMapLoader* textureLoader) const;

    enum IlluminationModel
    {
        ConstantColorModel     = 0,
        DiffuseModel           = 1,
        BlinnPhongModel        = 2,
        ReflectiveModel        = 3,
        GlassModel             = 4,
        FresnelReflectiveModel = 5,
        RefractiveModel        = 6,
        FresnelRefractiveModel = 7,
        ReflectiveModelNoRT    = 8,
        GlassModelNoRT         = 9,
        MaxIlluminationModel   = 10,
    };

    IlluminationModel illuminationModel;
    float dissolve;
    Spectrum diffuseColor;
    Spectrum specularColor;
    float specularPower;
    float indexOfRefraction;
    std::string diffuseMap;
    std::string specularMap;
};


/** ObjMaterialLibrary is a container for named materials loaded from
  * a Wavefront mtl file.
  */
class ObjMaterialLibrary
{
public:
    ObjMaterialLibrary();
    ~ObjMaterialLibrary();

    Material* material(const std::string& materialName) const;
    void addMaterial(const std::string& materialName, Material* material);

private:
    std::map<std::string, counted_ptr<Material> > m_materials;
};

class ObjMaterialLibraryLoader
{
public:
    ObjMaterialLibraryLoader(TextureMapLoader* textureLoader);
    ~ObjMaterialLibraryLoader();

    ObjMaterialLibrary* loadMaterials(std::istream& in);

    /** Get the error message from an unsuccessful call to loadMaterials(). If no error
      * occurred, the message string will be empty.
      */
    std::string errorMessage() const
    {
        return m_errorMessage;
    }

private:
    void reportError(const std::string& message);
    void finishMaterial();

private:
    ObjMaterialLibrary* m_materials;
    TextureMapLoader* m_textureLoader;
    ObjMaterial* m_currentMaterial;
    std::string m_currentMaterialName;
    unsigned int m_lineNumber;
    std::string m_errorMessage;
};

}

#endif // _VESTA_INTERNAL_OBJ_LOADER_H_
