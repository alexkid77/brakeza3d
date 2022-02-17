#include "../../include/Components/ComponentRender.h"
#include "../../include/ComponentsManager.h"
#include "../../include/Misc/Parallells.h"
#include "../../include/Brakeza3D.h"
#include "../../include/Render/Transforms.h"

void ComponentRender::onStart() {
    Logging::Log("ComponentRender onStart", "ComponentRender");
    initTiles();
}

void ComponentRender::preUpdate() {
    this->updateFPS(Brakeza3D::get()->deltaTime);
}

void ComponentRender::onUpdate() {
    this->onUpdateSceneObjects();

    if (SETUP->ENABLE_LIGHTS) {
        this->extractLightPointsFromObjects3D();
        this->updateLights();
        if (EngineSetup::get()->CREATE_LIGHT_ZBUFFER){
            this->createLightPointsDepthMappings();
        }
    }

    this->hiddenSurfaceRemoval();
    this->drawVisibleTriangles();


    frameTriangles.clear();

    if (SETUP->BULLET_DEBUG_MODE) {
        ComponentsManager::get()->getComponentCollisions()->getDynamicsWorld()->debugDrawWorld();
    }

    this->drawSceneOverlappingItems();

    if (EngineSetup::get()->CREATE_LIGHT_ZBUFFER){
        for (auto & lightpoint : this->lightpoints) {
            if (!lightpoint->isEnabled()) {
                lightpoint->clearShadowMappingBuffer();
            }
        }
    }

    if (SETUP->RENDER_MAIN_AXIS) {
        Drawable::drawMainAxis(ComponentsManager::get()->getComponentCamera()->getCamera());
    }
}

void ComponentRender::postUpdate() {

}

void ComponentRender::onEnd() {
}

void ComponentRender::onSDLPollEvent(SDL_Event *event, bool &finish) {

}

std::vector<Triangle *> &ComponentRender::getFrameTriangles() {
    return frameTriangles;
}

std::vector<Triangle *> &ComponentRender::getVisibleTriangles() {
    return visibleTriangles;
}

void ComponentRender::createLightPointsDepthMappings() {

    for (auto & lightpoint : this->lightpoints) {
        if (!lightpoint->isEnabled()) {
            continue;
        }

        std::vector<Triangle *> visibleTrianglesFromLight;
        std::vector<Triangle *> clippedTrianglesFromLight;

        for (auto & ft : frameTriangles) {
            this->hiddenSurfaceRemovalTriangleForLight(ft, lightpoint, visibleTrianglesFromLight, clippedTrianglesFromLight);
        }

        /*visibleTrianglesFromLight.insert(
                visibleTrianglesFromLight.end(),
                std::make_move_iterator(clippedTrianglesFromLight.begin()),
                std::make_move_iterator(clippedTrianglesFromLight.end())
        );*/

        for (auto & t : visibleTrianglesFromLight) {
            this->triangleRasterizerForDepthMapping(t, lightpoint);
        }

        if (lightpoint->showDeepMapping) {
            lightpoint->drawDeepMap(0, 0);
        }

        if (lightpoint->showFrustum) {
            Drawable::drawFrustum(
                    lightpoint->frustum,
                    ComponentsManager::get()->getComponentCamera()->getCamera(),
                    true, true, false
            );
        }
    }
}

void ComponentRender::extractLightPointsFromObjects3D() {
    this->lightpoints.resize(0);

    std::vector<Object3D *>::iterator it;
    for (it = getSceneObjects()->begin(); it != getSceneObjects()->end();it++) {
        Object3D *object = *(it);
        LightPoint3D *lp = dynamic_cast<LightPoint3D *>(object);
        if (lp != NULL) {
            lp->clearShadowMappingBuffer();
            lightpoints.push_back(lp);
        }
    }
}

std::vector<LightPoint3D *> &ComponentRender::getLightPoints() {

    return lightpoints;
}

