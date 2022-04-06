//
// Created by eduardo on 17/2/22.
//

#ifndef BRAKEDA3D_SHADEROBJECTSILHOUETTE_H
#define BRAKEDA3D_SHADEROBJECTSILHOUETTE_H

#include "../Render/Shader.h"
#include "../Misc/Color.h"
#include "../Misc/Tools.h"
#include "../EngineBuffers.h"
#include "../Render/Transforms.h"

class ShaderObjectSilhouette: public Shader {
public:
    ShaderObjectSilhouette(Camera3D *camera) {
        this->screenHeight = EngineSetup::get()->screenHeight;
        this->screenWidth = EngineSetup::get()->screenWidth;
        this->object = nullptr;
        this->color = Color::green();
        setPhaseRender(EngineSetup::ShadersPhaseRender::POSTUPDATE);
        this->camera = camera;
    }

    ShaderObjectSilhouette(Object3D *o, Camera3D *camera) {
        this->object = o;
        this->screenHeight = EngineSetup::get()->screenHeight;
        this->screenWidth = EngineSetup::get()->screenWidth;
        this->color = Color::green();
        this->camera = camera;
        setPhaseRender(EngineSetup::ShadersPhaseRender::POSTUPDATE);
    }

    void update() override {
        if (!isEnabled()) {
            return;
        }

        if (this->object == nullptr) {
            return;
        }

        if (!this->object->isStencilBufferEnabled()) {
            return;
        }

        auto mesh = dynamic_cast<Mesh3D*> (object);
        if (mesh == nullptr) {
            return;
        }

        mesh->updateBoundingBox();
        Vertex3D min = mesh->aabb.min;
        Transforms::cameraSpace(min, min, camera);
        min = Transforms::PerspectiveNDCSpace(min, camera->frustum);

        Point2D DP;
        Transforms::screenSpace(DP, min);

        Vertex3D max = mesh->aabb.max;
        Transforms::cameraSpace(max, max, camera);
        max = Transforms::PerspectiveNDCSpace(max, camera->frustum);

        Point2D DPmax;
        Transforms::screenSpace(DPmax, max);

        for (int y = DP.y; y < DPmax.y -1 ; y++) {
            for (int x = DP.x; x < DPmax.x -1 ; x++) {
                if (isBorderPixel(x, y) && x < screenWidth-1 && y < screenHeight - 1) {
                    if (Tools::isPixelInWindow(x, y)) {
                        EngineBuffers::getInstance()->setVideoBuffer(x, y, color.getColor());
                    }
                }
            }
        }
    }

    void setObject(Object3D *o) {
        this->object = o;
    }

    void setColor(Color c) {
        this->color = c;
    }

private:
    int screenWidth;
    int screenHeight;

    Object3D* object;
    Camera3D *camera;
    Color color;

    bool isBorderPixel(int x, int y) {
        bool isFilled = this->object->getStencilBufferValue(x, y);

        if (isFilled) return false;

        bool topLeft = this->object->getStencilBufferValue(std::max(0, x-1), std::max(0, y-1));
        bool topMiddle = this->object->getStencilBufferValue(x, std::max(0, y-1));
        bool topRight = this->object->getStencilBufferValue(std::min(screenWidth, x+1), std::max(0, y-1));

        bool centerLeft = this->object->getStencilBufferValue(std::max(0, x-1), y);
        bool centerRight = this->object->getStencilBufferValue(std::min(screenWidth, x+1), y);

        bool bottomLeft = this->object->getStencilBufferValue(std::max(0, x-1), y+1);
        bool bottomMiddle = this->object->getStencilBufferValue(x, std::min(screenHeight, y+1));
        bool bottomRight = this->object->getStencilBufferValue(std::min(screenWidth, x+1), std::min(screenHeight, y+1));

        if (topLeft || topMiddle || topRight || centerLeft || centerRight || bottomLeft || bottomMiddle || bottomRight) {
            return true;
        }

        return false;
    }
};

#endif //BRAKEDA3D_SHADEROBJECTSILHOUETTE_H
