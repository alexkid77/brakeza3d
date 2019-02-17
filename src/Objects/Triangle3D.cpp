
#include <SDL_surface.h>
#include <cmath>
#include <list>
#include "../../headers/Objects/Triangle3D.h"
#include "../../headers/Render/Color.h"
#include "../../headers/Render/Tools.h"
#include "../../headers/Render/EngineSetup.h"
#include "../../headers/Objects/Line2D.h"
#include "../../headers/Render/EngineBuffers.h"
#include "../../headers/Render/Transforms.h"
#include "../../headers/Render/Drawable.h"
#include "../../headers/Render/M3.h"
#include "../../headers/Render/Engine.h"
#include "../../headers/Render/Logging.h"
#include "../../headers/Render/Maths.h"

Triangle::Triangle() {
    texture = NULL;
    parent = NULL;
    lightmap = new Texture();
}

Triangle::Triangle(Vertex3D A, Vertex3D B, Vertex3D C, Object3D *parent)
{
    this->A = A;
    this->B = B;
    this->C = C;

    this->parent = parent;

    // la textura se carga bajo demanda
    texture = NULL;
    this->lod = 1;
    
    lightmap = new Texture();
}


void Triangle::updateVertexSpaces(Camera3D *cam)
{
    Ao = Transforms::objectSpace(A, parent);
    Bo = Transforms::objectSpace(B, parent);
    Co = Transforms::objectSpace(C, parent);

    Ac = Transforms::cameraSpace(Ao, cam);
    Bc = Transforms::cameraSpace(Bo, cam);
    Cc = Transforms::cameraSpace(Co, cam);

    An = Transforms::NDCSpace(Ac, cam);
    Bn = Transforms::NDCSpace(Bc, cam);
    Cn = Transforms::NDCSpace(Cc, cam);

    As = Transforms::screenSpace(An, cam);
    Bs = Transforms::screenSpace(Bn, cam);
    Cs = Transforms::screenSpace(Cn, cam);

    updateNormal();

    bs1 = Maths::barycentricSide( (int) As.x, (int) As.y, Bs, Cs );
    bs2 = Maths::barycentricSide( (int) Bs.x, (int) Bs.y, Cs, As );
    bs3 = Maths::barycentricSide( (int) Cs.x, (int) Cs.y, As, Bs );

    // lightmap coordinates
    if (getLightmap()->isLightMapped()) {
        light_u1 = Ac.u;
        light_v1 = Ac.v;

        light_u2 = Bc.u;
        light_v2 = Bc.v;

        light_u3 = Cc.u;
        light_v3 = Cc.v;
    }

    // texture coordinates
    if (this->getTexture() != NULL) {
        if (is_bsp) {
            tex_u1 = Ac.u / getTexture()->getSurface(1)->w;
            tex_v1 = Ac.v / getTexture()->getSurface(1)->h;

            tex_u2 = Bc.u / getTexture()->getSurface(1)->w;
            tex_v2 = Bc.v / getTexture()->getSurface(1)->h;

            tex_u3 = Cc.u / getTexture()->getSurface(1)->w;
            tex_v3 = Cc.v / getTexture()->getSurface(1)->h;
        } else {
            tex_u1 = Ac.u;
            tex_v1 = Ac.v;

            tex_u2 = Bc.u;
            tex_v2 = Bc.v;

            tex_u3 = Cc.u;
            tex_v3 = Cc.v;
        }
    }
}

void Triangle::updateNormal()
{
    Vector3D v1 = Vector3D(this->Ao, this->Bo);
    Vector3D v2 = Vector3D(this->Ao, this->Co);

    this->normal = v1.getComponent() % v2.getComponent();
}

Vertex3D Triangle::getNormal()
{
    return this->normal;
}

void Triangle::shadowMapping(LightPoint3D *lp)
{
    this->updateVertexSpaces(lp->cam);

    if (this->isBackFaceCulling(lp->cam))  {
        return;
    }

    if ( !lp->cam->frustum->isPointInFrustum(Ao) &&
         !lp->cam->frustum->isPointInFrustum(Bo) &&
         !lp->cam->frustum->isPointInFrustum(Co)
     ) {
        return;
    }

    this->scanVerticesForShadowMapping(lp);
}

