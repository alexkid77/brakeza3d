//
// Created by eduardo on 10/04/22.
//

#ifndef BRAKEDA3D_BOSSENEMY_H
#define BRAKEDA3D_BOSSENEMY_H


#include "../enemies/EnemyGhost.h"
#include "../weapons/AmmoProjectileBodyEmissor.h"

class BossEnemy : public EnemyGhost {

public:
    BossEnemy();

    void onUpdate() override;
};


#endif //BRAKEDA3D_BOSSENEMY_H
