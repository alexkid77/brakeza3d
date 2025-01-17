//
// Created by darkhead on 8/1/20.
//

#ifndef BRAKEDA3D_COMPONENTRENDER_H
#define BRAKEDA3D_COMPONENTRENDER_H

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <vector>
#include <mutex>
#include "Component.h"
#include "../Objects/Triangle3D.h"
#include "../Render/Tile.h"
#include "../EngineBuffers.h"
#include "../../include/Render/Drawable.h"
#include "../../include/Physics/BillboardBody.h"
#include "../../include/Render/Maths.h"
#include "ComponentWindow.h"
#include "ComponentCollisions.h"
#include "ComponentCamera.h"
#include "../Render/Shader.h"
#include <CL/cl.h>

class ComponentRender : public Component {
public:

    void onStart();

    void preUpdate();

    void onUpdate();

    void postUpdate();

    void onEnd();

    void onSDLPollEvent(SDL_Event *event, bool &finish);

    void onUpdateSceneObjects();

    void hiddenSurfaceRemoval();

    void hiddenSurfaceRemovalTriangle(Triangle *t);

    void hiddenSurfaceRemovalTriangleForLight(Triangle *t, LightPoint3D *l, std::vector<Triangle *> &visibleTrianglesForLight, std::vector<Triangle *> &clippedTriangles);

    void hiddenOctreeRemoval();

    void hiddenOctreeRemovalNode(OctreeNode *node, std::vector<Triangle *> &triangles);

    void drawVisibleTriangles();

    void handleTrianglesToTiles(std::vector<Triangle *> &triangles);

    void drawTilesGrid();

    void drawTriangles(std::vector<Triangle *> &visibleTriangles);

    void render(Triangle *t);

    void triangleRasterizer(Triangle *t);

    void triangleRasterizerForDepthMapping(Triangle *t, LightPoint3D *ligthpoint);

    void processPixel(Triangle *t, int bufferIndex, const int x, const int y, Fragment *, bool bilinear);

    void drawTilesTriangles(std::vector<Triangle *> *visibleTriangles);

    void drawSceneOverlappingItems();

    void initTiles();

    void drawTileTriangles(int i, std::vector<Triangle *> &trianglesToDraw);

    void softwareRasterizerForTile(Triangle *t, int minTileX, int minTileY, int maxTileX, int maxTileY);

    static void drawWireframe(Triangle *t);

    void processPixelTextureAnimated(Fragment *fragment);

    Color processPixelFog(Fragment *fragment, Color pixelColor);

    Color processPixelLights(Triangle *t, Fragment *f, Color c);

    void updateLights();

    void updateFPS(const float deltaTime);

    std::vector<Triangle *> &getFrameTriangles();

    std::vector<Triangle *> &getVisibleTriangles();

    void extractLightPointsFromObjects3D();

    void createLightPointsDepthMappings();

    std::string getUniqueGameObjectLabel();

    int fps = 0;
    int fpsFrameCounter = 0;
    float frameTime = 0;

    std::vector<LightPoint3D *> &getLightPoints();

    std::vector<Triangle *> frameTriangles;
    std::vector<Triangle *> clippedTriangles;
    std::vector<Triangle *> visibleTriangles;
    std::vector<LightPoint3D *> lightpoints;

    std::vector<Tile> tiles;
    int sizeTileWidth = (EngineSetup::get()->screenWidth / 2);
    int sizeTileHeight = (EngineSetup::get()->screenHeight / 2);
    int tilesWidth;
    int tilesHeight;
    int numTiles;
    int tilePixelsBufferSize;

    Object3D *selectedObject;

    cl_platform_id clPlatformId;
    cl_device_id clDeviceId;
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;
    cl_int ret;
    cl_context clContext;

    cl_command_queue clCommandQueue;
    cl_uint* uiInput = NULL; // Mapped Pointer to pinned Host input buffer for host processing


    Shader *getShaderByType(int id);

    const std::map<int, Shader *> &getShaders();

    Object3D* getSelectedObject();

    void setSelectedObject(Object3D *o);

    void updateSelectedObject3DInShaders(Object3D *object);

    void initOpenCL();

private:
    std::map<int, Shader*> shaders;
    void onUpdatePreUpdateShaders();
    void onUpdatePostUpdateShaders();

    Object3D *getObject3DFromClickPoint(int xClick, int yClick);

    void addShader(int id, std::string label, Shader *shader);

    void initializeShaders();

    void updateSelectedObject3D();
    void onPostUpdateSceneObjects();

    bool isPixelFullTransparent(Color &c, SDL_PixelFormat *pixelFormat);

    void OpenCLInfo();
};


#endif //BRAKEDA3D_COMPONENTRENDER_H
