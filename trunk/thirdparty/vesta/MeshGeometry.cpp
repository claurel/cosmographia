/*
 * $Revision: 610 $ $Date: 2011-04-29 14:45:37 -0700 (Fri, 29 Apr 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "MeshGeometry.h"
#include "RenderContext.h"
#include "Material.h"
#include "TextureMapLoader.h"
#include "glhelp/GLVertexBuffer.h"
#include "Debug.h"
#include <algorithm>
#include <cassert>
#include <limits>
#include <fstream>
#include <cctype>

// 3D file format support
#include "VertexPool.h"
#include "lib3ds/lib3ds.h"
#include "internal/ObjLoader.h"

using namespace vesta;
using namespace Eigen;
using namespace std;


MeshGeometry::MeshGeometry() :
    m_boundingSphereRadius(0.0f),
    m_meshScale(1.0f, 1.0f, 1.0f),
    m_hwBuffersCurrent(false)
{
    // By default, mesh geometry both casts and receives shadows
    setShadowReceiver(true);
    setShadowCaster(true);
}


MeshGeometry::~MeshGeometry()
{
    freeSubmeshBuffers();
}


void
MeshGeometry::render(RenderContext& rc,
                     double /* clock */) const
{
    if (!m_hwBuffersCurrent)
    {
        realize();
    }

    // Track the last used material in order to avoid redundant
    // material bindings.
    unsigned int lastMaterialIndex = Submesh::DefaultMaterialIndex;

    rc.pushModelView();
    rc.scaleModelView(m_meshScale);

    // Render all submeshes
    GLVertexBuffer* boundVertexBuffer = NULL;

    for (unsigned int i = 0; i < m_submeshes.size(); ++i)
    {
        const Submesh& submesh = *m_submeshes[i];
        if (i < m_submeshBuffers.size() && m_submeshBuffers[i])
        {
            boundVertexBuffer = m_submeshBuffers[i];
            rc.bindVertexBuffer(submesh.vertices()->vertexSpec(), m_submeshBuffers[i], submesh.vertices()->stride());
        }
        else
        {
            if (boundVertexBuffer)
            {
                boundVertexBuffer->unbind();
                boundVertexBuffer = false;
            }
            rc.bindVertexArray(submesh.vertices());
        }

        const vector<PrimitiveBatch*>& batches = submesh.primitiveBatches();
        const vector<unsigned int>& materials = submesh.materials();
        assert(batches.size() == materials.size());

        // Render all batches in the submesh
        for (unsigned int j = 0; j < batches.size(); j++)
        {
            // If we have a new material, bind it
            unsigned int materialIndex = materials[j];
            if (materialIndex != lastMaterialIndex)
            {
                if (materialIndex < m_materials.size())
                {
                    rc.bindMaterial(m_materials[materialIndex].ptr());
                }
                lastMaterialIndex = materialIndex;
            }

            rc.drawPrimitives(*batches[j]);
        }
    }

    if (boundVertexBuffer)
    {
        boundVertexBuffer->unbind();
    }

    rc.popModelView();
}


void
MeshGeometry::renderShadow(RenderContext& rc,
                           double /* clock */) const
{
    if (!m_hwBuffersCurrent)
    {
        realize();
    }

    // Use an extremely basic material to avoid wasting time
    // with pixel shader calculations when we're just interested
    // in depth values.
    Material simpleMaterial;
    rc.bindMaterial(&simpleMaterial);

    rc.pushModelView();
    rc.scaleModelView(m_meshScale);

    // Render all submeshes
    GLVertexBuffer* boundVertexBuffer = NULL;
    for (unsigned int i = 0; i < m_submeshes.size(); ++i)
    {
        const Submesh& submesh = *m_submeshes[i];

        if (i < m_submeshBuffers.size() && m_submeshBuffers[i])
        {
            boundVertexBuffer = m_submeshBuffers[i];
            rc.bindVertexBuffer(submesh.vertices()->vertexSpec(), m_submeshBuffers[i], submesh.vertices()->stride());
        }
        else
        {
            if (boundVertexBuffer)
            {
                boundVertexBuffer->unbind();
                boundVertexBuffer = false;
            }
            rc.bindVertexArray(submesh.vertices());
        }

        const vector<PrimitiveBatch*>& batches = submesh.primitiveBatches();
        const vector<unsigned int>& materials = submesh.materials();
        assert(batches.size() == materials.size());

        // Render all batches in the submesh
        for (unsigned int j = 0; j < batches.size(); j++)
        {
            // Skip mostly transparent items when drawing into the shadow
            // buffer.
            // TODO: Textures with transparent parts aren't handled here
            unsigned int materialIndex = materials[j];
            if (materialIndex >= m_materials.size() || m_materials[materialIndex]->opacity() > 0.5f)
            {
                rc.drawPrimitives(*batches[j]);
            }
        }
    }

    if (boundVertexBuffer)
    {
        boundVertexBuffer->unbind();
    }

    rc.popModelView();
}


