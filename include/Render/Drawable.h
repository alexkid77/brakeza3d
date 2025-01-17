
#ifndef SDL2_3D_ENGINE_DRAWABLE_H
#define SDL2_3D_ENGINE_DRAWABLE_H


#include <SDL2/SDL_surface.h>
#include "Frustum.h"
#include "../Components/Camera3D.h"
#include "../Objects/Line2D.h"
#include "../Objects/Cube3D.h"
#include "Billboard.h"
#include "../Misc/Octree.h"
#include "../Misc/Grid3D.h"
#include "../Misc/PathFinder.h"

class Drawable {
public:
    static void drawBox2D(SDL_Rect r);

    static void drawVertex(Vertex3D v, Camera3D *cam, Color color);
    static void drawVertex3D(Vertex3D v, Camera3D *cam, Color color);

    static void drawVector3D(Vector3D V, Camera3D *cam, Color color);

    static void drawVector3DZBuffer(Vector3D V, Camera3D *cam, Color color);

    static void drawLine2D(Line2D L, Color color);

    static void drawLinePoints(Vertex3D from, Vertex3D to, Color color);

    static void drawLine2DZBuffer(Line2D L, Color color, float z_start, float z_increment);

    static void drawObject3DAxis(Object3D *object, Camera3D *cam, bool drawUp, bool drawRight, bool drawForward);

    static void drawFrustum(Frustum *f, Camera3D *cam, bool drawNP, bool drawFP, bool drawSides);

    static void drawPlane(Plane P, Camera3D *cam, Color color);

    static void drawMainAxis(Camera3D *cam);

    static void drawBillboard(Billboard *B, std::vector<Triangle *> *frameTriangles);

    static void drawMainAxisOffset(Camera3D *cam, Vertex3D offset);

    static void drawLightning(Vertex3D, Vertex3D, Color color);

    static void drawCrossHair();

    static void drawFadeIn();

    static void drawFadeOut();

    static void drawFacePercent(float percent);

    static void drawAABB(AABB3D *, Color);

    static void drawOctree(Octree *, bool onlyWithTriangles);

    static void drawOctreeNode(OctreeNode *, bool onlyWithTriangles);

    static void drawGrid3D(Grid3D *);

    static void drawPathInGrid(Grid3D *, std::stack<PairData>);

    static void drawPathDebugForDevelopment(Grid3D *grid, PathFinder *pathfinder);

    static void drawMainDeepMapFromCamera(int pos_x, int pos_y);
};


#endif //SDL2_3D_ENGINE_DRAWABLE_H
