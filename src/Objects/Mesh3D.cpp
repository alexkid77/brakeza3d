
#include <fstream>
#include <vector>
#include "../../headers/Objects/Mesh3D.h"
#include "../../headers/Render/Logging.h"
#include "../../headers/Brakeza3D.h"

Mesh3D::Mesh3D() {
    this->modelVertices = new Vertex3D[MAX_VERTEX_MODEL];
    this->modelTextures = new Texture[MAX_MESH_TEXTURES];

    this->numTextures = 0;

    for (int i = 0; i < MAX_MESH_TEXTURES; i++) {
        this->modelTextures[i] = Texture();
    }

    shadowCaster = false;
    BSPEntityIndex = -1;
    decal = false;

    this->octree = nullptr;
    this->grid = nullptr;
}

void Mesh3D::sendTrianglesToFrame(std::vector<Triangle *> *frameTriangles) {
    // draw triangles of mesh
    for (unsigned int i = 0; i < this->modelTriangles.size(); i++) {
        this->modelTriangles[i]->updateTextureAnimated();
        this->modelTriangles[i]->updateLightmapFrame();
        frameTriangles->push_back(this->modelTriangles[i]);
    }
}

/*void Mesh3D::setLightPoints(std::vector<LightPoint3D *> &lightPoints)
{
    this->lightPoints = lightPoints;
}*/

/*void Mesh3D::shadowMapping(LightPoint3D *lp)
{
    for (int i = 0; i < this->modelTriangles.size(); i++) {
        this->modelTriangles[i]->shadowMapping(lp);
    }
}*/

bool Mesh3D::isShadowCaster() const {
    return shadowCaster;
}

void Mesh3D::setShadowCaster(bool shadow_caster) {
    Mesh3D::shadowCaster = shadow_caster;
}

int Mesh3D::getBspEntityIndex() const {
    return BSPEntityIndex;
}

void Mesh3D::setBspEntityIndex(int bspEntityIndex) {
    BSPEntityIndex = bspEntityIndex;
}

void Mesh3D::updateBoundingBox() {
    float maxX = -9999999, minX = 9999999, maxY = -9999999, minY = 9999999, maxZ = -9999999, minZ = 9999999;

    for (int i = 0; i < this->modelTriangles.size(); i++) {
        this->modelTriangles[i]->updateObjectSpace();

        maxX = std::fmax(maxX, this->modelTriangles[i]->Ao.x);
        minX = std::fmin(minX, this->modelTriangles[i]->Ao.x);

        maxY = std::fmax(maxY, this->modelTriangles[i]->Ao.y);
        minY = std::fmin(minY, this->modelTriangles[i]->Ao.y);

        maxZ = std::fmax(maxZ, this->modelTriangles[i]->Ao.z);
        minZ = std::fmin(minZ, this->modelTriangles[i]->Ao.z);

        //
        maxX = std::fmax(maxX, this->modelTriangles[i]->Bo.x);
        minX = std::fmin(minX, this->modelTriangles[i]->Bo.x);

        maxY = std::fmax(maxY, this->modelTriangles[i]->Bo.y);
        minY = std::fmin(minY, this->modelTriangles[i]->Bo.y);

        maxZ = std::fmax(maxZ, this->modelTriangles[i]->Bo.z);
        minZ = std::fmin(minZ, this->modelTriangles[i]->Bo.z);

        //
        maxX = std::fmax(maxX, this->modelTriangles[i]->Co.x);
        minX = std::fmin(minX, this->modelTriangles[i]->Co.x);

        maxY = std::fmax(maxY, this->modelTriangles[i]->Co.y);
        minY = std::fmin(minY, this->modelTriangles[i]->Co.y);

        maxZ = std::fmax(maxZ, this->modelTriangles[i]->Co.z);
        minZ = std::fmin(minZ, this->modelTriangles[i]->Co.z);
    }

    this->aabb.max.x = maxX;
    this->aabb.max.y = maxY;
    this->aabb.max.z = maxZ;

    this->aabb.min.x = minX;
    this->aabb.min.y = minY;
    this->aabb.min.z = minZ;

    this->aabb.vertices[0] = this->aabb.max;
    this->aabb.vertices[1] = this->aabb.min;
    this->aabb.vertices[2] = Vertex3D(this->aabb.max.x, this->aabb.max.y, this->aabb.min.z);
    this->aabb.vertices[3] = Vertex3D(this->aabb.max.x, this->aabb.min.y, this->aabb.max.z);
    this->aabb.vertices[4] = Vertex3D(this->aabb.min.x, this->aabb.max.y, this->aabb.max.z);
    this->aabb.vertices[4] = Vertex3D(this->aabb.min.x, this->aabb.max.y, this->aabb.max.z);
    this->aabb.vertices[5] = Vertex3D(this->aabb.max.x, this->aabb.min.y, this->aabb.min.z);
    this->aabb.vertices[6] = Vertex3D(this->aabb.min.x, this->aabb.max.y, this->aabb.min.z);
    this->aabb.vertices[7] = Vertex3D(this->aabb.min.x, this->aabb.min.y, this->aabb.max.z);
}

