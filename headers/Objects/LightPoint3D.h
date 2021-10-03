#ifndef SDL2_3D_ENGINE_LIGHTPOINT_H
#define SDL2_3D_ENGINE_LIGHTPOINT_H

#include "Object3D.h"
#include "../Render/Frustum.h"
#include "../Components/Camera3D.h"

class LightPoint3D : public Object3D {
public:

    LightPoint3D();

    void syncFrustum() const;

    void clearShadowMappingBuffer() const;

    float getShadowMappingBuffer(int x, int y) const;

    void setShadowMappingBuffer(int x, int y, float value) const;

    Uint32 color{};

    float kc = 1;   // constant attenuation
    float kl = 0;   // linear attenuation
    float kq = 0;   // quadratic attenuation

    float p = 100;

    Camera3D *cam;
    float *shadowMappingBuffer;
    int sizeBuffer;

    void setColor(int, int, int);

    Uint32 mixColor(Uint32 c, Vertex3D Q);

};


#endif //SDL2_3D_ENGINE_LIGHTPOINT_H
