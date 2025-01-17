//
// Created by eduardo on 3/04/22.
//

#include "GravitationalShield.h"
#include "../weapons/AmmoProjectileBody.h"
#include "../../../include/ComponentsManager.h"

GravitationalShield::GravitationalShield(
        float force,
        float factor,
        float stamina,
        float ttl)
        : GravitationalGhost(force, factor), stamina(stamina), ttl(ttl)
{
    this->startStamina = stamina;
    timeToLive.setStep(ttl);
    timeToLive.setEnabled(true);

    blink = new ShaderBlink(ComponentsManager::get()->getComponentCamera()->getCamera());
    blink->setObject(this);
    blink->setStep(0.05);
    blink->setPhaseRender(EngineSetup::ShadersPhaseRender::POSTUPDATE);
    blink->setEnabled(false);
    blink->setColor(Color::green());

    counterDamageBlink = new Counter(1);
    counterDamageBlink->setEnabled(false);

    shockWave = new ShaderShockWave(
        ComponentsManager::get()->getComponentRender()->clDeviceId,
        ComponentsManager::get()->getComponentRender()->clContext,
        ComponentsManager::get()->getComponentRender()->clCommandQueue,
        "shockWave.opencl",
        60,
        0.25f,
        1
    );
}

void GravitationalShield::onUpdate()
{
    GravitationalGhost::onUpdate();

    timeToLive.update();


    if (timeToLive.isFinished()) {
        remove();
    }
}

float GravitationalShield::getStartStamina() const {
    return startStamina;
}

void GravitationalShield::setStartStamina(float startStamina) {
    GravitationalShield::startStamina = startStamina;
}

float GravitationalShield::getStamina() const {
    return stamina;
}

void GravitationalShield::setStamina(float stamina) {
    GravitationalShield::stamina = stamina;
}

void GravitationalShield::takeDamage(float damageTaken) {
    this->stamina -= damageTaken;
    if (this->stamina <= 0) {
        remove();
    }
}

void GravitationalShield::resolveCollision(Collisionable *collisionable)
{
    Mesh3DGhost::resolveCollision(collisionable);
    auto *projectile = dynamic_cast<AmmoProjectileBody*> (collisionable);
    if (projectile != nullptr) {
        blink->setEnabled(true);
        counterDamageBlink->setEnabled(true);

        takeDamage(projectile->getWeaponType()->getDamage());
    }
}

void GravitationalShield::postUpdate() {
    Object3D::postUpdate();

    shockWave->onUpdate(getPosition());

    if (counterDamageBlink->isEnabled()) {
        counterDamageBlink->update();
        blink->update();
        if (counterDamageBlink->isFinished()) {
            blink->setEnabled(false);
        }
    }
}

GravitationalShield::~GravitationalShield()
{
    delete blink;
    delete shockWave;
    delete counterDamageBlink;
}
