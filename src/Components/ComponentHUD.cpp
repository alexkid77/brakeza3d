#include "../../headers/Components/ComponentHUD.h"
#include "../../headers/ComponentsManager.h"

ComponentHUD::ComponentHUD() {
    HUDTextures = new TexturePackage();
}

void ComponentHUD::onStart() {
    std::cout << "ComponentHUD onStart" << std::endl;

    loadImages();

    textureWriter = new TextWriter(
            ComponentsManager::get()->getComponentWindow()->renderer,
            std::string(EngineSetup::getInstance()->SPRITES_FOLDER + "conchars.png").c_str()
    );
}

void ComponentHUD::preUpdate() {
}

void ComponentHUD::onUpdate() {
    if (SETUP->DRAW_HUD && !SETUP->MENU_ACTIVE) {

        if (SETUP->DRAW_CROSSHAIR) {
            Drawable::drawCrossHair();
        }

        drawHUD();
    }
}

void ComponentHUD::postUpdate() {

}

void ComponentHUD::onEnd() {

}

void ComponentHUD::onSDLPollEvent(SDL_Event *event, bool &finish) {

}

void ComponentHUD::loadImages() {
    HUDTextures->addItem(SETUP->HUD_FOLDER + "splash.png", "splash");
    HUDTextures->addItem(SETUP->HUD_FOLDER + "hud_health.png", "health");
    HUDTextures->addItem(SETUP->HUD_FOLDER + "hud_ammo.png", "ammo");
    HUDTextures->addItem(SETUP->HUD_FOLDER + "hud.png", "hud");
    HUDTextures->addItem(SETUP->HUD_FOLDER + "loading.png", "loading");
}

void ComponentHUD::writeTextCenter(const char *text, bool bold) const {
    int totalW = EngineSetup::getInstance()->screenWidth;
    int totalH = EngineSetup::getInstance()->screenHeight;

    int xPosition = (totalW / 2) - (int) (strlen(text) * CONCHARS_CHARACTER_W) / 2;
    this->writeText(xPosition, totalH / 2, text, bold);
}

void ComponentHUD::writeText(int x, int y, const char *text, bool bold) const {
    this->textureWriter->writeText(x, y, text, bold);
}

void ComponentHUD::drawHUD() {

    int textY = 180;
    int textX = 10;
    int stepY = 10;

    ComponentsManager *componentManager = ComponentsManager::get();

    // Weapon
    if (!componentManager->getComponentWeapons()->isEmptyWeapon()) {
        this->textureWriter->writeText(textX, textY, std::string(
                "Weapon: " + componentManager->getComponentWeapons()->getCurrentWeaponType()->getClassname()).c_str(),
                                       false);
    }

    textY += stepY;

    if (SETUP->DRAW_FPS) {
        componentManager->getComponentHUD()->writeText(
                1, 10,
                std::to_string(componentManager->getComponentRender()->fps).c_str(),
                false
        );
    }

    // Ammo
    if (!componentManager->getComponentWeapons()->isEmptyWeapon()) {
        WeaponType *WeaponType = componentManager->getComponentWeapons()->getCurrentWeaponType();
        if (WeaponType->isAvailable()) {
            this->textureWriter->writeText(textX, textY, std::string(
                    "Reloads: " + std::to_string(WeaponType->getAmmoType()->getReloads())).c_str(), false);
            textY += stepY;
            this->textureWriter->writeText(textX, textY, std::string(
                    "Ammo: " + std::to_string(WeaponType->getAmmoType()->getAmount())).c_str(), false);
            textY += stepY;
        }
    }

    // Stamina
    this->textureWriter->writeText(textX, textY, std::string(
            "Health: " + std::to_string(componentManager->getComponentGame()->getPlayer()->getStamina())).c_str(),
                                   true);

    textY += stepY;

    // kills
    this->textureWriter->writeText(textX, textY, std::string(
            "Kills: " + std::to_string(componentManager->getComponentGame()->getKills())).c_str(), true);

}
