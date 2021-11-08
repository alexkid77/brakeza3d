
#ifndef SDL2_3D_ENGINE_CAMERA_H
#define SDL2_3D_ENGINE_CAMERA_H

#include <SDL2/SDL_surface.h>
#include "../Objects/Vertex3D.h"
#include "../Objects/Object3D.h"
#include "../Render/Frustum.h"
#include "../EngineSetup.h"
#include "../Render/M3.h"
#include "Component.h"
#include "ComponentWindow.h"
#include "Camera3D.h"
#include "ComponentCollisions.h"
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

typedef float vec3_t[3];

class ComponentWeapons;

class ComponentCamera : public Component {
public:
    ComponentCamera();

    void onStart();

    void preUpdate();

    void onUpdate();

    void postUpdate();

    void onEnd();

    void onSDLPollEvent(SDL_Event *event, bool &finish);

    Camera3D *camera;
    bool is_fly_mode{};

    Camera3D *getCamera() const;

    void updateCameraBSPCollider() const;

    static void drawCheckTrace(const std::string& o1, const std::string& o2);

    bool isFlyMode() const;

    void setIsFlyMode(bool isFlyMode);
};


#endif //SDL2_3D_ENGINE_CAMERA_H