
#include "../../headers/Render/Billboard.h"
#include "../../headers/Render/Transforms.h"
#include "../../headers/Render/Tools.h"
#include "../../headers/Render/Drawable.h"
#include "../../headers/Objects/Triangle3D.h"
#include "../../headers/Render/Logging.h"
#include "../../headers/Render/M3.h"

Billboard::Billboard()
{
    this->texture = new Texture();
}

void Billboard::updateUnconstrainedQuad(float w, float h, Object3D *o, Vertex3D U, Vertex3D R)
{
    this->width  = w;
    this->height = h;

    Vertex3D X;
    X.x = (width/2) * R.x;
    X.y = (width/2) * R.y;
    X.z = (width/2) * R.z;

    Vertex3D Y;
    Y.x = (height/2) * U.x;
    Y.y = (height/2) * U.y;
    Y.z = (height/2) * U.z;

    Q1.x = o->getPosition()->x + X.x + Y.x;
    Q1.y = o->getPosition()->y + X.y + Y.y;
    Q1.z = o->getPosition()->z + X.z + Y.z;

    Q2.x = o->getPosition()->x - X.x + Y.x;
    Q2.y = o->getPosition()->y - X.y + Y.y;
    Q2.z = o->getPosition()->z - X.z + Y.z;

    Q3.x = o->getPosition()->x - X.x - Y.x;
    Q3.y = o->getPosition()->y - X.y - Y.y;
    Q3.z = o->getPosition()->z - X.z - Y.z;

    Q4.x = o->getPosition()->x + X.x - Y.x;
    Q4.y = o->getPosition()->y + X.y - Y.y;
    Q4.z = o->getPosition()->z + X.z - Y.z;

    Q1 = Transforms::objectToLocal(Q1, o);
    Q2 = Transforms::objectToLocal(Q2, o);
    Q3 = Transforms::objectToLocal(Q3, o);
    Q4 = Transforms::objectToLocal(Q4, o);

    Q1.u = 1; Q1.v = 1;
    Q2.u = 0; Q2.v = 1;
    Q3.u = 0; Q3.v = 0;
    Q4.u = 1; Q4.v = 0;

    T1 = Triangle(Q3, Q2, Q1, o);
    T2 = Triangle(Q4, Q3, Q1, o);

}

void Billboard::loadTexture(std::string fileName) {
    this->texture->loadTGA( fileName.c_str(), 1);
    setTrianglesTexture(this->texture);
}

bool Billboard::isDrawable() const {
    return drawable;
}

void Billboard::setDrawable(bool drawable) {
    Billboard::drawable = drawable;
}

void Billboard::setTrianglesTexture(Texture *t)
{
    this->T1.setTexture(t);
    this->T2.setTexture(t);
}

void Billboard::reassignTexture()
{
    setTrianglesTexture(this->texture);

}