bool Triangle::draw(Camera3D *cam)
{
    this->updateVertexSpaces(cam);

    bool faceCulling = false;
    if (EngineSetup::getInstance()->TRIANGLE_BACK_FACECULLING && !isClipped()) {
        faceCulling = this->isBackFaceCulling(cam);
    }

    // Wireframe for faceculling polygons
    if (faceCulling) {
        if (EngineSetup::getInstance()->SHOW_WIREFRAME_FOR_BFC_HIDDEN_TRIANGLES) {
            Drawable::drawLine2D(Line2D(As.x, As.y, Bs.x, Bs.y), Color::yellow());
            Drawable::drawLine2D(Line2D(Bs.x, Bs.y, Cs.x, Cs.y), Color::yellow());
            Drawable::drawLine2D(Line2D(Cs.x, Cs.y, As.x, As.y), Color::yellow());
        }

        EngineBuffers::getInstance()->trianglesHidenByFaceCuling++;
        return false;
    }

    // Clipping
    if (EngineSetup::getInstance()->TRIANGLE_RENDER_CLIPPING && !isClipped()) {
        if (this->clipping(cam)) {
            return false;
        }
    }

    // Frustum Culling
    if (EngineSetup::getInstance()->TRIANGLE_FRUSTUM_CULLING && !isClipped()) {
        if (!cam->frustum->isPointInFrustum(Ao) &&
            !cam->frustum->isPointInFrustum(Bo) &&
            !cam->frustum->isPointInFrustum(Co)
        ) {
            EngineBuffers::getInstance()->trianglesOutFrustum++;
            return false;
        }
    }

    // Pixels
    if (EngineSetup::getInstance()->TRIANGLE_MODE_PIXELS ) {
        Drawable::drawVertex(Co, cam, Color::red());
        Drawable::drawVertex(Bo, cam, Color::green());
        Drawable::drawVertex(Co, cam, Color::blue());
    }

    EngineBuffers::getInstance()->trianglesDrawed++;

    if (EngineSetup::getInstance()->TRIANGLE_MODE_TEXTURIZED || EngineSetup::getInstance()->TRIANGLE_MODE_COLOR_SOLID) {
        this->scanVertices(cam);
    }

    if (EngineSetup::getInstance()->TRIANGLE_MODE_WIREFRAME) {
        if (is_colliding) {
            this->drawWireframeColor(Color::white());
        } else {
            this->drawWireframe();
        }
    }

    if (EngineSetup::getInstance()->TRIANGLE_RENDER_NORMAL) {
        this->drawNormal(cam, Color::white());
    }

    return true;
}

bool Triangle::clipping(Camera3D *cam)
{
    Vertex3D output_vertices[10] ; int num_outvertices   = 0;
    Vertex3D input_vertices[10]  ; int num_inputvertices = 0;

    input_vertices[0] = this->Ao; num_inputvertices++;
    input_vertices[1] = this->Bo; num_inputvertices++;
    input_vertices[2] = this->Co; num_inputvertices++;

    bool any_new_vertex = false;

    const int plane_init = EngineSetup::getInstance()->LEFT_PLANE;
    const int plane_end  = EngineSetup::getInstance()->BOTTOM_PLANE;

    // clip against planes
    for (int i = plane_init ; i <= plane_end ; i++) {
        bool isClip = Maths::ClippingPolygon(input_vertices, num_inputvertices, output_vertices, num_outvertices, i, cam);

        if (isClip) any_new_vertex = true;
        memcpy (&input_vertices, &output_vertices, sizeof(output_vertices));
        /*for (int j = 0; j < num_outvertices; j++) { input_vertices[j] = output_vertices[j]; }*/

        num_inputvertices = num_outvertices;
        num_outvertices = 0;
    }

    if ( any_new_vertex && num_inputvertices != 0) {
        Triangle new_triangles[10];
        int num_new_triangles = 0;

        Maths::TriangulatePolygon(num_inputvertices, input_vertices, this->getNormal(), new_triangles, num_new_triangles, parent, this->getTexture(), this->getLightmap(), true);

        for (int i = 0; i < num_new_triangles; i++) {
            EngineBuffers::getInstance()->trianglesClippingCreated++;
            new_triangles[i].is_bsp = true;
            new_triangles[i].draw(cam);
        }

        return true;
    }

    return false;
}

