//
// Created by darkhead on 1/1/20.
//

#ifndef BRAKEDA3D_ITEMWEAPONBODY_H
#define BRAKEDA3D_ITEMWEAPONBODY_H


#include "../Physics/BillboardBody.h"

class ItemWeaponBody: public BillboardBody {

    std::string weaponClassname;  // Related with weapon by classname

public:
    const std::string &getWeaponClassname() const;
    void  setWeaponClassname(const std::string &weaponClassname);
};


#endif //BRAKEDA3D_ITEMWEAPONBODY_H