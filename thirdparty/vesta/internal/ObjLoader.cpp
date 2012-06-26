/*
 * $Revision: 223 $ $Date: 2010-03-30 05:44:44 -0700 (Tue, 30 Mar 2010) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "ObjLoader.h"
#include "../VertexPool.h"
#include "../Submesh.h"
#include "../TextureMapLoader.h"
#include "../Debug.h"
#include <sstream>
#include <cassert>

using namespace vesta;
using namespace Eigen;
using namespace std;


// Vertex pattern strings
static const char* VertexPattern = "%d";
static const char* VertexTexPattern = "%d/%d";
static const char* VertexNormalPattern = "%d//%d";
static const char* VertexTexNormalPattern = "%d/%d/%d";

static const string Whitespace(" \t\r\n");


/** Construct a new loader for Wavefront .OBJ files. To actually load
  * a model, call loadModel(). The same loader may be re-used to load
  * models from multiple sources.
  */
ObjLoader::ObjLoader() :
    m_mesh(NULL),
    m_currentMaterial(0),
    m_materialGroupStart(0),
    m_lineNumber(0),
    m_currentVertexType(InvalidVertex),
    m_firstGroupFace(false)
{
}


ObjLoader::~ObjLoader()
{
    // We'll only have a mesh around if loading failed.
    delete m_mesh;
}


// Strip a hash-preceded comment
static string
stripComment(const string& s)
{
    string::size_type pos = s.find_first_of('#');
    if (pos == string::npos)
    {
        return s;
    }
    else
    {
        return string(s, 0, pos);
    }
}