void Triangle::drawWireframe()
{
    Drawable::drawLine2D(Line2D(As.x, As.y, Bs.x, Bs.y), Color::red());
    Drawable::drawLine2D(Line2D(Bs.x, Bs.y, Cs.x, Cs.y), Color::green());
    Drawable::drawLine2D(Line2D(Cs.x, Cs.y, As.x, As.y), Color::blue());
}

void Triangle::drawWireframeColor(Uint32 c)
{
    Drawable::drawLine2D(Line2D(As.x, As.y, Bs.x, Bs.y), c);
    Drawable::drawLine2D(Line2D(Bs.x, Bs.y, Cs.x, Cs.y), c);
    Drawable::drawLine2D(Line2D(Cs.x, Cs.y, As.x, As.y), c);
}

// (v0 - P) . N
bool Triangle::isBackFaceCulling(Camera3D *cam)
{
    // Camera-triangle vector
    Vertex3D v = this->Ao - *cam->getPosition();

    return (v * this->getNormal()) >= 0;
}

Vertex3D Triangle::getCenter()
{
    Vertex3D A;

    Vertex3D At = this->Ao;
    Vertex3D Bt = this->Bo;
    Vertex3D Ct = this->Co;

    A.x = (At.x+Bt.x+Ct.x)/3;
    A.y = (At.y+Bt.y+Ct.y)/3;
    A.z = (At.z+Bt.z+Ct.z)/3;

    return A;
}

void Triangle::drawNormal(Camera3D *cam, Uint32 color)
{
    Drawable::drawVector3D( Vector3D( this->Ao, this->getNormal() ), cam, color );
}

void Triangle::scanVertices(Camera3D *cam)
{
    this->lod = processLOD();

    // y obtenemos los puntos en la proyección 2d
    Point2D v1 = this->As;
    Point2D v2 = this->Bs;
    Point2D v3 = this->Cs;

    // Ordenamos los vertices y puntos por su valor en 'y'
    Maths::sortPointsByY(v1, v2, v3);

    if (v2.y == v3.y) {
        this->scanBottomFlatTriangle(v1, v2, v3);
    } else if (v1.y == v2.y) {
        this->scanTopFlatTriangle(v1, v2, v3);
    } else {
        // En este caso vamos a dividir el triángulo en dos
        // para tener uno que cumpla 'bottomFlat' y el otro 'TopFlat' y necesitamos un punto extra para separar ambos
        const int x = (int) (v1.x + ((v2.y - v1.y) / (v3.y - v1.y)) * (v3.x - v1.x));
        const int y = (int) v2.y;

        const Point2D v4(x, y);

        this->scanBottomFlatTriangle(v1, v2, v4);
        this->scanTopFlatTriangle(v4, v2, v3);

        if (EngineSetup::getInstance()->TRIANGLE_DEMO_EXTRALINE_ENABLED) {
            const Line2D extraLineDemo = Line2D(v4.x, v4.y, v2.x, v2.y);
            Drawable::drawLine2D(extraLineDemo, EngineSetup::getInstance()->TRIANGLE_DEMO_EXTRALINE_COLOR);
        }
    }
}