void Mesh3D::copyFrom(Mesh3D *source) {
    Logging::getInstance()->Log("Mesh3D: copyFrom " + source->getLabel() + " to " + this->getLabel());

    // Triangles
    for (auto &modelTriangle : source->modelTriangles) {
        auto *t = new Triangle(
                modelTriangle->A,
                modelTriangle->B,
                modelTriangle->C,
                this
        );

        t->setTexture(modelTriangle->getTexture());

        this->modelTriangles.push_back(t);
    }

    // Textures
    this->numTextures = source->numTextures;
    this->modelTextures = source->modelTextures;
    this->scale = source->scale;
    this->source_file = source->source_file;
}

void Mesh3D::onUpdate() {
    this->sendTrianglesToFrame(&ComponentsManager::get()->getComponentRender()->getFrameTriangles());

    if (EngineSetup::getInstance()->DRAW_MESH3D_OCTREE) {
        if (this->octree != nullptr) {
            Drawable::drawOctree(this->octree, true);
        }
    }

    if (EngineSetup::getInstance()->DRAW_MESH3D_GRID) {
        if (this->grid != nullptr) {
            Drawable::drawGrid3D(this->grid);
        }
    }

    if (EngineSetup::getInstance()->DRAW_MESH3D_AABB) {
        this->updateBoundingBox();
        Drawable::drawAABB(&this->aabb, Color::white());
    }
}

bool Mesh3D::AssimpLoadGeometryFromFile(const std::string &fileName) {
    setSourceFile(fileName);

    Logging::getInstance()->Log("AssimpLoadGeometryFromFile for " + fileName);

    const aiScene *scene = importer.ReadFile(fileName, aiProcess_Triangulate |
                                                       aiProcess_JoinIdenticalVertices |
                                                       aiProcess_SortByPType |
                                                       aiProcess_FlipUVs
    );

    if (!scene) {
        Logging::getInstance()->Log("Error import 3D file for ASSIMP");
        exit(-1);
        return false;
    }

    AssimpInitMaterials(scene, fileName);
    AssimpProcessNodes(scene, scene->mRootNode);

    return true;
}

bool Mesh3D::AssimpInitMaterials(const aiScene *pScene, const std::string &Filename) {
    // Extract the directory part from the file name
    std::string::size_type SlashIndex = Filename.find_last_of("/");
    std::string Dir;

    if (SlashIndex == std::string::npos) {
        Dir = ".";
    } else if (SlashIndex == 0) {
        Dir = "/";
    } else {
        Dir = Filename.substr(0, SlashIndex);
    }

    bool Ret = true;

    Logging::getInstance()->Log("ASSIMP: mNumMaterials: " + std::to_string(pScene->mNumMaterials), "Mesh3DAnimated");

    for (uint i = 0; i < pScene->mNumMaterials; i++) {

        aiMaterial *pMaterial = pScene->mMaterials[i];
        std::cout << "Import material: " << pMaterial->GetName().C_Str() << std::endl;

        if (std::string(pMaterial->GetName().C_Str()) == AI_DEFAULT_MATERIAL_NAME) {
            this->numTextures++;
            continue;
        }
        //if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE)  >= 1) {
        aiString Path;
        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, nullptr, nullptr, nullptr, nullptr, nullptr) == AI_SUCCESS) {
            std::string p(Path.data);

            std::string base_filename = p.substr(p.find_last_of("/\\") + 1);

            if (p.substr(0, 2) == ".\\") {
                p = p.substr(2, p.size() - 2);
            }

            std::string FullPath =
                    EngineSetup::getInstance()->TEXTURES_FOLDER + this->prefix_texture_folder + base_filename;

            std::cout << "Import texture " << FullPath << " for ASSIMP Mesh" << std::endl;
            auto *t = new Texture();
            if (t->loadTGA(FullPath.c_str(), 1)) {
                this->modelTextures[this->numTextures] = *t;
                this->modelTextures[this->numTextures].loaded = true;
                this->numTextures++;
            }
        } else {
            Logging::getInstance()->Log("ERROR: mMaterial[" + std::to_string(i) + "]: Not valid color",
                                        "Mesh3DAnimated");
        }
        //}
    }

    return Ret;
}

