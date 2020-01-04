
#ifndef BRAKEDA3D_WEAPONSMANAGER_H
#define BRAKEDA3D_WEAPONSMANAGER_H

#include <string>
#include <SDL.h>
#include "../Render/Timer.h"
#include "WeaponAnimation.h"
#include "../Game/WeaponType.h"
#include "../Objects/Camera3D.h"
#include "../Game/AmmoType.h"

class WeaponsManager {
public:
    WeaponsManager();

    int numWeapons = 0;
    int currentWeaponIndex = 0;

    float headBobOffsetY = 0;

    std::vector<WeaponType*> weaponTypes;
    std::vector<AmmoType*>   ammoTypes;

    // Global Offset When drawing
    float offsetX = 0;
    float offsetY = 0;

    void addWeaponType(std::string);

    WeaponType *getWeaponTypeByLabel(std::string label);
    WeaponType *getWeaponTypeByClassname(std::string label);
    WeaponType *getCurrentWeaponType();

    AmmoType *getAmmoTypeByClassname(std::string label);

    void onUpdate(Camera3D *cam, SDL_Surface *dst);

    void headBob(Vector3D velocity);
};


#endif //BRAKEDA3D_WEAPONSMANAGER_H