float
MeshGeometry::boundingSphereRadius() const
{
    return m_meshScale.maxCoeff() * m_boundingSphereRadius;
}


bool
MeshGeometry::handleRayPick(const Eigen::Vector3d& pickOrigin,
                            const Eigen::Vector3d& pickDirection,
                            double /* clock */,
                            double* distance) const
{
    Vector3d meshScale = m_meshScale.cast<double>();
    Matrix3d invScale = meshScale.cwise().inverse().asDiagonal();
    Vector3d origin = invScale * pickOrigin;
    Vector3d direction = (invScale * pickDirection).normalized();

    double closestHit = numeric_limits<double>::infinity();

    for (vector<counted_ptr<Submesh> >::const_iterator iter = m_submeshes.begin();
         iter != m_submeshes.end(); ++iter)
    {
        double submeshDistance = 0.0;

        // TODO: Check the bounding volume of the submesh before doing the actual mesh
        // intersection test.
        if ((*iter)->rayPick(origin, direction, &submeshDistance))
        {
            if (submeshDistance < closestHit)
            {
                closestHit = submeshDistance;
            }
        }
    }

    if (closestHit < numeric_limits<double>::infinity())
    {
        *distance = (meshScale.cwise() * direction).norm() * closestHit;
        return true;
    }
    else
    {
        return false;
    }
}


void
MeshGeometry::addSubmesh(Submesh* submesh)
{
    if (submesh)
    {
        // Compute bounding volumes for the mesh
        m_boundingSphereRadius = std::max(m_boundingSphereRadius, submesh->boundingSphereRadius());
        if (m_submeshes.empty())
            m_boundingBox = submesh->boundingBox();
        else
            m_boundingBox = m_boundingBox.merged(submesh->boundingBox());

        // Add the mesh
        m_submeshes.push_back(counted_ptr<Submesh>(submesh));
    }
}


void
MeshGeometry::addMaterial(Material* material)
{
    if (material)
    {
        m_materials.push_back(counted_ptr<Material>(material));
    }
}


/** Optimize the mesh by merging submeshes that share the same vertex spec.
  * This reduces the number of separate vertex buffers required.
  */
bool
MeshGeometry::mergeSubmeshes()
{
    if (m_submeshes.size() <= 1)
    {
        return true;
    }

    // At the beginning, all submeshes are unmerged, none are merged
    vector<counted_ptr<Submesh> > merged;
    vector<Submesh*> unmerged;
    for (vector<counted_ptr<Submesh> >::const_iterator iter = m_submeshes.begin(); iter != m_submeshes.end(); ++iter)
    {
        unmerged.push_back(iter->ptr());
    }

    // Iterate over submeshes repeatedly, removing groups that are merged.
    while (!unmerged.empty())
    {
        const VertexArray* vertexArray = unmerged.front()->vertices();

        vector<Submesh*> matches;
        vector<Submesh*> nonmatches;
        for (vector<Submesh*>::const_iterator iter = unmerged.begin(); iter != unmerged.end(); ++iter)
        {
            Submesh* s = *iter;
            if (vertexArray->stride() == s->vertices()->stride() &&
                vertexArray->vertexSpec() == s->vertices()->vertexSpec())
            {
                matches.push_back(s);
            }
            else
            {
                nonmatches.push_back(s);
            }
        }

        if (matches.size() == 1)
        {
            // Avoid the expense of merging when there's just a single
            // mesh.
            merged.push_back(counted_ptr<Submesh>(matches.front()));
        }
        else
        {
            Submesh* mergedSubmesh = Submesh::mergeSubmeshes(matches);
            if (!mergedSubmesh)
            {
                return false;
            }

            merged.push_back(counted_ptr<Submesh>(mergedSubmesh));
        }

        // Set submeshes to the list of unmerged submeshes
        unmerged = nonmatches;
    }

    VESTA_LOG("Merged %d submeshes into %d", m_submeshes.size(), merged.size());

    m_submeshes = merged;
    setMeshChanged();

    return true;
}


