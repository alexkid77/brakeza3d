
#ifndef SDL2_3D_ENGINE_DRAWABLE_H
#define SDL2_3D_ENGINE_DRAWABLE_H


#include <SDL_surface.h>
#include "Frustum.h"
#include "../Objects/Camera3D.h"
#include "Billboard.h"
#include "../../src/GUI/GUI.h"

class Drawable {
public:
    static void drawBox2D(SDL_Rect r);
    static void drawPlaneNormalVector(Plane *object, Camera3D *cam, Uint32 col);
    static void drawVertex(Vertex3D v, Camera3D *cam, Uint32 color);
    static void drawVector3D(Vector3D V, Camera3D *cam, Uint32 color);
    static void drawObject3DAxis(Object3D *object, Camera3D *cam, bool drawUp, bool drawRight, bool drawForward);
    static void drawFrustum(Frustum *f, Camera3D *cam, bool drawNP, bool drawFP, bool drawSides);
    static void drawPlane(Plane P, Camera3D *cam, Uint32 color);
    static void drawMainAxis(Camera3D *cam);
    static void drawBillboard(Billboard *B, Camera3D *cam);
    static void drawMainAxisOffset(Camera3D *cam, Vertex3D offset);

};


#endif //SDL2_3D_ENGINE_DRAWABLE_H
