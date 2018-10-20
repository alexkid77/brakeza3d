
#include "../headers/Sprite3D.h"
#include "../headers/Drawable.h"

Sprite3D::Sprite3D()
{
    this->billboard = new BillboardDirectional();

    this->width = EngineSetup::getInstance()->SPRITE3D_DEFAULT_WIDTH;
    this->height = EngineSetup::getInstance()->SPRITE3D_DEFAULT_HEIGHT;
}

BillboardDirectional *Sprite3D::getBillboard() const
{
    return billboard;
}

void Sprite3D::updateTrianglesCoordinates(Camera *cam)
{
    this->getBillboard()->updateUnconstrainedQuad( this->width, this->height, this, cam->upVector(), cam->rightVector() );
    this->getBillboard()->updateTextureFromCameraAngle(this, cam);
}

void Sprite3D::draw(Camera *cam)
{
    // Object's axis
    if (EngineSetup::getInstance()->RENDER_OBJECTS_AXIS) {
        Drawable::drawObject3DAxis(this, cam, true, true, true);
    }

    Drawable::drawBillboard(this->billboard, cam );
}

void Sprite3D::setTimer(LTimer *timer)
{
    this->timer = timer;
    this->getBillboard()->setTimer(this->timer);
}