void ComponentRender::onUpdateSceneObjects() {
    if (!SETUP->EXECUTE_GAMEOBJECTS_ONUPDATE) return;

    std::vector<Object3D *>::iterator it;
    for (it = getSceneObjects()->begin(); it != getSceneObjects()->end();) {
        Object3D *object = *(it);

        // Check for delete
        if (object->isRemoved()) {
            getSceneObjects()->erase(it);
            continue;
        } else {
            it++;
        }

        if (!object->isEnabled()) {
            continue;
        }

        object->onUpdate();
    }
}

void ComponentRender::hiddenSurfaceRemovalTriangleForLight(Triangle *t, LightPoint3D *l, std::vector<Triangle *> &visibleTrianglesForLight, std::vector<Triangle *> &clippedTriangles) {

    t->updateObjectSpace();
    t->updateNormal();

    // back face culling (needs objectSpace)
    if (t->isBackFaceCulling(&l->getPosition())) {
        return;
    }

    // Clipping (needs objectSpace)
    /*if (t->testForClipping(l->frustum->planes, SETUP->LEFT_PLANE, SETUP->BOTTOM_PLANE)) {
        if (SETUP->ENABLE_CLIPPING) {
            t->clipping(
                l->frustum,
                l->frustum->planes,
                SETUP->LEFT_PLANE,
                SETUP->BOTTOM_PLANE,
                t->parent,
                clippedTriangles,
                t->isBSP
            );
        }

        return;
    }*/

    // Frustum Culling (needs objectSpace)
    if (!l->frustum->isVertexInside(t->Ao) && !l->frustum->isVertexInside(t->Bo) && !l->frustum->isVertexInside(t->Co) ) {
        return;
    }

    t->updateCameraSpace(l);
    t->updateOrthographicNDCSpace(l->frustum);
    t->updateScreenSpace();
    t->updateBoundingBox();
    t->updateFullArea();

    visibleTrianglesForLight.push_back(t);
}

void ComponentRender::hiddenSurfaceRemovalTriangle(Triangle *t) {
    Camera3D *cam = ComponentsManager::get()->getComponentCamera()->getCamera();

    t->updateObjectSpace();
    t->updateNormal();

    // back face culling (needs objectSpace)
    if (t->isBackFaceCulling(&cam->getPosition())) {
        return;
    }

    // Clipping (needs objectSpace)
    if (t->testForClipping(cam->frustum->planes, SETUP->LEFT_PLANE, SETUP->BOTTOM_PLANE)) {
        if (SETUP->ENABLE_CLIPPING) {
            t->clipping(
                cam->frustum,
                cam->frustum->planes,
                SETUP->LEFT_PLANE,
                SETUP->BOTTOM_PLANE,
                t->parent,
                clippedTriangles,
                t->isBSP
            );
        }

        return;
    }

    // Frustum Culling (needs objectSpace)
    if (!cam->frustum->isVertexInside(t->Ao) && !cam->frustum->isVertexInside(t->Bo) && !cam->frustum->isVertexInside(t->Co) ) {
        return;
    }

    // Triangle precached data
    // Estas operaciones las hacemos después de descartar triángulos
    // para optimización en el rasterizador por software
    t->updateCameraSpace(cam);
    t->updatePerspectiveNDCSpace(cam->frustum);
    t->updateScreenSpace();
    t->updateBoundingBox();
    t->updateFullArea();
    t->updateUVCache();

    t->drawed = false;
    visibleTriangles.emplace_back(t);
}

void ComponentRender::hiddenSurfaceRemoval() {
    for (auto & clippedTriangle : clippedTriangles) {
        delete clippedTriangle;
    }
    clippedTriangles.clear();

    visibleTriangles.clear();

    for (auto & frameTriangle : frameTriangles) {
        this->hiddenSurfaceRemovalTriangle(frameTriangle);
    }

    visibleTriangles.insert(
            visibleTriangles.end(),
            std::make_move_iterator(clippedTriangles.begin()),
            std::make_move_iterator(clippedTriangles.end())
    );

    if (SETUP->DEBUG_RENDER_INFO) {
        Logging::Log("[DEBUG_RENDER_INFO] frameTriangles: " + std::to_string(frameTriangles.size()) +
                                    ", numVisibleTriangles: " + std::to_string(visibleTriangles.size()), "Render");
    }

}

