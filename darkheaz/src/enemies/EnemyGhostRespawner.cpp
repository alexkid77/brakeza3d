//
// Created by eduardo on 20/3/22.
//

#include "EnemyGhostRespawner.h"
#include "../../../include/Brakeza3D.h"

EnemyGhostRespawner::EnemyGhostRespawner(EnemyGhost *object, float step)
{
    this->object = object;
    counter.setStep(step);
    counter.setEnabled(false);
    object->setEnabled(true);
    object->setAlphaEnabled(true);
    startCounter();
    Brakeza3D::get()->addObject3D(object, object->getLabel());
}

void EnemyGhostRespawner::onUpdate()
{
    if (object == nullptr) {
        return;
    }

    if (object->isRemoved()) {
        counter.setEnabled(false);
        this->setRemoved(true);
        return;
    }

    Object3D::onUpdate();

    if (!counter.isEnabled()) return;

    counter.update();

    const float increase = (Brakeza3D::get()->getDeltaTime() * 255) / counter.getStep();
    object->setAlpha(std::fmin(object->getAlpha() + increase, 255));

    if (counter.isFinished()) {
        if (object->getWeapon() != nullptr) {
            object->getWeapon()->counterCadence->setEnabled(true);
        }
        object->setAlphaEnabled(false);
        object->setAlpha(255);
        counter.setEnabled(false);
        this->setRemoved(true);
        return;
    }
}

void EnemyGhostRespawner::startCounter()
{
    this->counter.setEnabled(true);
}

