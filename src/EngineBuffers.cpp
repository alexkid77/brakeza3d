
#include <SDL2/SDL_surface.h>
#include "../include/EngineBuffers.h"

EngineBuffers *EngineBuffers::instance = nullptr;

EngineBuffers *EngineBuffers::getInstance() {
    if (instance == nullptr) {
        instance = new EngineBuffers();
    }

    return instance;
}

EngineBuffers::EngineBuffers() {
    EngineSetup *setup = EngineSetup::get();
    this->fireColors.resize(37);

    sizeBuffers = setup->RESOLUTION;

    depthBuffer = new float[sizeBuffers];
    videoBuffer = new Uint32[sizeBuffers];

    //make sure the fire buffer is zero in the beginning
    HUDbuffer = new Uint32[sizeBuffers];
    int h = EngineSetup::get()->screenHeight;
    int w = EngineSetup::get()->screenWidth;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int index = y * w + x;
            HUDbuffer[index] = 0;
        }
    }
    soundPackage = new SoundPackage();

    // buffer for fire shader
    int firePixelsBufferSize = setup->FIRE_WIDTH * setup->FIRE_HEIGHT;
    firePixelsBuffer = new int[firePixelsBufferSize];

    // 37 colores * 3 (rgb channels)
    this->makeFireColors();
    this->fireShaderSetup();

    goreDecalTemplates = new Sprite3D();
    goreDecalTemplates->setAutoRemoveAfterAnimation(true);
    goreDecalTemplates->addAnimation(setup->SPRITES_FOLDER + "gore/gore1", 1, 25);
    goreDecalTemplates->addAnimation(setup->SPRITES_FOLDER + "gore/gore2", 1, 25);
    goreDecalTemplates->addAnimation(setup->SPRITES_FOLDER + "gore/gore3", 1, 25);
    goreDecalTemplates->addAnimation(setup->SPRITES_FOLDER + "gore/gore4", 1, 25);
    goreDecalTemplates->addAnimation(setup->SPRITES_FOLDER + "gore/gore5", 1, 25);
    goreDecalTemplates->addAnimation(setup->SPRITES_FOLDER + "gore/gore6", 1, 25);
    goreDecalTemplates->addAnimation(setup->SPRITES_FOLDER + "gore/gore7", 1, 25);
    goreDecalTemplates->addAnimation(setup->SPRITES_FOLDER + "gore/gore8", 1, 25);
    goreDecalTemplates->addAnimation(setup->SPRITES_FOLDER + "gore/gore9", 1, 25);
    goreDecalTemplates->addAnimation(setup->SPRITES_FOLDER + "gore/gore10", 1, 25);
    goreDecalTemplates->addAnimation(setup->SPRITES_FOLDER + "gore/gore11", 1, 25);

    bloodTemplates = new Sprite3D();
    bloodTemplates->addAnimation(setup->SPRITES_FOLDER + "blood1/blood", 19, 25);
    bloodTemplates->addAnimation(setup->SPRITES_FOLDER + "blood2/blood", 18, 25);
    bloodTemplates->addAnimation(setup->SPRITES_FOLDER + "blood2/blood", 13, 25);

    gibsTemplate = new Sprite3D();
    gibsTemplate->addAnimation(setup->SPRITES_FOLDER + "gibs/gibs1", 7, 25);
    gibsTemplate->addAnimation(setup->SPRITES_FOLDER + "gibs/gibs2", 7, 25);
    gibsTemplate->addAnimation(setup->SPRITES_FOLDER + "gibs/gibs3", 8, 25);
}

void EngineBuffers::clearDepthBuffer() const {
    std::fill(depthBuffer, depthBuffer + sizeBuffers, 90000);
}

float EngineBuffers::getDepthBuffer(int x, int y) const {
    return depthBuffer[y * this->widthVideoBuffer + x];
}

float EngineBuffers::getDepthBuffer(int i) const {
    return depthBuffer[i];
}

void EngineBuffers::setDepthBuffer(int x, int y, float value) const {
    depthBuffer[y * this->widthVideoBuffer + x] = value;
}

void EngineBuffers::setDepthBuffer(const int i, const float value) const {
    depthBuffer[i] = value;
}

void EngineBuffers::setVideoBuffer(const int x, const int y, Uint32 value) const {
    videoBuffer[y * this->widthVideoBuffer + x] = value;
}

void EngineBuffers::setVideoBuffer(const int i, Uint32 value) const {
    videoBuffer[i] = value;
}

float EngineBuffers::getVideoBuffer(int x, int y) const {
    return videoBuffer[y * this->widthVideoBuffer + x];
}

void EngineBuffers::clearVideoBuffer() const {
    if (EngineSetup::get()->ENABLE_FOG) {
        std::fill(videoBuffer, videoBuffer + sizeBuffers, EngineSetup::get()->FOG_COLOR.getColor());
        return;
    }

    std::fill(videoBuffer, videoBuffer + sizeBuffers, NULL);
}

void EngineBuffers::flipVideoBufferToSurface(SDL_Surface *surface) {
    // buffer -> surface
    memcpy(&surface->pixels, &videoBuffer, sizeof(surface->pixels));
}

void EngineBuffers::makeFireColors() {
    // Populate pallete
    for (int i = 0; i < 111 / 3; i++) {
        fireColors[i] = Color(
                rgbs[i * 3 + 0],
                rgbs[i * 3 + 1],
                rgbs[i * 3 + 2]
        );

        videoBuffer[100 * 320 + i] = fireColors[i].getColor();
    }
}

void EngineBuffers::fireShaderSetup() const {
    // Set whole screen to 0 (color: 0x07,0x07,0x07)
    int FIRE_WIDTH = EngineSetup::get()->FIRE_WIDTH;
    int FIRE_HEIGHT = EngineSetup::get()->FIRE_HEIGHT;

    int firePixelsBufferSize = FIRE_HEIGHT * FIRE_WIDTH;
    for (int i = 0; i < firePixelsBufferSize; i++) {
        this->firePixelsBuffer[i] = 0;
    }

    // Set bottom line to 37 (color white: 0xFF,0xFF,0xFF)
    for (int i = 0; i < FIRE_WIDTH; i++) {
        this->firePixelsBuffer[(FIRE_HEIGHT - 1) * FIRE_WIDTH + i] = 36;
    }
}

NPCEnemyBody *EngineBuffers::getEnemyTemplateForClassname(const std::string& classname) {
    for (NPCEnemyBody *e : this->enemyTemplates) {
        if (e->getClassname() == classname) {
            return e;
        }
    }

    return nullptr;
}