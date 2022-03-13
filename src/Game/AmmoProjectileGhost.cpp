//
// Created by eduardo on 9/3/22.
//

#include "../../include/Game/AmmoProjectileGhost.h"

WeaponType *AmmoProjectileGhost::getWeaponType() const {
    return weaponType;
}

void AmmoProjectileGhost::setWeaponType(WeaponType *weaponType) {
    AmmoProjectileGhost::weaponType = weaponType;
}

void AmmoProjectileGhost::resolveCollision(Collisionable *collisionable) {
    auto projectile = dynamic_cast<AmmoProjectileGhost*> (collisionable);
    if (projectile != nullptr) {
        if (projectile->getParent() == getParent()) {
            return;
        }
    }

    this->remove();
}