void Triangle::scanTopFlatTriangle(Point2D pa, Point2D pb, Point2D pc)
{
    const float invslope1 = (pc.x - pa.x) / (pc.y - pa.y);
    const float invslope2 = (pc.x - pb.x) / (pc.y - pb.y);

    float curx1 = pc.x;
    float curx2 = pc.x;

    for (int scanlineY = (int) pc.y; scanlineY >= pa.y; scanlineY--) {
        this->scanLine(curx1, curx2, scanlineY);
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

void Triangle::scanBottomFlatTriangle(Point2D pa, Point2D pb, Point2D pc)
{
    const float invslope1 = (pb.x - pa.x) / (pb.y - pa.y);
    const float invslope2 = (pc.x - pa.x) / (pc.y - pa.y);

    float curx1 = pa.x;
    float curx2 = pa.x;

    for (int scanlineY = (int) pa.y; scanlineY <= pb.y; scanlineY++) {
        this->scanLine(curx1, curx2, scanlineY);
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

void Triangle::scanVerticesForShadowMapping(LightPoint3D *lp)
{
    Vertex3D Aos = this->Ao;
    Vertex3D Bos = this->Bo;
    Vertex3D Cos = this->Co;

    // Pasamos por la cámara
    Vertex3D A = Ac;
    Vertex3D B = Bc;
    Vertex3D C = Cc;

    A = Transforms::NDCSpace(A, lp->cam);
    B = Transforms::NDCSpace(B, lp->cam);
    C = Transforms::NDCSpace(C, lp->cam);

    // y obtenemos los puntos en la proyección 2d
    Point2D v1 = Transforms::screenSpace(A, lp->cam);
    Point2D v2 = Transforms::screenSpace(B, lp->cam);
    Point2D v3 = Transforms::screenSpace(C, lp->cam);

    // Ordenamos los vertices y puntos por su valor en 'y'
    Maths::sortPointsByY(v1, v2, v3);
    Maths::sortVerticesByY(A, B, C);
    Maths::sortVerticesByY(Aos, Bos, Cos);

    if (v2.y == v3.y) {
        this->scanShadowMappingBottomFlatTriangle(v1, v2, v3, A, B, C, lp);
    } else if (v1.y == v2.y) {
        this->scanShadowMappingTopFlatTriangle( v1, v2, v3, A, B, C, lp);
    } else {
        // En este caso tenemos vamos a dividir los triángulos
        // para tener uno que cumpla 'bottomFlat' y el otro 'TopFlat'
        // y necesitamos un punto extra para separar ambos.
        const int x = (v1.x + ((v2.y - v1.y) / (v3.y - v1.y)) * (v3.x - v1.x));
        const int y = v2.y;

        const Point2D v4(x, y);

        // Hayamos las coordenadas baricéntricas del punto v4 respecto al triángulo v1, v2, v3
        float alpha, theta, gamma;
        Maths::getBarycentricCoordinates(alpha, theta, gamma, v4.x, v4.y, v1, v2, v3);

        const float u = alpha * A.u + theta * B.u + gamma * C.u;
        const float v = alpha * A.v + theta * B.v + gamma * C.v;

        // Creamos un nuevo vértice que representa v4 (el nuevo punto creado) en el triángulo original
        Vertex3D D = Vertex3D(
            alpha * A.x + theta * B.x + gamma * C.x,
            alpha * A.y + theta * B.y + gamma * C.y,
            alpha * A.z + theta * B.z + gamma * C.z
        );

        D.u = u; D.v = v;

        this->scanShadowMappingBottomFlatTriangle(v1, v2, v4, A, B, D, lp);
        this->scanShadowMappingTopFlatTriangle(v2, v4, v3, B, D, C, lp);
    }
}

void Triangle::scanShadowMappingTopFlatTriangle(Point2D pa, Point2D pb, Point2D pc, Vertex3D A, Vertex3D B, Vertex3D C, LightPoint3D *lp)
{
    float invslope1 = (float) (pc.x - pa.x) / (pc.y - pa.y);
    float invslope2 = (float) (pc.x - pb.x) / (pc.y - pb.y);

    float curx1 = pc.x;
    float curx2 = pc.x;

    for (int scanlineY = pc.y; scanlineY > pa.y; scanlineY--) {
        this->scanShadowMappingLine(curx1, curx2, scanlineY, pa, pb, pc, A, B, C, lp);
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

void Triangle::scanShadowMappingBottomFlatTriangle(Point2D pa, Point2D pb, Point2D pc, Vertex3D A, Vertex3D B, Vertex3D C, LightPoint3D *lp)
{
    float invslope1 = (float) (pb.x - pa.x) / (pb.y - pa.y);
    float invslope2 = (float) (pc.x - pa.x) / (pc.y - pa.y);

    float curx1 = pa.x;
    float curx2 = pa.x;

    for (int scanlineY = pa.y; scanlineY <= pb.y; scanlineY++) {
        this->scanShadowMappingLine(curx1, curx2, scanlineY, pa, pb, pc, A, B, C, lp);
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

void Triangle::scanLine(float start_x, float end_x, const int y)
{
    if (start_x == end_x) return;

    if (start_x > end_x) {
        const int tmp = (int) start_x;
        start_x = end_x;
        end_x = tmp;
    }

    for (int x = (int) start_x ; x <= end_x; x++) {
        processPixel(Point2D(x, y));
    }
}

void Triangle::processPixel(const Point2D &pointFinal)
{
    // Hayamos las coordenadas baricéntricas del punto (x,y) respecto al triángulo As, Bs, Cs
    float alpha, theta, gamma;
    Uint32 pixelColor;

    Maths::getBarycentricCoordinatesPrecalc(alpha, theta, gamma, pointFinal.x, pointFinal.y, As, Bs, Cs, bs1, bs2, bs3);
    float z = alpha * (An.z) + theta * (Bn.z) + gamma * (Cn.z); // Homogeneous clipspace
    float ignorablePartInt;

    // EngineBuffers
    if (EngineSetup::getInstance()->TRIANGLE_RENDER_DEPTH_BUFFER) {
        const float buffer_z = EngineBuffers::getInstance()->getDepthBuffer(pointFinal.x, pointFinal.y);

        if ( z >= buffer_z ) {
            return;
        }
    }

    // Gradient
    if (EngineSetup::getInstance()->TRIANGLE_MODE_COLOR_SOLID) {
        pixelColor = (Uint32) Tools::createRGB(alpha * 255, theta * 255, gamma * 255);
    }

    // Texture
    if (EngineSetup::getInstance()->TRIANGLE_MODE_TEXTURIZED) {
        if (this->getTexture() != NULL) {

            // Correct perspective mapping
            const float z = 1 / ( alpha * (1/Ac.z) + theta * (1/Bc.z) + gamma * (1/Cc.z) );

            // lightmap coordinates
            float light_u = ( alpha * (light_u1/Ac.z) + theta * (light_u2/Bc.z) + gamma * (light_u3/Cc.z) );
            float light_v = ( alpha * (light_v1/Ac.z) + theta * (light_v2/Bc.z) + gamma * (light_v3/Cc.z) );

            // texture coordinates
            float tex_u = ( alpha * (tex_u1/Ac.z) + theta * (tex_u2/Bc.z) + gamma * (tex_u3/Cc.z) );
            float tex_v = ( alpha * (tex_v1/Ac.z) + theta * (tex_v2/Bc.z) + gamma * (tex_v3/Cc.z) );

            // apply perspective correct
            tex_u   *= z; tex_v   *= z;
            light_u *= z; light_v *= z;

            // Check for inversion U
            if (!std::signbit(tex_u)) {
                tex_u = modf(abs(tex_u) , &ignorablePartInt);
            } else {
                tex_u = 1 - modf(abs(tex_u) , &ignorablePartInt);
            }

            // Check for inversion V
            if (!std::signbit(tex_v)) {
                tex_v = modf(abs(tex_v) , &ignorablePartInt);
            } else {
                tex_v = 1 - modf(abs(tex_v) , &ignorablePartInt);
            }

            tex_u = modf(abs(tex_u) , &ignorablePartInt);
            tex_v = modf(abs(tex_v) , &ignorablePartInt);

            pixelColor = Tools::readSurfacePixelFromUV(getTexture()->getSurface(lod), tex_u, tex_v);

            if (EngineSetup::getInstance()->TEXTURES_BILINEAR_INTERPOLATION) {
                pixelColor = Tools::readSurfacePixelFromBilinearUV(getTexture()->getSurface(lod), tex_u, tex_v);
            }

            if (getLightmap()->isLightMapped() && EngineSetup::getInstance()->ENABLE_LIGHTMAPPING) {

                light_u -= getLightmap()->mins[1];
                light_u /= getLightmap()->extents[1];
                light_u  = modf(abs(light_u), &ignorablePartInt);

                light_v -= getLightmap()->mins[0];
                light_v /= getLightmap()->extents[0];
                light_v  = modf(abs(light_v), &ignorablePartInt);

                Uint32 lightmap_color = Tools::readSurfacePixelFromUV(getLightmap()->lightmap, light_v, light_u);
                Uint8 lightmap_intensity = Tools::getRedValueFromColor(lightmap_color); // RGB son iguales en un gris

                Uint8 pred, pgreen, pblue, palpha;
                SDL_GetRGBA(pixelColor, texture->getSurface(lod)->format, &pred, &pgreen, &pblue, &palpha);

                float t = lightmap_intensity * EngineSetup::getInstance()->LIGHTMAPPING_INTENSITY;

                pixelColor = (Uint32) Tools::createRGB(
                    std::min(int(pred * t), 255),
                    std::min(int(pgreen * t), 255),
                    std::min(int(pblue * t), 255)
                );

                if (EngineSetup::getInstance()->SHOW_LIGHTMAPPING) {
                    pixelColor = lightmap_color;
                }
            }

            Uint8 red, green, blue, alpha;
            SDL_GetRGBA(pixelColor, texture->getSurface(lod)->format, &red, &green, &blue, &alpha);

            if (alpha == 0) {
                return;
            }
        }
    }


    if (EngineSetup::getInstance()->ENABLE_LIGHTS) {
        Vertex3D D;

        if (this->numberLightPoints > 0) {
            // Coordenadas del punto que estamos procesando en el mundo (object space)
            float x3d = alpha * Ao.x + theta * Bo.x + gamma * Co.x;
            float y3d = alpha * Ao.y + theta * Bo.y + gamma * Co.y;
            float z3d = alpha * Ao.z + theta * Bo.z + gamma * Co.z;

            D = Vertex3D( x3d, y3d, z3d ); // Object space
        }

        for (int i = 0; i < this->numberLightPoints; i++) {
            if (!this->lightPoints[i]->isEnabled()) {
                continue;
            }

            // Color light apply
            float d = Maths::distanteBetweenpoints( *this->lightPoints[i]->getPosition(), D );
            pixelColor = Maths::mixColor(pixelColor, d, this->lightPoints[i], D);

            if (EngineSetup::getInstance()->ENABLE_SHADOW_CASTING) {
                Mesh3D *isMesh = dynamic_cast<Mesh3D*> (parent);

                if (isMesh != NULL && isMesh->isShadowCaster()) {
                    // Shadow test
                    Vertex3D Dl = Transforms::cameraSpace(D, this->lightPoints[i]->cam);
                    Dl = Transforms::NDCSpace(Dl, this->lightPoints[i]->cam);
                    const Point2D DP = Transforms::screenSpace(Dl, this->lightPoints[i]->cam);

                    if (Tools::isPixelInWindow(DP.x, DP.y)) {
                        float buffer_shadowmapping_z = this->lightPoints[i]->getShadowMappingBuffer(DP.x, DP.y);
                        if ( Dl.z > buffer_shadowmapping_z) {
                            pixelColor = Color::red();
                        }
                    }
                }
            }
        }
    }

    EngineBuffers::getInstance()->pixelesDrawed++;
    EngineBuffers::getInstance()->setDepthBuffer(pointFinal.x, pointFinal.y, z);
    EngineBuffers::getInstance()->setVideoBuffer(pointFinal.x, pointFinal.y, pixelColor);
}

void Triangle::scanShadowMappingLine(float start_x, float end_x, int y,
                        Point2D pa, Point2D pb, Point2D pc,
                        Vertex3D A, Vertex3D B, Vertex3D C, LightPoint3D *lp)
{
    float offset_self_shadow = 0.25f;

    if (start_x == end_x) return;

    // left to right
    if (start_x > end_x) {
        int tmp = (int) start_x;

        start_x = end_x;
        end_x = tmp;
    }

    float alpha, theta, gamma;

    for (int x = (int) start_x; x < end_x; x++) {
        const Point2D pointFinal(x, y);

        if (Tools::isPixelInWindow(pointFinal.x, pointFinal.y)) {
            // Hayamos las coordenadas baricéntricas del punto v4 respecto al triángulo pa, pb, pc
            Maths::getBarycentricCoordinates(alpha, theta, gamma, x, y, pa, pb, pc);

            float z = alpha * A.z + theta * B.z + gamma * C.z; // Homogeneous clipspace

            float buffer_shadowmap_z = lp->getShadowMappingBuffer(pointFinal.x, pointFinal.y);
            if (buffer_shadowmap_z != NULL) {
                if ( z < buffer_shadowmap_z ) {
                    lp->setShadowMappingBuffer(pointFinal.x, pointFinal.y, z + offset_self_shadow);
                }
            }  else {
                lp->setShadowMappingBuffer(pointFinal.x, pointFinal.y, z + offset_self_shadow);
            }
        }
    }
}

Texture *Triangle::getTexture() const
{
    return texture;
}

void Triangle::setTexture(Texture *t)
{
    texture = t;
}

Texture *Triangle::getLightmap() const
{
    return lightmap;
}

void Triangle::setLightmap(Texture *t)
{
    lightmap = t;
}

void Triangle::setLightPoints(LightPoint3D **lightPoints, int number)
{
    this->lightPoints = lightPoints;
    this->numberLightPoints = number;
}

void Triangle::setClipped(bool value)
{
    this->is_clipped = value;
}

bool Triangle::isClipped()
{
    return this->is_clipped;
}

bool Triangle::isPointInside(Vertex3D v)
{
    return Maths::PointInTriangle(v, Ao, Bo, Co);
}

bool Triangle::isCollisionWithSphere(Collider *collider, float radius, Camera3D *cam)
{
    this->is_colliding = false;

    Plane trianglePlane = Plane(this->Ao, this->Bo, this->Co);

    if (!trianglePlane.isFrontFacingTo(collider->normalizedVelocity)) {
        return false;
    }

    Vertex3D triangleNormal = trianglePlane.getNormalVector().getNormalize();

    float t0, t1;
    bool embeddedInPlane = false;

    float signedDistToTrianglePlane = trianglePlane.distance(collider->basePoint);
    float normalDotVelocity = triangleNormal * collider->frameVelocity;

    if (normalDotVelocity == 0.0f) {
        if (fabs(signedDistToTrianglePlane) >= radius) {
            // Sphere is not embedded in plane.
            // No collision possible:
            return false;
        } else {
            // sphere is embedded in plane.
            // It intersects in the whole range [0..1]
            embeddedInPlane = true;
            t0 = 0.0;
            t1 = 1.0;
        }
    } else {
        t0 = (-radius - signedDistToTrianglePlane) / normalDotVelocity;
        t1 = (radius - signedDistToTrianglePlane) / normalDotVelocity;

        // Swap so t0 < t1
        if (t0 > t1) {
            float temp = t1;
            t1 = t0;
            t0 = temp;
        }

        if (t0 > 1.0f || t1 < 0.0f) {
            return false;
        }

        if (t0 < 0.0) t0 = 0.0;
        if (t1 < 0.0) t1 = 0.0;
        if (t0 > 1.0) t0 = 1.0;
        if (t1 > 1.0) t1 = 1.0;
    }

    bool foundCollison = false;
    Vertex3D collisionPoint;
    float t = 1.0;

    if (!embeddedInPlane) {
            Vertex3D planeIntersectionPoint = (collider->basePoint - triangleNormal.getScaled(radius)) + collider->frameVelocity.getScaled(t0);
        if (this->isPointInside(planeIntersectionPoint)) {
            foundCollison = true;
            collisionPoint = planeIntersectionPoint;
            t = t0;
        }
    }

    // some commonly used terms:
    Vertex3D base = collider->basePoint;
    Vertex3D velocity = collider->frameVelocity;

    float velocitySquaredLength = velocity.squaredLength();
    float a, b, c; // Params for equation
    float newT;

    if (!foundCollison) {
        a = velocitySquaredLength;
        // P1
        b = 2.0 * (velocity * (base - this->Ao));
        c = (this->Ao - base).squaredLength() - 1.0;
        if (Maths::getLowestRoot(a, b, c, t, &newT)) {
            t = newT;
            foundCollison = true;
            collisionPoint = this->Ao;
        }
        // P2
        b = 2.0 * (velocity * (base - this->Bo));
        c = (this->Bo - base).squaredLength() - 1.0;
        if (Maths::getLowestRoot(a, b, c, t, &newT)) {
            t = newT;
            foundCollison = true;
            collisionPoint = this->Bo;
        }

        // P3
        b = 2.0 * (velocity * (base - this->Co));
        c = (this->Co - base).squaredLength() - 1.0;
        if (Maths::getLowestRoot(a, b, c, t, &newT)) {
            t = newT;
            foundCollison = true;
            collisionPoint = this->Co;
        }
    }

    if (!foundCollison) {
        // Check agains edges:
        // p1 -> p2:
        Vertex3D edge = this->Bo - this->Ao;
        Vertex3D baseToVertex = this->Ao - base;
        float edgeSquaredLength = edge.squaredLength();
        float edgeDotVelocity = edge * velocity;
        float edgeDotBaseToVertex = edge * baseToVertex;
        // Calculate parameters for equation
        a = edgeSquaredLength * -velocitySquaredLength + (edgeDotVelocity * edgeDotVelocity);
        b = edgeSquaredLength * ( 2 * (velocity * baseToVertex)) - 2.0 * (edgeDotVelocity * edgeDotBaseToVertex);
        c = edgeSquaredLength * ( 1 - baseToVertex.squaredLength()) + (edgeDotBaseToVertex * edgeDotBaseToVertex);
        // Does the swept sphere collide against infinite edge?
        if(Maths::getLowestRoot(a, b, c, t, &newT)) {
            // Check if intersection is within line segment:
            float f = (edgeDotVelocity*newT - edgeDotBaseToVertex) / edgeSquaredLength;
            if (f >= 0.0 && f <= 1.0) {
                // intersection took place within segment.
                t = newT;
                foundCollison = true;
                collisionPoint = this->Ao + edge.getScaled(f);
            }
        }

        // p2 -> p3:
        edge = this->Co - this->Bo;
        baseToVertex = this->Bo - base;
        edgeSquaredLength = edge.squaredLength();
        edgeDotVelocity = edge * velocity;
        edgeDotBaseToVertex = edge * baseToVertex;
        // Calculate parameters for equation
        a = edgeSquaredLength * -velocitySquaredLength + (edgeDotVelocity * edgeDotVelocity);
        b = edgeSquaredLength * ( 2 * (velocity * baseToVertex) ) - 2.0 * (edgeDotVelocity * edgeDotBaseToVertex);
        c = edgeSquaredLength * ( 1 - baseToVertex.squaredLength() ) + (edgeDotBaseToVertex * edgeDotBaseToVertex);
        if (Maths::getLowestRoot(a, b, c, t, &newT)) {
            float f = (edgeDotVelocity*newT - edgeDotBaseToVertex) / edgeSquaredLength;
            if (f >= 0.0 && f <= 1.0) {
                t = newT;
                foundCollison = true;
                collisionPoint = this->Bo + edge.getScaled(f);
            }
        }
        // p3 -> p1:
        edge = this->Ao - this->Co;
        baseToVertex = this->Co - base;
        edgeSquaredLength = edge.squaredLength();
        edgeDotVelocity = edge * velocity;
        edgeDotBaseToVertex = edge * baseToVertex;
        // Calculate parameters for equation
        a = edgeSquaredLength * -velocitySquaredLength + (edgeDotVelocity * edgeDotVelocity);
        b = edgeSquaredLength * ( 2 * (velocity * baseToVertex)) - 2.0 * (edgeDotVelocity * edgeDotBaseToVertex);
        c = edgeSquaredLength * ( 1 - baseToVertex.squaredLength()) + (edgeDotBaseToVertex * edgeDotBaseToVertex);
        if (Maths::getLowestRoot(a, b, c, t, &newT)) {
            float f = (edgeDotVelocity*newT - edgeDotBaseToVertex) / edgeSquaredLength;
            if (f >= 0.0 && f <= 1.0) {
                t = newT;
                foundCollison = true;
                collisionPoint = this->Co + edge.getScaled(f);
            }
        }
    }

    if (foundCollison) {
        float distToCollision = t * collider->frameVelocity.getModule();

        if (!collider->foundCollision || distToCollision < collider->nearestDistance) {

            collider->nearestDistance = distToCollision;
            collider->intersectionPoint = collisionPoint;

            collider->foundCollision = true;
            this->is_colliding = true;

            // Check if is player is on ground
            Vector3D down = Vector3D(collider->basePoint, collider->basePoint + Vertex3D(0, 1, 0).getScaled(radius+1));
            int on_ground = Maths::isVector3DClippingPlane(trianglePlane, down);
            if (on_ground > 1) {
                cam->collider->onGround = true;
            }
        }
    }

    return foundCollison;
}

int Triangle::processLOD()
{
    int lod = EngineSetup::getInstance()->LOAD_OF_DETAIL;

    if (getTexture() == NULL) return 0;

    if (getTexture()->isMipMapped() && EngineSetup::getInstance()->ENABLE_MIPMAPPING) {
        float area_screen = Maths::TriangleArea(As.x, As.y, Bs.x, Bs.y, Cs.x, Cs.y);
        float area_texture = getTexture()->getAreaForVertices(A, B, C, 1);

        float r = area_texture / area_screen;

        int triangle_lod = (int) floor(r);
        int clamped_lod = 1;

        // Range LOD selection
        if (triangle_lod < 10) {
            clamped_lod = 1;
        } else if (triangle_lod >= 10 && triangle_lod  < 15) {
            clamped_lod = 2;
        } else if (triangle_lod >= 15 && triangle_lod  < 25) {
            clamped_lod = 4;
        } else if (triangle_lod > 25) {
            clamped_lod = 8;
        }

        lod =  clamped_lod;
    }

    return lod;
}