/** Mark the mesh as changed so that hardware buffers will be regenerated
  * the next time it is rendered. This method should be called after submeshes are
  * added, removed, or changed in order ensure that the correct mesh geometry
  * is displayed.
  */
void
MeshGeometry::setMeshChanged()
{
    m_hwBuffersCurrent = false;
}


/** Optimize the mesh by removing duplicate vertices. This operation is expensive for large meshes,
  * but can greatly reduce the number of vertices in unoptimized meshes. Naive normal generation can
  * produce large numbers of duplicate vertices that should be removed with a uniquify operation.
  * The positionTolerance, normalTolerance, and texCoordTolerance determine how closely vertices
  * need to match in order to be considered identical. Components of two positions (or normals, etc.)
  * are tested to see if their difference is less the tolerance value.
  *
  * \return true if unquification was successful, false if an error occurred (should only happen
  * in a low memory situation.)
  */
bool
MeshGeometry::uniquifyVertices(float positionTolerance, float normalTolerance, float texCoordTolerance)
{
    for (vector<counted_ptr<Submesh> >::const_iterator iter = m_submeshes.begin(); iter != m_submeshes.end(); ++iter)
    {
        bool ok = iter->ptr()->uniquifyVertices(positionTolerance, normalTolerance, texCoordTolerance);
        if (!ok)
        {
            VESTA_WARNING("Error occurred in while uniquifying mesh vertices.");
            return false;
        }
    }

    setMeshChanged();

    return true;
}


/** Compress indices to 16-bit where possible. This can improve rendering performance
  * on some hardware, and some mobile GPUs can only use 16-bit vertex indices.
  */
void
MeshGeometry::compressIndices()
{
    for (vector<counted_ptr<Submesh> >::const_iterator iter = m_submeshes.begin(); iter != m_submeshes.end(); ++iter)
    {
        iter->ptr()->compressIndices();
    }

    setMeshChanged();
}


#define TEST_PROPERTY(p0, p1) if ((p0) < (p1)) return true; else if ((p0) > (p1)) return false;

static bool CompareSpectra(const Spectrum& s0, const Spectrum& s1)
{
    TEST_PROPERTY(s0.red(), s1.red());
    TEST_PROPERTY(s0.green(), s1.green());
    TEST_PROPERTY(s0.blue(), s1.blue());
    return false;
}

#define TEST_COLOR_PROPERTY(c0, c1) if (CompareSpectra((c0), (c1))) return true; else if (CompareSpectra((c1), (c0))) return false;

// This class enforces a strict ordering on materials. This is used to sort and
// eliminate redundant materials, which often appear in mesh files. Once extra
// materials are removed, small batches can be merged in to larger ones, thereby
// reducing the number of draw calls and improving rendering performance.
class MaterialOrderingPredicate
{
public:
    MaterialOrderingPredicate() {}

    bool operator()(const Material& m0, const Material& m1) const
    {
        TEST_PROPERTY((int) m0.brdf(), (int) m1.brdf());
        TEST_PROPERTY(m0.opacity(), m1.opacity());
        TEST_COLOR_PROPERTY(m0.diffuse(), m1.diffuse());
        TEST_COLOR_PROPERTY(m0.specular(), m1.specular());
        TEST_PROPERTY(m0.phongExponent(), m1.phongExponent());
        TEST_PROPERTY(m0.fresnelReflectance(), m1.fresnelReflectance());
        TEST_COLOR_PROPERTY(m0.emission(), m1.emission());
        TEST_PROPERTY((int) m0.blendMode(), (int) m1.blendMode());
        TEST_PROPERTY(m0.baseTexture(), m1.baseTexture());
        TEST_PROPERTY(m0.normalTexture(), m1.normalTexture());
        TEST_PROPERTY(m0.specularTexture(), m1.specularTexture());
        TEST_PROPERTY((int) m0.specularModifier(), (int) m1.specularModifier());
        return false;
    }
};

