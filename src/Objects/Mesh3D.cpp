
#include <vector>
#include <cimport.h>
#include "../../include/Objects/Mesh3D.h"
#include "../../include/Render/Logging.h"
#include "../../include/Brakeza3D.h"

Mesh3D::Mesh3D()
{
    BSPEntityIndex = -1;
    decal = false;

    this->octree = nullptr;
    this->grid = nullptr;
    this->flatTextureColor = false;
    this->enableLights = true;
    this->sharedTextures = false;
}

void Mesh3D::sendTrianglesToFrame(std::vector<Triangle *> *frameTriangles)
{
    for (auto & modelTriangle : this->modelTriangles) {
        frameTriangles->push_back(modelTriangle);
    }
}

int Mesh3D::getBspEntityIndex() const
{
    return BSPEntityIndex;
}

void Mesh3D::setBspEntityIndex(int bspEntityIndex)
{
    BSPEntityIndex = bspEntityIndex;
}

void Mesh3D::updateBoundingBox()
{
    float maxX = -9999999, minX = 9999999, maxY = -9999999, minY = 9999999, maxZ = -9999999, minZ = 9999999;

    for (auto & modelTriangle : this->modelTriangles) {
        modelTriangle->updateObjectSpace();

        maxX = std::fmax(maxX, modelTriangle->Ao.x);
        minX = std::fmin(minX, modelTriangle->Ao.x);

        maxY = std::fmax(maxY, modelTriangle->Ao.y);
        minY = std::fmin(minY, modelTriangle->Ao.y);

        maxZ = std::fmax(maxZ, modelTriangle->Ao.z);
        minZ = std::fmin(minZ, modelTriangle->Ao.z);

        //
        maxX = std::fmax(maxX, modelTriangle->Bo.x);
        minX = std::fmin(minX, modelTriangle->Bo.x);

        maxY = std::fmax(maxY, modelTriangle->Bo.y);
        minY = std::fmin(minY, modelTriangle->Bo.y);

        maxZ = std::fmax(maxZ, modelTriangle->Bo.z);
        minZ = std::fmin(minZ, modelTriangle->Bo.z);

        //
        maxX = std::fmax(maxX, modelTriangle->Co.x);
        minX = std::fmin(minX, modelTriangle->Co.x);

        maxY = std::fmax(maxY, modelTriangle->Co.y);
        minY = std::fmin(minY, modelTriangle->Co.y);

        maxZ = std::fmax(maxZ, modelTriangle->Co.z);
        minZ = std::fmin(minZ, modelTriangle->Co.z);
    }

    this->aabb.max.x = maxX;
    this->aabb.max.y = maxY;
    this->aabb.max.z = maxZ;

    this->aabb.min.x = minX;
    this->aabb.min.y = minY;
    this->aabb.min.z = minZ;

    this->aabb.updateVertices();
}

void Mesh3D::clone(Mesh3D *source)
{
    for (auto &triangle : source->modelTriangles) {
        auto *t = new Triangle(triangle->A,triangle->B,triangle->C,this);

        t->setTexture(triangle->getTexture());
        t->setFlatTextureColor(source->isFlatTextureColor());
        t->setFlatColor(source->flatColor);
        t->setEnableLights(triangle->isEnableLights());
        t->setBSPTriangle(triangle->isBSPTriangle());

        this->modelTriangles.push_back(t);
    }

    this->modelTextures = source->modelTextures;
    this->scale = source->scale;
    this->sharedTextures = true;
}

void Mesh3D::onUpdate()
{
    Object3D::onUpdate();

    if (isRemoved()) return;

    this->sendTrianglesToFrame(&ComponentsManager::get()->getComponentRender()->getFrameTriangles());

    if (EngineSetup::get()->DRAW_MESH3D_OCTREE) {
        if (this->octree != nullptr) {
            Drawable::drawOctree(this->octree, true);
        }
    }

    if (EngineSetup::get()->DRAW_MESH3D_GRID) {
        if (this->grid != nullptr) {
            Drawable::drawGrid3D(this->grid);
        }
    }

    if (EngineSetup::get()->DRAW_MESH3D_AABB) {
        this->updateBoundingBox();
        Drawable::drawAABB(&this->aabb, Color::white());
    }
}

void Mesh3D::AssimpLoadGeometryFromFile(const std::string &fileName)
{
    Logging::Log("AssimpLoadGeometryFromFile for " + fileName, "Mesh3D");

    if (!Tools::fileExists(fileName)) {
        Logging::Log("Error import 3D file not exist", "Mesh3D");
        exit(-1);
    }

    Assimp::Importer assimpImporter;
    const aiScene *scene = assimpImporter.ReadFile(
        fileName,
        aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipUVs
    );

    if (!scene) {
        Logging::Log("Error import 3D file for ASSIMP", "Mesh3D");
        exit(-1);
    }

    AssimpInitMaterials(scene, fileName);
    AssimpProcessNodes(scene, scene->mRootNode);
}