void Mesh3D::AssimpProcessNodes(const aiScene *scene, aiNode *node) {
    for (int x = 0; x < node->mNumMeshes; x++) {
        int idMesh = node->mMeshes[x];
        this->AssimpLoadMesh(scene->mMeshes[idMesh]);
    }

    for (int j = 0; j < node->mNumChildren; j++) {
        AssimpProcessNodes(scene, node->mChildren[j]);
    }
}

void Mesh3D::AssimpLoadMesh(aiMesh *mesh) {

    if (mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE) {
        Logging::getInstance()->Log("Skip mesh non triangle");
        return;
    }

    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    std::vector<Vertex3D> localMeshVertices(mesh->mNumVertices);
    for (unsigned int j = 0; j < mesh->mNumVertices; j++) {

        aiVector3t vf = mesh->mVertices[j];

        Vertex3D v(vf.x, -vf.y, vf.z);

        const aiVector3D *pTexCoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][j]) : &Zero3D;
        v.u = pTexCoord->x;
        v.v = pTexCoord->y;

        localMeshVertices[j] = v;
    }

    for (unsigned int k = 0; k < mesh->mNumFaces; k++) {
        const aiFace &Face = mesh->mFaces[k];

        if (Face.mNumIndices < 3) continue;

        Vertex3D V1 = localMeshVertices.at(Face.mIndices[0]);
        Vertex3D V2 = localMeshVertices.at(Face.mIndices[1]);
        Vertex3D V3 = localMeshVertices.at(Face.mIndices[2]);

        this->modelTriangles.push_back(new Triangle(V3, V2, V1, this));

        if (this->numTextures > 0) {
            this->modelTriangles[k]->setTexture(&this->modelTextures[mesh->mMaterialIndex]);
        }
    }
}

const std::string &Mesh3D::getSourceFile() const {
    return source_file;
}

void Mesh3D::setSourceFile(const std::string &sourceFile) {
    source_file = sourceFile;
}

Octree *Mesh3D::getOctree() const {
    return octree;
}

Grid3D *Mesh3D::getGrid3D() const {
    return grid;
}

void Mesh3D::buildGrid3DForEmptyContainsStrategy(int sizeX, int sizeY, int sizeZ) {
    Logging::getInstance()->Log("Building Grid3D for " + this->getLabel() + "(TriangleContains)");
    this->updateBoundingBox();
    this->grid = new Grid3D(&this->modelTriangles, this->aabb, sizeX, sizeY, sizeZ,
                            Grid3D::EmptyStrategies::CONTAIN_TRIANGLES);
    this->grid->applyCheckCellEmptyStrategy();
}

void Mesh3D::buildGrid3DForEmptyRayIntersectionStrategy(int sizeX, int sizeY, int sizeZ, Vertex3D direction) {
    Logging::getInstance()->Log("Building Grid3D for " + this->getLabel() + "(RayIntersection)");
    this->updateBoundingBox();
    this->grid = new Grid3D(&this->modelTriangles, this->aabb, sizeX, sizeY, sizeZ,
                            Grid3D::EmptyStrategies::RAY_INTERSECTION);
    this->grid->setRayIntersectionDirection(direction);
    this->grid->applyCheckCellEmptyStrategy();
}

void Mesh3D::buildGrid3DForEmptyDataImageStrategy(int sizeX, int sizeZ, const std::string& filename, int fixedY) {
    Logging::getInstance()->Log("Building Grid3D for " + this->getLabel() + "(DataImage)");
    this->updateBoundingBox();
    this->grid = new Grid3D(&this->modelTriangles, this->aabb, sizeX, 1, sizeZ, Grid3D::EmptyStrategies::IMAGE_FILE);
    this->grid->setImageFilename(filename);
    this->grid->setFixedYImageData(fixedY);
    this->grid->applyCheckCellEmptyStrategy();
}

void Mesh3D::buildOctree() {
    Logging::getInstance()->Log("Building Octree for " + this->getLabel());
    this->updateBoundingBox();
    this->octree = new Octree(this->modelTriangles, this->aabb);
}