void ComponentRender::hiddenOctreeRemoval() {
    std::vector<Triangle *> newFrameTriangles;

    this->frameTriangles = newFrameTriangles;
}

void ComponentRender::hiddenOctreeRemovalNode(OctreeNode *node, std::vector<Triangle *> &triangles) {
    if (
            node->isLeaf() &&
            ComponentsManager::get()->getComponentCamera()->getCamera()->frustum->isAABBInFrustum(
                    &node->bounds
            )
            ) {
        for (auto & triangle : node->triangles) {
            triangles.push_back(triangle);
        }
    }

    for (auto & i : node->children) {
        if (i != nullptr) {
            this->hiddenOctreeRemovalNode(i, triangles);
        }
    }
}

void ComponentRender::drawVisibleTriangles() {
    if (SETUP->BASED_TILE_RENDER) {

        this->handleTrianglesToTiles(visibleTriangles);
        this->drawTilesTriangles(&visibleTriangles);

        if (SETUP->DRAW_TILES_GRID) {
            this->drawTilesGrid();
        }

        return;
    }

    this->drawTriangles(visibleTriangles);
}

void ComponentRender::handleTrianglesToTiles(std::vector<Triangle *> &triangles) {
    for (int i = 0; i < this->numTiles; i++) {
        this->tiles[i].triangleIds.clear();
    }

    for (unsigned int i = 0; i < triangles.size(); i++) {
        int tileStartX = (int) std::max((float) (triangles[i]->minX / this->sizeTileWidth), 0.0f);
        int tileEndX = (int) std::min((float) (triangles[i]->maxX / this->sizeTileWidth),
                                (float) this->tilesWidth - 1);

        int tileStartY = (int) std::max((float) (triangles[i]->minY / this->sizeTileHeight), 0.0f);
        int tileEndY = (int) std::min((float) (triangles[i]->maxY / this->sizeTileHeight),
                                (float) this->tilesHeight - 1);

        for (int y = tileStartY; y <= tileEndY; y++) {
            for (int x = tileStartX; x <= tileEndX; x++) {
                this->tiles[y * tilesWidth + x].triangleIds.emplace_back(i);
            }
        }
    }
}

void ComponentRender::drawTilesGrid() {
    for (int j = 0; j < (this->tilesWidth * this->tilesHeight); j++) {
        Tile *t = &this->tiles[j];
        Color color = Color::white();

        if (!t->triangleIds.empty()) {
            color = Color::red();
        }

        Line2D top = Line2D(t->start_x, t->start_y, t->start_x + sizeTileWidth, t->start_y);
        Line2D left = Line2D(t->start_x, t->start_y, t->start_x, t->start_y + sizeTileHeight);
        Drawable::drawLine2D(top, color);
        Drawable::drawLine2D(left, color);
    }
}

void ComponentRender::drawTriangles(std::vector<Triangle *> &visibleTriangles)
{

    std::vector<Triangle *>::iterator it;
    for (it = visibleTriangles.begin(); it != visibleTriangles.end(); it++) {
        Triangle *triangle = *(it);
        render(triangle);
    }

    if (SETUP->DRAW_MAIN_DEEP_MAPPING) {
        Drawable::drawMainDeepMapFromCamera(0, 0);
    }
}

void ComponentRender::render(Triangle *t)
{
    // degradate
    if (t->getTexture() != nullptr && SETUP->TRIANGLE_MODE_TEXTURIZED) {
        triangleRasterizer(t);
    }

    // texture
    if (SETUP->TRIANGLE_MODE_COLOR_SOLID) {
        triangleRasterizer(t);
    }

    // wireframe
    if (SETUP->TRIANGLE_MODE_WIREFRAME || (t->parent->isDecal() && SETUP->DRAW_DECAL_WIREFRAMES)) {
        drawWireframe(t);
    }

    // Pixels
    if (SETUP->TRIANGLE_MODE_PIXELS) {
        Camera3D *CC = ComponentsManager::get()->getComponentCamera()->getCamera();
        Drawable::drawVertex(t->Co, CC, Color::green());
        Drawable::drawVertex(t->Bo, CC, Color::green());
        Drawable::drawVertex(t->Co, CC, Color::green());
    }


    t->drawed = true;
}

