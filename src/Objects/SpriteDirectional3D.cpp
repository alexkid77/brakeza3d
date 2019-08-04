
#include "../../headers/Objects/SpriteDirectional3D.h"
#include "../../headers/Render/Drawable.h"
#include "../../headers/Render/Logging.h"
#include "../../headers/Render/Maths.h"

SpriteDirectional3D::SpriteDirectional3D()
{
    this->billboard = new Billboard();

    this->width = EngineSetup::getInstance()->BILLBOARD_WIDTH_DEFAULT;
    this->height = EngineSetup::getInstance()->BILLBOARD_HEIGHT_DEFAULT;

    for (int i = 0; i< BILLBOARD3D_MAX_ANIMATIONS; i++) {
        this->animations[i] = new AnimationDirectional2D();
    }
}

Billboard *SpriteDirectional3D::getBillboard() const
{
    return billboard;
}

void SpriteDirectional3D::updateTrianglesCoordinates(Camera3D *cam)
{
    Vertex3D up = cam->getRotation().getTranspose() * EngineSetup::getInstance()->up;
    Vertex3D right = cam->getRotation().getTranspose() * EngineSetup::getInstance()->right;

    this->getBillboard()->updateUnconstrainedQuad( this, up, right );
    this->updateTextureFromCameraAngle(this, cam);
}

void SpriteDirectional3D::draw(Camera3D *cam)
{
    if (EngineSetup::getInstance()->RENDER_OBJECTS_AXIS) {
        Drawable::drawObject3DAxis(this, cam, true, true, true);
    }

    Drawable::drawBillboard(this->billboard, cam );
}

void SpriteDirectional3D::setTimer(Timer *timer)
{
    this->timer = timer;
}

void SpriteDirectional3D::addAnimationDirectional2D(std::string animation_folder, int num_frames)
{
    std::string full_animation_folder = EngineSetup::getInstance()->SPRITES_FOLDER + animation_folder;

    Logging::getInstance()->Log("Loading AnimationDirectional2D: " + animation_folder + " ("+ std::to_string(num_frames)+" frames)", "BILLBOARD");

    this->animations[this->num_animations]->setup(full_animation_folder, num_frames);
    this->animations[this->num_animations]->loadImages();
    this->num_animations++;
}

void SpriteDirectional3D::updateTextureFromCameraAngle(Object3D *o, Camera3D *cam)
{
    if (num_animations == 0) return;

    float angle = (int) Maths::getHorizontalAngleBetweenObject3DAndCamera(o, cam);

    int direction;

    if (angle >= 337.5f || angle < 22.5f)
        direction = DIR_N;
    else if (angle >= 22.5f && angle < 67.5f)
        direction = DIR_NE;
    else if (angle >= 67.5f && angle < 112.5f)
        direction = DIR_E;
    else if (angle >= 112.5f && angle < 157.5f)
        direction = DIR_SE;
    else if (angle >= 157.5f && angle < 202.5f)
        direction = DIR_S;
    else if (angle >= 202.5f && angle < 247.5f)
        direction = DIR_SW;
    else if (angle >= 247.5f && angle < 292.5f)
        direction = DIR_W;
    else if (angle >= 292.5f && angle < 337.5f)
        direction = DIR_NW;

    // Frame secuence control
    float deltatime = this->timer->getTicks() - this->last_ticks;
    this->last_ticks = this->timer->getTicks();
    timerCurrent += (deltatime/1000.f);

    float step = (float) 1 / this->fps;

    if (timerCurrent > step) {
        this->animations[current_animation]->nextFrame();
        timerCurrent = 0;
    }

    this->getBillboard()->setTrianglesTexture( this->animations[current_animation]->getCurrentFrame(direction) );
}

void SpriteDirectional3D::setAnimation(int index_animation)
{
    this->current_animation = index_animation;
}