#undef TEST_PROPERTY
#undef TEST_COLOR_PROPERTY


// Predicate to test materials for equality; the current implemtation just
// uses the strict ordering predicate (i.e. if !(a < b) and !(b < a), then
// a must equal b)
class MaterialEqualityPredicate
{
public:
    MaterialEqualityPredicate() {}

    bool operator()(const Material& m0, const Material& m1) const
    {
        MaterialOrderingPredicate pred;
        return !pred(m0, m1) && !pred(m1, m0);
    }
};


struct MaterialRef
{
    unsigned int index;
    Material* material;

    bool operator<(const MaterialRef& other) const
    {
        MaterialOrderingPredicate pred;
        return pred(*(this->material), *(other.material));
    }
};


/** Optimize the mesh by eliminating redundant materials and combining multiple
  * small primitive batches into larger ones. This can improve performance in two
  * ways:
  *   - the number of draw calls that must be issued to the graphics processor is reduced
  *   - the number of graphics state transitions is reduced
  *
  * For the best mesh optimization, mergeMaterials should be called after mergeSubmeshes and
  * uniquifyVertices.
  *
  * \return true if the optimization was successful (it should only ever fail in
  *         out of memory conditions.)
  */
bool
MeshGeometry::mergeMaterials()
{
    // Build the map that associates materials in the old material array with unique indices.
    vector<MaterialRef> materialRefs(m_materials.size());
    vector<unsigned int> materialMap(m_materials.size());
    for (unsigned int i = 0; i < materialMap.size(); ++i)
    {
        materialRefs[i].index = i;
        materialRefs[i].material = material(i);
    }

    sort(materialRefs.begin(), materialRefs.end());

    vector<counted_ptr<Material> > mergedMaterials;

    // Create a list of unique materials along with a map from old (non-unique) materials
    // to merged materials.
    MaterialEqualityPredicate eqPred;
    unsigned int uniqueCount = 0;
    for (unsigned int i = 0; i < materialRefs.size(); ++i)
    {
        if (i == 0 || !eqPred(*materialRefs[i].material, *materialRefs[i - 1].material))
        {
            uniqueCount++;
            mergedMaterials.push_back(counted_ptr<Material>(materialRefs[i].material));
        }
        materialMap[materialRefs[i].index] = uniqueCount - 1;
    }

    bool reducedMaterialCount = mergedMaterials.size() < m_materials.size();
    if (reducedMaterialCount)
    {
        VESTA_LOG("Merge materials reduced from %d to %d", m_materials.size(), uniqueCount);
        m_materials = mergedMaterials;
    }

    for (vector<counted_ptr<Submesh> >::iterator iter = m_submeshes.begin(); iter != m_submeshes.end(); ++iter)
    {
        if (reducedMaterialCount)
        {
            const vector<unsigned int>& submeshMaterials = (*iter)->materials();
            for (unsigned int i = 0; i < submeshMaterials.size(); ++i)
            {
                (*iter)->setMaterial(i, materialMap[submeshMaterials[i]]);
            }
        }

        bool ok = iter->ptr()->mergeMaterials();
        if (!ok)
        {
            VESTA_WARNING("Error occurred in while merging mesh materials.");
            return false;
        }
    }

    return true;
}


// Create buffers required for drawing the mesh on hardware. This must be called
// before rendering whenever the mesh has changed.
//
// return true if all hardware buffers could be created.
bool
MeshGeometry::realize() const
{
    // Mark hardware buffers as up-to-date. Even if vertex buffer allocation fails,
    // we don't want to keep retrying every time a frame is rendered.
    m_hwBuffersCurrent = true;

    // Only create vertex buffers for submeshes for which the size of the vertex
    // data exceeds the limit below.
    const unsigned int VertexBufferSizeThreshold = 4096;

    // Don't create anything if hardware/driver doesn't support vertex buffer objects,
    // but report success anyhow.
    if (!GLVertexBuffer::supported())
    {
        return true;
    }

    bool ok = true;
    freeSubmeshBuffers();

    for (vector< counted_ptr<Submesh> >::const_iterator iter = m_submeshes.begin();
         iter != m_submeshes.end(); ++iter)
    {
        Submesh* submesh = iter->ptr();
        GLVertexBuffer* vertexBuffer = NULL;

        unsigned int size = submesh->vertices()->stride() * submesh->vertices()->count();
        if (size > VertexBufferSizeThreshold)
        {
            vertexBuffer = new GLVertexBuffer(size, GL_STATIC_DRAW_ARB, submesh->vertices()->data());
            if (!vertexBuffer->isValid())
            {
                delete vertexBuffer;
                vertexBuffer = NULL;
                ok = false;
                VESTA_WARNING("Failed to create vertex buffer");
            }
        }

        // A null vertex pointer is legal and indicates that the vertex data is
        // stored in system memory instead of graphics memory.
        m_submeshBuffers.push_back(vertexBuffer);
    }

    return ok;
}