void ComponentRender::triangleRasterizerForDepthMapping(Triangle *t, LightPoint3D *ligthpoint) {
    // Triangle setup
    int A01 = (int) (-t->As.y + t->Bs.y);
    int A12 = (int) (-t->Bs.y + t->Cs.y);
    int A20 = (int) (-t->Cs.y + t->As.y);

    int B01 = (int) (-t->Bs.x + t->As.x);
    int B12 = (int) (-t->Cs.x + t->Bs.x);
    int B20 = (int) (-t->As.x + t->Cs.x);

    Point2D startP(t->minX, t->minY);
    int w0_row = Maths::orient2d(t->Bs, t->Cs, startP);
    int w1_row = Maths::orient2d(t->Cs, t->As, startP);
    int w2_row = Maths::orient2d(t->As, t->Bs, startP);

    Fragment fragment = Fragment();

    for (int y = t->minY; y < t->maxY; y++) {
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (int x = t->minX; x < t->maxX; x++) {

            if ((w0 | w1 | w2) > 0) {

                fragment.alpha = (float) w0 * t->reciprocalFullArea;
                fragment.theta = (float) w1 * t->reciprocalFullArea;
                fragment.gamma = 1 - fragment.alpha - fragment.theta;

                fragment.depth = (fragment.alpha * t->An.z) + (fragment.theta * t->Bn.z) + (fragment.gamma * t->Cn.z);

                if (Tools::isPixelInWindow(x, y)) {
                    float buffer_shadowmapping_z = ligthpoint->getShadowMappingBuffer(x, y);
                    if (buffer_shadowmapping_z != NULL) {
                        if ( fragment.depth < buffer_shadowmapping_z) {
                            ligthpoint->setShadowMappingBuffer(x, y, fragment.depth);
                        }
                    } else {
                        ligthpoint->setShadowMappingBuffer( x, y, fragment.depth);
                    }
                }
            }

            // edge function increments
            w0 += A12;
            w1 += A20;
            w2 += A01;
        }

        w0_row += B12;
        w1_row += B20;
        w2_row += B01;
    }
}

void ComponentRender::triangleRasterizer(Triangle *t) {
    // LOD determination
    t->lod = t->processLOD();

    // Triangle setup
    int A01 = (int) (-t->As.y + t->Bs.y);
    int A12 = (int) (-t->Bs.y + t->Cs.y);
    int A20 = (int) (-t->Cs.y + t->As.y);

    int B01 = (int) (-t->Bs.x + t->As.x);
    int B12 = (int) (-t->Cs.x + t->Bs.x);
    int B20 = (int) (-t->As.x + t->Cs.x);

    Point2D startP(t->minX, t->minY);
    int w0_row = Maths::orient2d(t->Bs, t->Cs, startP);
    int w1_row = Maths::orient2d(t->Cs, t->As, startP);
    int w2_row = Maths::orient2d(t->As, t->Bs, startP);

    Fragment fragment = Fragment();
    bool bilinearInterpolation = EngineSetup::get()->TEXTURES_BILINEAR_INTERPOLATION;

    for (int y = t->minY; y < t->maxY; y++) {
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (int x = t->minX; x < t->maxX; x++) {

            if ((w0 | w1 | w2) > 0) {
                fragment.alpha = (float) w0 * t->reciprocalFullArea;
                fragment.theta = (float) w1 * t->reciprocalFullArea;
                fragment.gamma = 1 - fragment.alpha - fragment.theta;

                fragment.depth =
                        (fragment.alpha * t->An.z) + (fragment.theta * t->Bn.z) + (fragment.gamma * t->Cn.z);

                int bufferIndex = (y * SETUP->screenWidth) + x;

                if (t->parent->isDecal()) {
                    fragment.depth -= 1;
                }

                if (fragment.depth < BUFFERS->getDepthBuffer(bufferIndex)) {
                    fragment.affineUV = 1 / (fragment.alpha * (t->persp_correct_Az) +
                                              fragment.theta * (t->persp_correct_Bz) +
                                              fragment.gamma * (t->persp_correct_Cz));
                    fragment.texU = (fragment.alpha * (t->tex_u1_Ac_z) + fragment.theta * (t->tex_u2_Bc_z) +
                                      fragment.gamma * (t->tex_u3_Cc_z)) * fragment.affineUV;
                    fragment.texV = (fragment.alpha * (t->tex_v1_Ac_z) + fragment.theta * (t->tex_v2_Bc_z) +
                                      fragment.gamma * (t->tex_v3_Cc_z)) * fragment.affineUV;

                    if (t->getLightmap() != nullptr) {
                        fragment.lightU =
                                (fragment.alpha * (t->light_u1_Ac_z) + fragment.theta * (t->light_u2_Bc_z) +
                                 fragment.gamma * (t->light_u3_Cc_z)) * fragment.affineUV;
                        fragment.lightV =
                                (fragment.alpha * (t->light_v1_Ac_z) + fragment.theta * (t->light_v2_Bc_z) +
                                 fragment.gamma * (t->light_v3_Cc_z)) * fragment.affineUV;
                    }

                    this->processPixel(t, bufferIndex, x, y, &fragment, bilinearInterpolation);
                }
            }

            // edge function increments
            w0 += A12;
            w1 += A20;
            w2 += A01;
        }

        w0_row += B12;
        w1_row += B20;
        w2_row += B01;
    }
}

