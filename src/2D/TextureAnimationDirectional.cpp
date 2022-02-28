
#include "../../include/2D/TextureAnimationDirectional.h"

#include <utility>
#include "../../include/EngineSetup.h"

TextureAnimationDirectional::TextureAnimationDirectional() {
    for (int d = 0; d <= 8; d++) {
        for (int j = 0; j < ANIMATION2D_MAX_FRAMES; j++) {
            this->frames[d][j] = new Texture();
        }
    }
}

void TextureAnimationDirectional::setup(std::string file, int newNumFrames, int newFps, int newMaxTimes) {
    this->base_file = std::move(file);
    this->numFrames = newNumFrames;
    this->fps = newFps;
    this->maxTimes = newMaxTimes;
}

void TextureAnimationDirectional::loadImages() {
    for (int d = 1; d <= 8; d++) {
        for (int i = 0; i < this->getNumFrames(); i++) {
            std::string file = this->base_file + "/" + std::to_string(d) + "_" + std::to_string(i) + ".png";
            this->frames[d][i]->getImage()->loadTGA(file.c_str());
        }
    }
}

void TextureAnimationDirectional::loadImagesForZeroDirection() {
    int d = 0;
    for (int i = 0; i < this->getNumFrames(); i++) {
        std::string file = this->base_file + "/" + std::to_string(d) + "_" + std::to_string(i) + ".png";
        this->frames[0][i]->getImage()->loadTGA(file.c_str());
    }
}

int TextureAnimationDirectional::getNumFrames() const {
    return numFrames;
}

Texture *TextureAnimationDirectional::getCurrentFrame(int direction) {
    return this->frames[direction][current];
}

void TextureAnimationDirectional::nextFrame() {
    current++;

    if (current >= this->getNumFrames()) {
        current = 0;
        times++;
        if ((maxTimes != -1 && times >= maxTimes)) {
            current = getNumFrames() - 1;
        }
    }
}

void TextureAnimationDirectional::importTextures(TextureAnimationDirectional *origin, int numFrames) {
    for (int d = 0; d <= 8; d++) {
        for (int j = 0; j < numFrames; j++) {
            this->frames[d][j] = origin->frames[d][j];
        }
    }
}