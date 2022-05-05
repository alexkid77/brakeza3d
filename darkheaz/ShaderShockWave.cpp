
#include "ShaderShockWave.h"
#include "../include/EngineBuffers.h"
#include "../include/Render/Transforms.h"
#include "../include/ComponentsManager.h"
#include "../include/Brakeza3D.h"

ShaderShockWave::ShaderShockWave(float size, float speed, float ttl)
{
    this->startSize = size;
    this->currentSize = size;
    this->waveSpeed = speed;
    this->ttlWave.setStep(ttl);
    this->ttlWave.setEnabled(true);
}

void ShaderShockWave::onUpdate(Vertex3D position)
{
    ttlWave.update();

    Point2D focalPoint = Transforms::WorldToPoint(position, ComponentsManager::get()->getComponentCamera()->getCamera());

    int edgecut = fract(this->ttlWave.getAcumulatedTime() * waveSpeed) * (float) this->w;

    auto engineBuffers = EngineBuffers::getInstance();

    auto b = this->videoBuffer;
    for (int y = 0; y < this->h; y++) {
        for (int x = 0; x < this->w; x++) {
            Point2D uv(x, y);
            Point2D modifiedUV = uv - focalPoint;
            const float res = smoothstep( (float) edgecut - currentSize , (float) edgecut + currentSize,modifiedUV.getLength());

            const float invertedRes = 1.0f - res;

            uv = uv + modifiedUV.getScaled(res * invertedRes);

            uv.x = std::clamp(uv.x, 0, this->w);
            uv.y = std::clamp(uv.y, 0, this->h);

            *(++b) = engineBuffers->getVideoBuffer(uv.x, uv.y);
        }
    }

    const float sizeDecreasing = (Brakeza3D::get()->getDeltaTime() * startSize) / ttlWave.getStep();
    currentSize -= sizeDecreasing;

    auto screenBuffer = engineBuffers->videoBuffer;
    auto currentBuffer = this->videoBuffer;

    for (int i = 0; i < bufferSize; i++, ++screenBuffer, ++currentBuffer) {
        *screenBuffer = *currentBuffer;
    }

    if (ttlWave.isFinished()) {
        currentSize = startSize;
        ttlWave.setEnabled(true);
    }
}

float ShaderShockWave::fract(float x)
{
    return x - floor(x);
}

float ShaderShockWave::smoothstep(float edge0, float edge1, float x)
{
    // Scale, bias and saturate x to 0..1 range
    x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    // Evaluate polynomial
    return x * x * (3 - 2 * x);
}