void ComponentRender::processPixelTextureAnimated(Fragment *fragment) {
    float cache1 = fragment->texU / SETUP->LAVA_CLOSENESS;
    float cache2 = fragment->texV / SETUP->LAVA_CLOSENESS;
    fragment->texU =
            (cache1 + SETUP->LAVA_INTENSITY * sin(SETUP->LAVA_SPEED * Brakeza3D::get()->executionTime + cache2)) *
            SETUP->LAVA_SCALE;
    fragment->texV =
            (cache2 + SETUP->LAVA_INTENSITY * sin(SETUP->LAVA_SPEED * Brakeza3D::get()->executionTime + cache1)) *
            SETUP->LAVA_SCALE;
}

Color ComponentRender::processPixelFog(Fragment *fragment, Color pixelColor) {
    float nZ = Maths::normalizeToRange(fragment->depth, 0, SETUP->FOG_DISTANCE);

    if (nZ >= 1) {
        pixelColor = SETUP->FOG_COLOR;
    } else {
        pixelColor = Tools::mixColor(pixelColor, SETUP->FOG_COLOR, nZ * SETUP->FOG_INTENSITY);
    }

    return pixelColor;
}

Color ComponentRender::processPixelLights(Triangle *t, Fragment *f, Color c)
{
    if (this->lightpoints.empty()) {
        return c;
    }

    // object space
    Vertex3D D = Vertex3D(
        f->alpha * t->Ao.x + f->theta * t->Bo.x + f->gamma * t->Co.x,
        f->alpha * t->Ao.y + f->theta * t->Bo.y + f->gamma * t->Co.y,
        f->alpha * t->Ao.z + f->theta * t->Bo.z + f->gamma * t->Co.z
    );

    Vertex3D Dl;
    Point2D DP;
    for (auto & lightpoint : this->lightpoints) {
        if (!lightpoint->isEnabled()) {
            continue;
        }

        if (SETUP->ENABLE_SHADOW_MAPPING && SETUP->CREATE_LIGHT_ZBUFFER) {
            Transforms::cameraSpace(Dl, D, lightpoint);
            Dl = Transforms::OrthographicNDCSpace(Dl, lightpoint->frustum);
            float depth = Dl.z;
            Transforms::screenSpace(DP, Dl);

            if (!Tools::isPixelInWindow(DP.x, DP.y)) {
                continue;
            }

            float bufferShadowMappingDepth = lightpoint->getShadowMappingBuffer(DP.x,DP.y);
            if (depth <= bufferShadowMappingDepth + 0.0005) {
                c = lightpoint->mixColor(c, D, f);
            }

            continue;
        }

        c = lightpoint->mixColor(c, D, f);
    }

    return c;
}