// Free any index and vertex buffers used by this mesh
// TODO: make this non-const, as it modifies the submesh buffers list. This requires
// the render() method to be non-const.
void
MeshGeometry::freeSubmeshBuffers() const
{
    for (vector<GLVertexBuffer*>::iterator iter = m_submeshBuffers.begin(); iter != m_submeshBuffers.end(); ++iter)
    {
        if (*iter)
        {
            delete *iter;
        }
    }
    m_submeshBuffers.clear();
}


static MeshGeometry*
Convert3DSMesh(Lib3dsFile* meshfile, TextureMapLoader* textureLoader)
{
    MeshGeometry* meshGeometry = new MeshGeometry();

    for (int materialIndex = 0; materialIndex < meshfile->nmaterials; ++materialIndex)
    {
        Lib3dsMaterial* material = meshfile->materials[materialIndex];
        Material* vmaterial = new Material();

        // Convert a 3ds material to VESTA material
        vmaterial->setOpacity(1.0f - material->transparency);
        vmaterial->setDiffuse(Spectrum(material->diffuse));

        if (material->shininess != 0.0f)
        {
            vmaterial->setSpecular(Spectrum(material->specular));
            vmaterial->setPhongExponent(std::pow(2.0f, 1.0f + 10.0f * material->shininess));
        }

        if (material->self_illum_flag)
        {
            vmaterial->setEmission(vmaterial->diffuse() * material->self_illum);
        }

        const string baseTextureName(material->texture1_map.name);
        if (!baseTextureName.empty())
        {
            TextureProperties texProperties;
            if ((material->texture1_map.flags & LIB3DS_TEXTURE_NO_TILE) != 0)
            {
                texProperties.addressS = TextureProperties::Clamp;
                texProperties.addressT = TextureProperties::Clamp;
            }

            if (textureLoader)
            {
                TextureMap* baseTexture = textureLoader->loadTexture(baseTextureName, texProperties);
                vmaterial->setBaseTexture(baseTexture);
            }
        }

        meshGeometry->addMaterial(vmaterial);
    }

    for (int meshIndex = 0; meshIndex < meshfile->nmeshes; ++meshIndex)
    {
        Lib3dsMesh* mesh = meshfile->meshes[meshIndex];
        if (mesh->nfaces > 0)
        {
            bool hasTextureCoords = mesh->texcos != 0;

            // Generate normals for the mesh
            float* normals = new float[mesh->nfaces * 9];
            lib3ds_mesh_calculate_vertex_normals(mesh, (float(*)[3]) normals);

            VertexPool vertexPool;

            for (int faceIndex = 0; faceIndex < mesh->nfaces; ++faceIndex)
            {
                for (int i = 0; i < 3; i++)
                {
                    int vertexIndex = mesh->faces[faceIndex].index[i];
                    vertexPool.addVec3(mesh->vertices[vertexIndex]);
                    vertexPool.addVec3(&normals[(faceIndex * 3 + i) * 3]);

                    if (hasTextureCoords)
                    {
                        // Invert the v texture coordinate, since 3ds uses a texture
                        // coordinate system that is flipped with respect to OpenGL's
                        vertexPool.addVec2(mesh->texcos[vertexIndex][0], 1.0f - mesh->texcos[vertexIndex][1]);
                    }
                }
            }

            delete[] normals;

            const VertexSpec* vertexSpec = hasTextureCoords ? &VertexSpec::PositionNormalTex : &VertexSpec::PositionNormal;
            VertexArray* vertexArray = vertexPool.createVertexArray(mesh->nfaces * 3, *vertexSpec);
            PrimitiveBatch* batch = new PrimitiveBatch(PrimitiveBatch::Triangles, mesh->nfaces);

            // Get the material for the primitive batch
            // TODO: This assumes that a single material is applied to the whole mesh; however,
            // materials can be assigned per-face (but rarely are in most 3ds files.)
            unsigned int materialIndex = Submesh::DefaultMaterialIndex;
            if (mesh->nfaces > 0)
            {
                // Use the material of the first face
                materialIndex = mesh->faces[0].material;
            }

            Submesh* submesh = new Submesh(vertexArray);
            submesh->addPrimitiveBatch(batch, materialIndex);

            meshGeometry->addSubmesh(submesh);
        }
    }

    return meshGeometry;
}