static bool
getInteger(const string& token, int& i)
{
    if (sscanf(token.c_str(), "%d", &i) == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}


static bool
getFloat(const string& token, float& f)
{
    if (sscanf(token.c_str(), "%f", &f) == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}


static bool
getVector2(const vector<string>& tokens, Vector2f& v)
{
    float x = 0.0f;
    float y = 0.0f;

    if (sscanf(tokens[1].c_str(), "%f", &x) == 1 &&
        sscanf(tokens[2].c_str(), "%f", &y) == 1)
    {
        v = Vector2f(x, y);
        return true;
    }
    else
    {
        return false;
    }
}


static bool
getVector3(const vector<string>& tokens, Vector3f& v)
{
    float x = 0.0;
    float y = 0.0;
    float z = 0.0;

    if (sscanf(tokens[1].c_str(), "%f", &x) == 1 &&
        sscanf(tokens[2].c_str(), "%f", &y) == 1 &&
        sscanf(tokens[3].c_str(), "%f", &z) == 1)
    {
        v = Vector3f(x, y, z);
        return true;
    }
    else
    {
        return false;
    }
}


static bool
getSpectrum(const vector<string>& tokens, Spectrum& s)
{
    Vector3f v;
    if (!getVector3(tokens, v))
    {
        return false;
    }

    s = Spectrum(v.x(), v.y(), v.z());
    return true;
}


// Determine the vertex type based on a vertex string. Wavefront OBJ files have
// four types of vertices, each represented by a sequence of slash separated
// integers.
//    - Position only (12)
//    - Position and texture coordinate (12/34)
//    - Position and normal (12//56)
//    - Position, texture coordinate, and normal (12/34/56)
//
// It is not valid for a single geometry element to mix different vertex
// types.
static ObjLoader::ObjVertexType
getVertexType(const std::string vertex)
{
    int positionIndex = 0;
    int texCoordIndex = 0;
    int normalIndex = 0;

    if (sscanf(vertex.c_str(), VertexTexNormalPattern, &positionIndex, &texCoordIndex, &normalIndex) == 3)
    {
        return ObjLoader::PositionTexNormalVertex;
    }
    else if (sscanf(vertex.c_str(), VertexNormalPattern, &positionIndex, &normalIndex) == 2)
    {
        return ObjLoader::PositionNormalVertex;
    }
    else if (sscanf(vertex.c_str(), VertexTexPattern, &positionIndex, &texCoordIndex) == 2)
    {
        return ObjLoader::PositionTexVertex;
    }
    else if (sscanf(vertex.c_str(), VertexPattern, &positionIndex) == 1)
    {
        return ObjLoader::PositionVertex;
    }
    else
    {
        return ObjLoader::InvalidVertex;
    }
}


// Get the pattern string for a vertex type
static const char*
getVertexPattern(ObjLoader::ObjVertexType type)
{
    switch (type)
    {
    case ObjLoader::PositionVertex:
        return VertexPattern;
    case ObjLoader::PositionTexVertex:
        return VertexTexPattern;
    case ObjLoader::PositionNormalVertex:
        return VertexNormalPattern;
    case ObjLoader::PositionTexNormalVertex:
        return VertexTexNormalPattern;
    default:
        return NULL;
    }
}


static bool
getFace(const std::vector<std::string> tokens,
        const char* vertexPattern,
        ObjLoader::ObjVertex faceVertices[])
{
    for (unsigned int i = 1; i < tokens.size(); ++i)
    {
        ObjLoader::ObjVertex v;
        
        if (vertexPattern == VertexPattern)
        {
            if (sscanf(tokens[i].c_str(), vertexPattern, &v.positionIndex) != 1)
            {
                return false;
            }
        }
        else if (vertexPattern == VertexTexPattern)
        {
            if (sscanf(tokens[i].c_str(), vertexPattern, &v.positionIndex, &v.texCoordIndex) != 2)
            {
                return false;
            }
        }
        else if (vertexPattern == VertexNormalPattern)
        {
            if (sscanf(tokens[i].c_str(), vertexPattern, &v.positionIndex, &v.normalIndex) != 2)
            {
                return false;
            }
        }
        else if (vertexPattern == VertexTexNormalPattern)
        {
            if (sscanf(tokens[i].c_str(), vertexPattern, &v.positionIndex, &v.texCoordIndex, &v.normalIndex) != 3)
            {
                return false;
            }
        }

        faceVertices[i - 1] = v;
    }

    return true;
}


// Split a string into substrings at whitespace boundaries
static void 
tokenize(const string& s, vector<string>& tokens)
{
    tokens.clear();

    string::size_type lastPos = s.find_first_not_of(Whitespace, 0);
    string::size_type pos = s.find_first_of(Whitespace, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        tokens.push_back(s.substr(lastPos, pos - lastPos));
        
        // Skip whitespapace.
        lastPos = s.find_first_not_of(Whitespace, pos);
        
        // Find the next non-whitespace character
        pos = s.find_first_of(Whitespace, lastPos);
    }
}


// Convert a one-based .obj vertex index into a zero-based index that can
// be used in VESTA. Return -1 if the index is invalid.
static int
convertIndex(int objIndex, int maxIndex)
{
    if (objIndex > 0)
    {
        return objIndex > maxIndex ? -1 : objIndex - 1;
    }
    else if (objIndex < 0)
    {
        return objIndex < -maxIndex ? -1 : objIndex + maxIndex;
    }
    else
    {
        return -1;
    }
}


// Convert all indices in a triangle to zero-based VESTA indices.
bool
ObjLoader::convertTriangleIndices(ObjTriangle& tri)
{
    for (unsigned int i = 0; i < 3; ++i)
    {
        tri.vertices[i].positionIndex = convertIndex(tri.vertices[i].positionIndex, m_positions.size());

        if (m_currentVertexType == PositionTexVertex || m_currentVertexType == PositionTexNormalVertex)
        {
            tri.vertices[i].texCoordIndex = convertIndex(tri.vertices[i].texCoordIndex, m_texCoords.size());
        }

        if (m_currentVertexType == PositionNormalVertex || m_currentVertexType == PositionTexNormalVertex)
        {
            tri.vertices[i].normalIndex = convertIndex(tri.vertices[i].normalIndex, m_normals.size());
        }

        if (tri.vertices[i].positionIndex < 0 ||
            tri.vertices[i].texCoordIndex < 0 ||
            tri.vertices[i].normalIndex < 0)
        {
            return false;
        }
    }

    return true;
}


void
ObjLoader::reportError(const string& message)
{
    ostringstream str(m_errorMessage);
    str << message << " (line: " << m_lineNumber << ")";
    VESTA_LOG("%s", m_errorMessage.c_str());
}


// Convert a complete vertex group to a VESTA Submesh and added
// it to the mesh.
void
ObjLoader::finishVertexGroup()
{
    finishMaterialGroup();

    if (m_materialGroups.size() == 0)
    {
        // Nothing to do
        return;
    }

    // If we have material groups, we must also have triangles
    assert(m_triangles.size() > 0);

    VertexPool vertexPool;

    // Create vertices; we will probably end up with duplicate vertices, but these
    // can be removed later on in mesh processing.
    for (unsigned int i = 0; i < m_triangles.size(); ++i)
    {
        const ObjTriangle& tri = m_triangles[i];

        for (unsigned int j = 0; j < 3; ++j)
        {
            const ObjVertex& vertex = tri.vertices[j];

            vertexPool.addVec3(m_positions[vertex.positionIndex]);

            if (m_currentVertexType == PositionNormalVertex || m_currentVertexType == PositionTexNormalVertex)
            {
                vertexPool.addVec3(m_normals[vertex.normalIndex]);
            }

            if (m_currentVertexType == PositionTexVertex || m_currentVertexType == PositionTexNormalVertex)
            {
                vertexPool.addVec2(m_texCoords[vertex.texCoordIndex]);
            }
        }
    }

    VertexSpec* vertexSpec = NULL;
    switch (m_currentVertexType)
    {
    case PositionVertex:
        vertexSpec = &VertexSpec::Position;
        break;
    case PositionNormalVertex:
        vertexSpec = &VertexSpec::PositionNormal;
        break;
    case PositionTexVertex:
        vertexSpec = &VertexSpec::PositionTex;
        break;
    case PositionTexNormalVertex:
        vertexSpec = &VertexSpec::PositionNormalTex;
        break;
    default:
        // Should never get here
        break;
    }

    assert(vertexSpec != NULL);

    VertexArray* vertexArray = vertexPool.createVertexArray(m_triangles.size() * 3, *vertexSpec);
    if (vertexArray)
    {
        Submesh* submesh = new Submesh(vertexArray);
        for (unsigned int i = 0; i < m_materialGroups.size(); ++i)
        {
            const ObjMaterialGroup& matGroup = m_materialGroups[i];
            PrimitiveBatch* prims = new PrimitiveBatch(PrimitiveBatch::Triangles, matGroup.triangleCount, matGroup.firstTriangle * 3);
            submesh->addPrimitiveBatch(prims, matGroup.materialIndex);
        }

        m_mesh->addSubmesh(submesh);
    }

    m_triangles.clear();
    m_materialGroups.clear();
    m_materialGroupStart = 0;
}


// Add a completed material group the list of material groups.
void
ObjLoader::finishMaterialGroup()
{
    int materialGroupSize = m_triangles.size() - m_materialGroupStart;
    if (materialGroupSize != 0)
    {
        ObjMaterialGroup materialGroup;
        materialGroup.firstTriangle = m_materialGroupStart;
        materialGroup.triangleCount = materialGroupSize;
        materialGroup.materialIndex = m_currentMaterial;
        m_materialGroups.push_back(materialGroup);

        m_materialGroupStart = m_triangles.size();
    }
}


// Handle a usemtl directive
unsigned int
ObjLoader::useMaterial(const string& materialName)
{
    unsigned int materialIndex;

    map<string, unsigned int>::const_iterator iter = m_materialTable.find(materialName);
    if (iter == m_materialTable.end())
    {
        // New material; add it to the material tables
        materialIndex = m_materials.size();
        m_materialTable[materialName] = materialIndex;
        m_materials.push_back(materialName);
    }
    else
    {
        // We've seen this material name already
        materialIndex = iter->second;
    }

    m_currentMaterial = materialIndex;

    return materialIndex;
}


/** Load a mesh in Wavefront OBJ format from an input stream. This method
  * returns a pointer to a newly created MeshGeometry object or null if the
  * mesh isn't valid. If loading fails, the error message is available via
  * the errorMessage() method.
  *
  * The meshes returned by loadModel() are not optimized and will usually
  * contain duplicate vertices. They should be processed by calling
  * mergeSubmeshes() and uniquifyVertices() on the mesh.
  *
  * The Wavefront OBJ format is described here:
  *   http://local.wasp.uwa.edu.au/~pbourke/dataformats/obj/
  *
  * loadModel() only handles triangle geometry right now. Point, line, curve,
  * and surface elements are currently ignored. Only a single material library
  * per model file is supported.
  */
MeshGeometry*
ObjLoader::loadModel(istream& in)
{
    // Delete any partially loaded mesh
    if (m_mesh)
    {
        delete m_mesh;
    }
    m_mesh = new MeshGeometry();

    m_lineNumber = 1;
    m_errorMessage = "";

    m_materialGroupStart = 0;
    m_firstGroupFace = true;

    m_positions.clear();
    m_normals.clear();
    m_texCoords.clear();

    m_materialGroups.clear();

    // Clear the material table and a default, anonymous material
    m_materialTable.clear();
    m_materials.clear();
    useMaterial("");
    m_materialLibrary = "";

    vector<string> tokens;

    for (;;)
    {
        string line;
        getline(in, line);
        if (!in.good())
        {
            break;
        }

        line = stripComment(line);
        tokenize(line, tokens);

        if (!tokens.empty())
        {
            string keyword = tokens.front();

            if (keyword == "v")
            {
                if (tokens.size() == 4)
                {
                    Vector3f position;
                    if (!getVector3(tokens, position))
                    {
                        reportError("Bad vertex position");
                        return NULL;
                    }
                    else
                    {
                        m_positions.push_back(position);
                    }
                }
                else
                {
                    reportError("Vertex position must have three components");
                    return NULL;
                }
            }
            else if (keyword == "vn")
            {
                if (tokens.size() == 4)
                {
                    Vector3f normal;
                    if (!getVector3(tokens, normal))
                    {
                        reportError("Bad vertex normal");
                        return NULL;
                    }
                    else
                    {
                        m_normals.push_back(normal);
                    }
                }
                else
                {
                    reportError("Vertex normal must have three components");
                    return NULL;
                }
            }
            else if (keyword == "vt")
            {
                if (tokens.size() == 3)
                {
                    Vector2f texCoord;
                    if (!getVector2(tokens, texCoord))
                    {
                        reportError("Bad texture coordinate");
                        return NULL;
                    }
                    else
                    {
                        m_texCoords.push_back(texCoord);
                    }
                }
                else
                {
                    reportError("Texture coordinate must have two components");
                    return NULL;
                }
            }
            else if (keyword == "f")
            {
                const int MaxFaceVertices = 3;
                if (tokens.size() < 4)
                {
                    reportError("Face has less than three vertices.");
                    return NULL;
                }
                else if (tokens.size() > MaxFaceVertices + 1)
                {
                    reportError("Face has too many vertices");
                    return NULL;
                }
                else
                {
                    ObjVertex faceVertices[MaxFaceVertices];
                    ObjVertexType vertexType = getVertexType(tokens[1]);
                    const char* vertexPattern = getVertexPattern(vertexType);

                    if (vertexPattern == NULL)
                    {
                        cerr << "vertex type: " << (int) vertexType << endl;
                        reportError("Bad vertex data for face");
                        return NULL;
                    }

                    if (vertexType != m_currentVertexType)
                    {
                        finishVertexGroup();
                        m_currentVertexType = vertexType;
                    }

                    if (!getFace(tokens, vertexPattern, faceVertices))
                    {
                        cerr << "vertex type:: " << (int) vertexType << endl;
                        reportError("Bad vertex data for face");
                        return NULL;
                    }
                    
                    ObjTriangle triangle;
                    triangle.vertices[0] = faceVertices[0];
                    triangle.vertices[1] = faceVertices[1];
                    triangle.vertices[2] = faceVertices[2];

                    if (!convertTriangleIndices(triangle))
                    {
                        reportError("Bad indexes in face");
                        return NULL;
                    }

                    m_triangles.push_back(triangle);
                }
            }
            else if (keyword == "g")
            {
                if (tokens.size() == 2)
                {
                    finishMaterialGroup();
                    m_firstGroupFace = true;
                }
                else
                {
                    reportError("Bad group");
                }
            }
            else if (keyword == "o")
            {
                // object keyword ignored
            }
            else if (keyword == "s")
            {
                // smooth group keyword ignored
            }
            else if (keyword == "usemtl")
            {
                if (tokens.size() == 2)
                {
                    finishMaterialGroup();
                    useMaterial(tokens[1]);
                }
                else
                {
                    reportError("Bad material");
                }
            }
            else if (keyword == "mtllib")
            {
                if (tokens.size() == 2)
                {
                    m_materialLibrary = tokens[1];
                }
                else
                {
                    reportError("Bad material library");
                }
            }
            else 
            {
                cout << "Unknown keyword " << keyword << endl;
            }
        }
        
        ++m_lineNumber;
    }

    finishVertexGroup();

    // Create default materials
    for (unsigned int i = 0; i < m_materials.size(); ++i)
    {
        Material* material = new Material();
        material->setDiffuse(Spectrum::Flat(1.0f));
        m_mesh->addMaterial(material);
    }

    MeshGeometry* m_loadedMesh = m_mesh;
    m_mesh = NULL;

    return m_loadedMesh;
}



ObjMaterial::ObjMaterial() :
    illuminationModel(BlinnPhongModel),
    dissolve(1.0f),
    specularPower(1.0f),
    indexOfRefraction(1.0f)
{
}


ObjMaterial::ObjMaterial(const ObjMaterial& other) :
    illuminationModel(other.illuminationModel),
    dissolve(other.dissolve),
    diffuseColor(other.diffuseColor),
    specularColor(other.specularColor),
    specularPower(other.specularPower),
    indexOfRefraction(other.indexOfRefraction),
    diffuseMap(diffuseMap),
    specularMap(specularMap)
{
}


ObjMaterial&
ObjMaterial::operator=(const ObjMaterial& other)
{
    illuminationModel = other.illuminationModel;
    dissolve = other.dissolve;
    diffuseColor = other.diffuseColor;
    specularColor = other.specularColor;
    specularPower = other.specularPower;
    indexOfRefraction = other.indexOfRefraction;
    diffuseMap = diffuseMap;
    specularMap = specularMap;

    return *this;
}


/** Convert a Wavefront material to a VESTA material.
  */
Material*
ObjMaterial::convert(TextureMapLoader* textureLoader) const
{
    Material* material = new Material();

    material->setOpacity(dissolve);

    if (illuminationModel == ConstantColorModel)
    {
        material->setEmission(diffuseColor);
        return material;
    }
    else
    {
        TextureProperties textureProps(TextureProperties::Wrap);

        material->setDiffuse(diffuseColor);
        if (textureLoader && !diffuseMap.empty())
        {
            material->setBaseTexture(textureLoader->loadTexture(diffuseMap, textureProps));
        }

        if (int(illuminationModel) > int(DiffuseModel))
        {
            material->setSpecular(specularColor);
            material->setPhongExponent(specularPower);
            if (textureLoader && !specularMap.empty())
            {
                material->setSpecularTexture(textureLoader->loadTexture(specularMap, textureProps));
            }
        }

        if (illuminationModel == FresnelReflectiveModel)
        {
            // Estimate the reflectance at normal incidence assuming a dielectric material
            // and that light is traveling through a vacuum (index of refraction = 1) before
            // hitting the surface.
            float Nsurf = indexOfRefraction;
            float Nmed = 1.0f;
            float fresnel = pow((Nmed - Nsurf) / (Nmed + Nsurf), 2.0f);
            material->setFresnelReflectance(fresnel);
        }

        // Set the BRDF from the illumination model
        switch (illuminationModel)
        {
        case ConstantColorModel:
        case DiffuseModel:
            material->setBrdf(Material::Lambert);
            break;

        case ReflectiveModel:
        case FresnelReflectiveModel:
        case ReflectiveModelNoRT:
            material->setBrdf(Material::BlinnPhongReflective);
            break;

        default:
            if (specularColor == Spectrum::Black())
            {
                material->setBrdf(Material::Lambert);
            }
            else
            {
                material->setBrdf(Material::BlinnPhong);
            }
            break;
        }

    }

    return material;
}


/** Construct an empty material library.
  */
ObjMaterialLibrary::ObjMaterialLibrary()
{
}


/** Destructor.
  */
ObjMaterialLibrary::~ObjMaterialLibrary()
{
}


/** Lookup a material in the library. Returns null if the named material is
  * not in the library.
  */
Material*
ObjMaterialLibrary::material(const string& materialName) const
{
    map<string, counted_ptr<Material> >::const_iterator iter = m_materials.find(materialName);
    if (iter == m_materials.end())
    {
        return NULL;
    }
    else
    {
        return iter->second.ptr();
    }
}


/** Add a new material to the library. If there's already a material in the
  * library with materialName, it will be replaced.
  */
void
ObjMaterialLibrary::addMaterial(const string& materialName, Material* material)
{
    m_materials[materialName] = material;
}



/** Create a new material library loader. To actually load a material library,
  * call loadMaterials().
  */
ObjMaterialLibraryLoader::ObjMaterialLibraryLoader(TextureMapLoader* textureLoader) :
    m_materials(NULL),
    m_textureLoader(textureLoader),
    m_currentMaterial(NULL),
    m_lineNumber(1)
{
}


ObjMaterialLibraryLoader::~ObjMaterialLibraryLoader()
{
    delete m_currentMaterial;

    // Delete any partially loaded library
    if (m_materials)
    {
        delete m_materials;
    }
}



void
ObjMaterialLibraryLoader::reportError(const string& message)
{
    ostringstream str(m_errorMessage);
    str << message << " (line: " << m_lineNumber << ")";
    VESTA_LOG("%s", m_errorMessage.c_str());
}


// Add a completed material to the current material library
void
ObjMaterialLibraryLoader::finishMaterial()
{
    if (!m_currentMaterial)
    {
        return;
    }

    Material* material = m_currentMaterial->convert(m_textureLoader);
    m_materials->addMaterial(m_currentMaterialName, material);

    delete m_currentMaterial;
    m_currentMaterial = NULL;
}


/** Load a libary of materials from an input stream. Returns a new material
  * library if the materials were read successfully or null if there was an
  * error.
  *
  * Material definitions for the Wavefront obj format don't map perfectly to
  * VESTA's materials. The format is fully documented here:
  *    http://local.wasp.uwa.edu.au/~pbourke/dataformats/mtl/
  *
  * The following keywords are recognized:
  *
  * newmtl - start a new material definition
  * illum - illumination model
  * d - dissolve factory (opacity)
  * Kd - diffuse color
  * Ks - specular color
  * Ns - specular exponent
  * map_Kd - diffuse texture
  * map_Ks - specular texture
  * Tr - opacity (apparently not official, but output by Filmbox converter)
  * Ni - index of refraction; while refraction is not supported, the index of refraction
  *      is used to estimate the Fresnel reflectance for materials with illumination
  *      models that include a Fresnel term.
  *
  * Ignored attributes:
  * Ka - ambient color; VESTA assumes physical materials with no separate ambient color
  * map_Ka - ambient map
  * map_Ns - specular exponent map
  * map_d - alpha channel of the diffuse map is used instead of a separate texture
  * map_aat - enable/disable anti-aliasing of textures. Mipmaps are always enabled in VESTA.
  * decal - decal textures not supported
  * disp - displacement map
  * bump - bump map
  * refl - local reflection map
  *
  * Not all Wavefront illuminations models are completely supported. VESTA doesn't
  * handle refraction or perform ray tracing. Other features of models are supported.
  */
ObjMaterialLibrary*
ObjMaterialLibraryLoader::loadMaterials(istream& in)
{
    // Delete any partially loaded mesh
    if (m_materials)
    {
        delete m_materials;
    }

    // Create a new material library
    m_materials = new ObjMaterialLibrary();

    if (m_currentMaterial)
    {
        delete m_currentMaterial;
        m_currentMaterial = NULL;
    }

    m_lineNumber = 1;
    m_errorMessage = "";

    vector<string> tokens;

    for (;;)
    {
        string line;
        getline(in, line);
        if (!in.good())
        {
            break;
        }

        line = stripComment(line);
        tokenize(line, tokens);

        if (!tokens.empty())
        {
            string keyword = tokens.front();

            if (keyword == "newmtl")
            {
                finishMaterial();
                if (tokens.size() != 2)
                {
                    reportError("Bad material definition");
                    return NULL;
                }
                else
                {
                    m_currentMaterialName = tokens[1];
                    m_currentMaterial = new ObjMaterial();
                }
            }
            else if (m_currentMaterial)
            {
                if (keyword == "Kd")
                {
                    // Diffuse color
                    if (tokens.size() != 4 || !getSpectrum(tokens, m_currentMaterial->diffuseColor))
                    {
                        reportError("Bad diffuse color");
                    }
                }
                else if (keyword == "Ks")
                {
                    // Specular color
                    if (tokens.size() != 4 || !getSpectrum(tokens, m_currentMaterial->specularColor))
                    {
                        reportError("Bad specular color");
                    }
                }
                else if (keyword == "Ns")
                {
                    // Specular power for Blinn/Phong model
                    if (tokens.size() != 2 || !getFloat(tokens[1], m_currentMaterial->specularPower))
                    {
                        reportError("Bad specular exponent");
                    }
                }
                else if (keyword == "Ni")
                {
                    if (tokens.size() != 2 || !getFloat(tokens[1], m_currentMaterial->indexOfRefraction))
                    {
                        reportError("Bad index of refraction");
                    }
                }
                else if (keyword == "Tr")
                {
                    // Opacity; Tr isn't documented in the mtl spec from Alias|Wavefront, but it
                    // is used by the FBX converter.
                    if (tokens.size() != 2 || !getFloat(tokens[1], m_currentMaterial->dissolve))
                    {
                        reportError("Bad transparency value");
                    }
                }
                else if (keyword == "d")
                {
                    // Dissolve (equivalent to opacity in VESTA)
                    if (tokens.size() != 2 || !getFloat(tokens[1], m_currentMaterial->dissolve))
                    {
                        reportError("Bad dissolve value");
                    }
                }
                else if (keyword == "map_Kd")
                {
                    // Diffuse map
                    if (tokens.size() != 2)
                    {
                        reportError("Bad diffuse map");
                    }
                    else
                    {
                        m_currentMaterial->diffuseMap = tokens[1];
                    }
                }
                else if (keyword == "map_Ks")
                {
                    // Specular map
                    if (tokens.size() != 2)
                    {
                        reportError("Bad specular map");
                    }
                    else
                    {
                        m_currentMaterial->specularMap = tokens[1];
                    }
                }
                else if (keyword == "illum")
                {
                    int illuminationModel = 0;
                    if (tokens.size() != 2 || !getInteger(tokens[1], illuminationModel))
                    {
                        reportError("Bad illumination model");
                    }
                    else if (illuminationModel < 0 || illuminationModel >= int(ObjMaterial::MaxIlluminationModel))
                    {
                        reportError("Unsupported illumination model");
                    }
                    else
                    {
                        m_currentMaterial->illuminationModel = ObjMaterial::IlluminationModel(illuminationModel);
                    }
                }
            }
            else
            {
                // No material definition started
            }
        }

        m_lineNumber++;
    }

    // Complete the last material
    finishMaterial();

    ObjMaterialLibrary* m_loadedMaterials = m_materials;
    m_materials = NULL;

    return m_loadedMaterials;
}
