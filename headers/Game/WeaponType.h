
#ifndef BRAKEDA3D_WEAPONTYPE_H
#define BRAKEDA3D_WEAPONTYPE_H

#include <SDL_mixer.h>
#include "../2D/WeaponAnimation.h"
#include "../Objects/SpriteDirectional3D.h"
#include "../Objects/Sprite3D.h"
#include "AmmoType.h"
#include "../Objects/Mesh3D.h"
#include "../Objects/Mesh3DAnimatedCollection.h"

#define WEAPON_MAX_ANIMATIONS 10

class WeaponType {
public:
    WeaponType(std::string label);

    bool available;
    int status = EngineSetup::getInstance()->WeaponsActions::WALKING;

    int index; // For related with Enum in Setup

    std::string label;
    std::string classname;

    AmmoType *ammo;

    std::string billboardTextureFile;

    float billboardWidth;
    float billboardHeight;

    float damage;
    float damageRadius;

    float accuracy;

    bool firing = false;

    bool keyDownHandle;
    bool keyUpHandle;

    int keyDownAnimationStatus;
    int keyUpAnimationStatus;

    int dispersion = 0;

    Mesh3D   *projectileTemplate;
    Sprite3D *markTemplate;

    int speed = 500;

    int numAnimations = 0;
    int currentAnimationIndex = 0;

    Mesh3DAnimatedCollection *animations;

    SDL_Surface *iconHUD;

    bool sniper;
    bool sniperEnabled = false;
    SDL_Surface *sniperHUD;

    WeaponType();

    void addAnimation(std::string label, std::string model, float scale, bool stopEnd);

    Mesh3DAnimated *getCurrentWeaponAnimation();
    void onUpdate(Camera3D *cam);

    void setWeaponAnimation(int);

    bool isAvailable() const;
    void setAvailable(bool available);

    void setSpeed(float speed);
    int  getSpeed() const;

    void    makeProjectileTemplate();
    Mesh3D* getProjectileTemplate();

    float getDamage();
    void  setDamage(float damage);
    float getDamageRadius() const;
    void  setDamageRadius(float damageRadius);

    Sprite3D *getMarkTemplate();
    void setupMarkTemplate(std::string path, int numFrames, int fps, float w, float h);

    void loadIconHUD(std::string file);
    void loadSniperHUD(std::string file);

    float getAccuracy() const;
    void  setAccuracy(float accuracy);

    int  getDispersion() const;
    void setDispersion(float dispersion);

    bool isFiring() const;
    void setFiring(bool firing);

    bool isKeyDownHandle() const;
    void setKeyDownHandle(bool keyDownHandle);

    bool isKeyUpHandle() const;
    void setKeyUpHandle(bool keyUpHandle);

    int  getKeyDownAnimationStatus() const;
    void setKeyDownAnimationStatus(int keyDownAnimationStatus);

    int  getKeyUpAnimationStatus() const;
    void setKeyUpAnimationStatus(int keyUpAnimationStatus);

    bool isSniper() const;
    void setSniper(bool sniper);

    bool isSniperEnabled() const;

    void setSniperEnabled(bool sniperEnabled);

    const std::string &getClassname() const;
    void  setClassname(const std::string &classname);

    const std::string &getBillboardTextureFile() const;
    void  setBillboardTextureFile(const std::string &billboardTexturePath);

    void setBillboardDimensions(int w, int h);

    int  getIndex() const;
    void setIndex(int index);

    AmmoType *getAmmoType() const;
    void      setAmmoType(AmmoType *ammo);
};


#endif //BRAKEDA3D_WEAPONTYPE_H