bool Mesh3D::AssimpInitMaterials(const aiScene *pScene, const std::string &Filename)
{
    // Extract the directory part from the file name
    std::string::size_type SlashIndex = Filename.find_last_of('/');
    std::string Dir;

    if (SlashIndex == std::string::npos) {
        Dir = ".";
    } else if (SlashIndex == 0) {
        Dir = "/";
    } else {
        Dir = Filename.substr(0, SlashIndex);
    }

    bool Ret = true;

    Logging::Log("ASSIMP: mNumMaterials: " + std::to_string(pScene->mNumMaterials), "Mesh3DAnimated");

    for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {

        aiMaterial *pMaterial = pScene->mMaterials[i];
        std::cout << "Import material: " << pMaterial->GetName().C_Str() << std::endl;

        if (std::string(pMaterial->GetName().C_Str()) == AI_DEFAULT_MATERIAL_NAME) {
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

            std::string FullPath = EngineSetup::get()->TEXTURES_FOLDER + base_filename;

            std::cout << "Import texture " << FullPath << " for ASSIMP Mesh" << std::endl;
            this->modelTextures.push_back(new Texture(FullPath));
        } else {
            Logging::Log("ERROR: mMaterial[" + std::to_string(i) + "]: Not valid color", "Mesh3DAnimated");
        }
        //}
    }

    return Ret;
}

void Mesh3D::AssimpProcessNodes(const aiScene *scene, aiNode *node)
{
    for (unsigned int x = 0; x < node->mNumMeshes; x++) {
        int idMesh = (int) node->mMeshes[x];
        this->AssimpLoadMesh(scene->mMeshes[idMesh]);
    }

    for (unsigned int j = 0; j < node->mNumChildren; j++) {
        AssimpProcessNodes(scene, node->mChildren[j]);
    }
}

void Mesh3D::AssimpLoadMesh(aiMesh *mesh)
{
    if (mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE) {
        Logging::Log("Skip mesh non triangle", "Mesh3D");
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

        auto t = new Triangle(V3, V2, V1, this);
        t->setFlatTextureColor(this->isFlatTextureColor());
        t->setEnableLights(this->isEnableLights());
        t->setFlatColor(this->flatColor);
        this->modelTriangles.push_back(t);

        if (!this->modelTriangles.empty()) {
            this->modelTriangles[k]->setTexture(this->modelTextures[mesh->mMaterialIndex]);
        }
    }
}

Octree *Mesh3D::getOctree() const {
    return octree;
}

Grid3D *Mesh3D::getGrid3D() const {
    return grid;
}

void Mesh3D::buildGrid3DForEmptyContainsStrategy(int sizeX, int sizeY, int sizeZ)
{
    Logging::Log("Building Grid3D for " + this->getLabel() + "(TriangleContains)", "Mesh3D");
    this->updateBoundingBox();
    this->grid = new Grid3D(
        &this->modelTriangles,
        this->aabb,
        sizeX,
        sizeY,
        sizeZ,
        Grid3D::EmptyStrategies::CONTAIN_TRIANGLES
    );
    this->grid->applyCheckCellEmptyStrategy();
}

void Mesh3D::buildGrid3DForEmptyRayIntersectionStrategy(int sizeX, int sizeY, int sizeZ, Vertex3D direction)
{
    Logging::Log("Building Grid3D for " + this->getLabel() + "(RayIntersection)", "Mesh3D");
    this->updateBoundingBox();
    this->grid = new Grid3D(
        &this->modelTriangles,
        this->aabb,
        sizeX,
        sizeY,
        sizeZ,
Grid3D::EmptyStrategies::RAY_INTERSECTION
    );
    this->grid->setRayIntersectionDirection(direction);
    this->grid->applyCheckCellEmptyStrategy();
}

void Mesh3D::buildGrid3DForEmptyDataImageStrategy(int sizeX, int sizeZ, const std::string& filename, int fixedY)
{
    Logging::Log("Building Grid3D for " + this->getLabel() + "(DataImage)", "Mesh3D");
    this->updateBoundingBox();
    this->grid = new Grid3D(
        &this->modelTriangles,
        this->aabb,
        sizeX,
        1,
        sizeZ,
Grid3D::EmptyStrategies::IMAGE_FILE
    );
    this->grid->setImageFilename(filename);
    this->grid->setFixedYImageData(fixedY);
    this->grid->applyCheckCellEmptyStrategy();
}

void Mesh3D::buildOctree()
{
    Logging::Log("Building Octree for " + this->getLabel(), "Mesh3D");
    this->updateBoundingBox();
    this->octree = new Octree(this->modelTriangles, this->aabb);
}

bool Mesh3D::isFlatTextureColor() const
{
    return flatTextureColor;
}

void Mesh3D::setFlatTextureColor(bool isFlatTextureColor)
{
    this->flatTextureColor = isFlatTextureColor;
}

bool Mesh3D::isEnableLights() const
{
    return this->enableLights;
}

void Mesh3D::setEnableLights(bool enableLights)
{
    this->enableLights = enableLights;
}

void Mesh3D::setFlatColor(const Color &flatColor) {
    Mesh3D::flatColor = flatColor;
}

const Color &Mesh3D::getFlatColor() const {
    return flatColor;
}

Mesh3D::~Mesh3D()
{
    Logging::getInstance()->Log("Delete Mesh3D: " + getLabel());

    for (auto triangle : modelTriangles) {
        delete triangle;
    }

    for (auto vertex : modelVertices) {
        delete vertex;
    }

    if (!sharedTextures) {
        for (auto texture : modelTextures) {
            delete texture;
        }
    }
}