void ComponentRender::processPixel(Triangle *t, int bufferIndex, const int x, const int y, Fragment *fragment, bool bilinear)
{
    Color pixelColor(0, 0, 0);

    if (SETUP->TRIANGLE_MODE_COLOR_SOLID) {
        pixelColor = Color(
                fragment->alpha * 255,
                fragment->theta * 255,
                fragment->gamma * 255
        );
    }

    // Texture
    if (SETUP->TRIANGLE_MODE_TEXTURIZED && t->getTexture() != nullptr && !t->isFlatTextureColor()) {
        if (t->texture->getSurface(t->lod) == nullptr) return;

        if (t->getTexture()->liquid && SETUP->TRIANGLE_TEXTURES_ANIMATED) {
            // texU and texV are "animated"
            this->processPixelTextureAnimated(fragment);
        }

        if (t->parent->isDecal()) {
            if ((fragment->texU < 0 || fragment->texU > 1) || (fragment->texV < 0 || fragment->texV > 1)) {
                return;
            }
        }

        t->processPixelTexture(pixelColor, fragment->texU, fragment->texV, bilinear);

        if (!t->parent->isDecal() && t->getLightmap() != nullptr && !t->getTexture()->liquid && SETUP->ENABLE_LIGHTMAPPING) {
            Uint8 red, green, blue, alpha;
            SDL_GetRGBA(pixelColor.getColor(), t->texture->getSurface(t->lod)->format, &red, &green, &blue, &alpha);
            t->processPixelLightmap(pixelColor, fragment->lightU, fragment->lightV, red, green, blue, alpha);
        }
    }

    if (EngineSetup::get()->ENABLE_LIGHTS && t->isEnableLights()) {
        pixelColor = this->processPixelLights(t, fragment, pixelColor);
    }

    if (SETUP->ENABLE_FOG) {
        pixelColor = this->processPixelFog(fragment, pixelColor);
    }

    BUFFERS->setDepthBuffer(bufferIndex, fragment->depth);
    BUFFERS->setVideoBuffer(bufferIndex, pixelColor.getColor());
}

void ComponentRender::drawTilesTriangles(std::vector<Triangle *> *visibleTriangles)
{
    std::vector<std::thread> threads;
    for (int i = 0; i < numTiles; i++) {
        if (!tiles[i].draw) continue;

        if (SETUP->BASED_TILE_RENDER_THREADED) {
            threads.emplace_back(std::thread(ParallellDrawTileTriangles, i, visibleTriangles));
        } else {
            this->drawTileTriangles(i, *visibleTriangles);
        }
    }

    if (SETUP->BASED_TILE_RENDER_THREADED) {
        for (std::thread &th : threads) {
            if (th.joinable()) {
                th.join();
            }
        }
    }

    if (SETUP->DRAW_MAIN_DEEP_MAPPING) {
        Drawable::drawMainDeepMapFromCamera(0, 0);
    }
}

void ComponentRender::drawSceneOverlappingItems() {
    for (unsigned int i = 0; i < getSceneObjects()->size(); i++) {
        if (getSceneObjects()->operator[](i)->isEnabled()) {
            if (SETUP->RENDER_OBJECTS_AXIS) {
                Drawable::drawObject3DAxis(
                    getSceneObjects()->operator[](i),
                    ComponentsManager::get()->getComponentCamera()->getCamera(),
                    true,
                    true,
                    true
                );
            }

            // Only for meshes
            auto *lightpoint = dynamic_cast<LightPoint3D *>(getSceneObjects()->operator[](i));
            if (lightpoint != nullptr) {
                if (EngineSetup::get()->DRAW_LIGHTS_DIRECTION) {
                    Drawable::drawLinePoints(
                        lightpoint->getPosition(),
                        lightpoint->getPosition() + lightpoint->AxisForward().getInverse().getNormalize().getScaled(
                          EngineSetup::get()->LIGHTS_DIRECTION_SIZE
                        ),
                       Color::white()
                    );
                }
            }
        }
    }
}