static MeshGeometry*
ConvertObjMesh(istream& in, TextureMapLoader* textureLoader, const std::string& pathName)
{
    ObjLoader loader;
    ObjMaterialLibrary* materialLibrary = NULL;

    MeshGeometry* mesh = loader.loadModel(in);
    if (mesh)
    {
        if (!loader.materialLibrary().empty())
        {
            string materialLibraryFileName = pathName + loader.materialLibrary();
            ifstream matStream(materialLibraryFileName.c_str(), ios::in);
            if (!matStream.good())
            {
                VESTA_LOG("Can't find material library file '%s' for OBJ format mesh", materialLibraryFileName.c_str());
            }
            else
            {
                ObjMaterialLibraryLoader matLoader(textureLoader);
                materialLibrary = matLoader.loadMaterials(matStream);
            }
        }

        if (materialLibrary)
        {
            const vector<string>& materials = loader.materials();
            for (unsigned int i = 0; i < materials.size(); ++i)
            {
                string materialName = materials[i];
                if (!materialName.empty())
                {
                    Material* material = materialLibrary->material(materialName);
                    if (material)
                    {
                        Material* meshMaterial = mesh->material(i);
                        if (meshMaterial)
                        {
                            *meshMaterial = *material;
                        }
                    }
                    else
                    {
                        VESTA_LOG("Missing material in OBJ file: '%s'", materialName.c_str());
                    }
                }
            }

            delete materialLibrary;
        }
    }

    return mesh;
}


/** Load a mesh from the specified file. Returns null if the
  * file was not found or if it is in an unrecognized format.
  * Currently, only 3ds and Wavefront obj files are supported.
  *
  * All textures used by the mesh will be loaded with the
  * specified TextureLoader. The TextureLoader may be null, in
  * which case no textures will be loaded for the mesh.
  */
MeshGeometry*
MeshGeometry::loadFromFile(const string& fileName, TextureMapLoader* textureLoader)
{
    // TODO: There will be more 3D formats supported eventually. Develop
    // a plugin architecture for them to keep the code organized.

    MeshGeometry* meshGeometry = NULL;

    // Get the file extension (in lower case)
    string extension;
    string::size_type dotPos = fileName.find_last_of('.');
    if (dotPos != string::npos)
    {
        extension = fileName.substr(dotPos + 1);
    }
    transform(extension.begin(), extension.end(), extension.begin(), (int(*)(int)) std::tolower);

    // Get the path of the directory containing the mesh file
    string pathName;
    string::size_type pathSepPos = fileName.find_last_of('/');
    if (pathSepPos != string::npos)
    {
        pathName = fileName.substr(0, pathSepPos + 1);
    }


    if (extension == "3ds")
    {
        // Only 3DS and Wavefront OBJ are supported right now
        Lib3dsFile* meshfile = lib3ds_file_open(fileName.c_str());
        if (!meshfile)
        {
            VESTA_LOG("MeshGeometry::loadFromFile() : Can't find mesh file '%s'", fileName.c_str());
        }
        else
        {
            meshGeometry = Convert3DSMesh(meshfile, textureLoader);
        }
    }
    else if (extension == "obj")
    {
        ifstream meshStream(fileName.c_str(), ios::in);
        if (!meshStream.good())
        {
            VESTA_LOG("MeshGeometry::loadFromFile() : Can't find mesh file '%s'", fileName.c_str());
        }
        else
        {
            meshGeometry = ConvertObjMesh(meshStream, textureLoader, pathName);
        }
    }
    else
    {
        VESTA_LOG("Unrecognized 3D mesh file extension %s", extension.c_str());
    }

    return meshGeometry;
}


