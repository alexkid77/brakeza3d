
#ifndef BRAKEDA3D_BILLBOARDDIRECTIONAL_H
#define BRAKEDA3D_BILLBOARDDIRECTIONAL_H


#include "Billboard.h"
#include "AnimationDirectional2D.h"

#define DIR_C 0
#define DIR_S 1
#define DIR_SW 2
#define DIR_W 3
#define DIR_NW 4
#define DIR_N 5
#define DIR_NE 6
#define DIR_E 7
#define DIR_SE 8

#define BILLBOARD_MAX_ANIMATIONS 5


class BillboardDirectional : public Billboard {

public:
    int num_animations = 0;
    int current_animation = 0;

    int fps = 2;

    LTimer *timer;
    float last_ticks;
    float timerCurrent = 0;

    AnimationDirectional2D *animations[BILLBOARD_MAX_ANIMATIONS];

    BillboardDirectional();

    void addAnimationDirectional2D(std::string, int);
    void updateTextureFromCameraAngle(Object3D *, Camera *);
    void setAnimation(int);
    void setTimer(LTimer *);
};

#endif //BRAKEDA3D_BILLBOARDDIRECTIONAL_H