void ComponentRender::initTiles() {
    if (SETUP->screenWidth % this->sizeTileWidth != 0) {
        printf("Bad sizetileWidth\r\n");
        exit(-1);
    }
    if (SETUP->screenHeight % this->sizeTileHeight != 0) {
        printf("Bad sizeTileHeight\r\n");
        exit(-1);
    }

    // Tiles Raster setup
    this->tilesWidth = SETUP->screenWidth / this->sizeTileWidth;
    this->tilesHeight = SETUP->screenHeight / this->sizeTileHeight;
    this->numTiles = tilesWidth * tilesHeight;
    this->tilePixelsBufferSize = this->sizeTileWidth * this->sizeTileHeight;

    Logging::Log(
            "Tiles: (" + std::to_string(tilesWidth) + "x" + std::to_string(tilesHeight) + "), Size: (" +
            std::to_string(sizeTileWidth) + "x" + std::to_string(sizeTileHeight) + ") - bufferTileSize: " +
            std::to_string(sizeTileWidth * sizeTileHeight), "Render");

    for (int y = 0; y < SETUP->screenHeight; y += this->sizeTileHeight) {
        for (int x = 0; x < SETUP->screenWidth; x += this->sizeTileWidth) {

            Tile t;

            t.draw = true;
            t.id_x = (x / this->sizeTileWidth);
            t.id_y = (y / this->sizeTileHeight);
            t.id = t.id_y * this->tilesWidth + t.id_x;
            t.start_x = x;
            t.start_y = y;

            this->tiles.emplace_back(t);
            // Load up the vector with MyClass objects

            Logging::Log(
                    "Tiles: (id:" + std::to_string(t.id) + "), (offset_x: " + std::to_string(x) + ", offset_y: " +
                    std::to_string(y) + ")", "Render");
        }
    }

    // Create local buffers and openCL buffer pointers
    for (int i = 0; i < numTiles; i++) {

        this->tiles[i].buffer = new unsigned int[tilePixelsBufferSize];
        for (int j = 0; j < tilePixelsBufferSize; j++) {
            this->tiles[i].buffer[j] = 0;
        }

        this->tiles[i].bufferDepth = new float[tilePixelsBufferSize];
        for (int j = 0; j < tilePixelsBufferSize; j++) {
            this->tiles[i].bufferDepth[j] = 0;
        }
    }
}

void ComponentRender::drawTileTriangles(int i, std::vector<Triangle *> &trianglesToDraw) {
    for (int j = 0; j < this->tiles[i].triangleIds.size(); j++) {
        int triangleId = this->tiles[i].triangleIds[j];
        Tile *t = &this->tiles[i];

        this->softwareRasterizerForTile(
                trianglesToDraw[triangleId],
                t->start_x,
                t->start_y,
                t->start_x + sizeTileWidth,
                t->start_y + sizeTileHeight
        );

        // wireframe
        if (SETUP->TRIANGLE_MODE_WIREFRAME ||
            (
                    trianglesToDraw[triangleId]->parent->isDecal() &&
                    SETUP->DRAW_DECAL_WIREFRAMES
            )
                ) {
            drawWireframe(trianglesToDraw[triangleId]);
        }

        // Pixels
        if (SETUP->TRIANGLE_MODE_PIXELS) {
            Camera3D *CC = ComponentsManager::get()->getComponentCamera()->getCamera();
            Drawable::drawVertex(trianglesToDraw[triangleId]->Co, CC, Color::green());
            Drawable::drawVertex(trianglesToDraw[triangleId]->Bo, CC, Color::green());
            Drawable::drawVertex(trianglesToDraw[triangleId]->Co, CC, Color::green());
        }
    }
}

void ComponentRender::softwareRasterizerForTile(Triangle *t, int minTileX, int minTileY, int maxTileX, int maxTileY) {
    // LOD determination
    t->lod = t->processLOD();

    // Triangle setup
    int A01 = (int) (-t->As.y + t->Bs.y);
    int A12 = (int) (-t->Bs.y + t->Cs.y);
    int A20 = (int) (-t->Cs.y + t->As.y);

    int B01 = (int) (-t->Bs.x + t->As.x);
    int B12 = (int) (-t->Cs.x + t->Bs.x);
    int B20 = (int) (-t->As.x + t->Cs.x);

    Point2D startP(t->minX, t->minY);
    int w0_row = Maths::orient2d(t->Bs, t->Cs, startP);
    int w1_row = Maths::orient2d(t->Cs, t->As, startP);
    int w2_row = Maths::orient2d(t->As, t->Bs, startP);

    Fragment fragment = Fragment();

    Vertex3D normal = t->getNormal().getNormalize();

    bool bilinear = EngineSetup::get()->TEXTURES_BILINEAR_INTERPOLATION;

    for (int y = t->minY; y < t->maxY; y++) {
        int w0 = w0_row;
        int w1 = w1_row;
        int w2 = w2_row;

        for (int x = t->minX; x < t->maxX; x++) {

            if ((w0 | w1 | w2) > 0) {
                if (!((x < minTileX || x > maxTileX) || (y < minTileY || y > maxTileY))) {

                    fragment.alpha = w0 * t->reciprocalFullArea;
                    fragment.theta = w1 * t->reciprocalFullArea;
                    fragment.gamma = 1 - fragment.alpha - fragment.theta;

                    fragment.depth = (fragment.alpha * t->An.z) + (fragment.theta * t->Bn.z) + (fragment.gamma * t->Cn.z);

                    const int bufferIndex = y * SETUP->screenWidth + x;
                    if (fragment.depth < BUFFERS->getDepthBuffer(bufferIndex)) {
                        fragment.affineUV = 1 / ((fragment.alpha * t->persp_correct_Az) +
                                                 (fragment.theta * t->persp_correct_Bz) +
                                                 (fragment.gamma * t->persp_correct_Cz));

                        fragment.texU = ((fragment.alpha * t->tex_u1_Ac_z) + (fragment.theta * t->tex_u2_Bc_z) +
                                         (fragment.gamma * t->tex_u3_Cc_z)) * fragment.affineUV;
                        fragment.texV = ((fragment.alpha * t->tex_v1_Ac_z) + (fragment.theta * t->tex_v2_Bc_z) +
                                         (fragment.gamma * t->tex_v3_Cc_z)) * fragment.affineUV;

                        fragment.normalZ = normal.x;
                        fragment.normalY = normal.y;
                        fragment.normalZ = normal.z;

                        if (t->getLightmap() != nullptr) {
                            fragment.lightU =
                                    ((fragment.alpha * t->light_u1_Ac_z) + (fragment.theta * t->light_u2_Bc_z) +
                                     (fragment.gamma * t->light_u3_Cc_z)) * fragment.affineUV;
                            fragment.lightV =
                                    ((fragment.alpha * t->light_v1_Ac_z) + (fragment.theta * t->light_v2_Bc_z) +
                                     (fragment.gamma * t->light_v3_Cc_z)) * fragment.affineUV;
                        }

                        processPixel(t, bufferIndex, x, y, &fragment, bilinear);
                    }
                }
            }

            // edge function increments
            w0 += A12;
            w1 += A20;
            w2 += A01;
        }

        w0_row += B12;
        w1_row += B20;
        w2_row += B01;
    }
}

void ComponentRender::drawWireframe(Triangle *t)
{
    Drawable::drawLine2D(Line2D(t->As.x, t->As.y, t->Bs.x, t->Bs.y), Color::green());
    Drawable::drawLine2D(Line2D(t->Bs.x, t->Bs.y, t->Cs.x, t->Cs.y), Color::green());
    Drawable::drawLine2D(Line2D(t->Cs.x, t->Cs.y, t->As.x, t->As.y), Color::green());
}

void ComponentRender::updateFPS(const float deltaTime) {
    if (!EngineSetup::get()->DRAW_FPS) return;

    frameTime += deltaTime;
    ++fpsFrameCounter;

    if (frameTime > 1000) {
        fps = fpsFrameCounter;
        frameTime = 0;
        fpsFrameCounter = 0;
    }
}

void ComponentRender::updateLights()
{
    for (auto & lightpoint : this->lightpoints) {
        if (!lightpoint->isEnabled()) {
            continue;
        }

        lightpoint->syncFrustum();
    }
}

