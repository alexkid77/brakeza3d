#include "../../include/Components/ComponentGame.h"
#include "../../include/Components/ComponentCollisions.h"
#include "../../include/Brakeza3D.h"
#include "../../include/Game/AmmoProjectileBody.h"
#include "../../include/Game/ItemHealthGhost.h"
#include "../../include/Game/ItemWeaponGhost.h"
#include "../../include/Game/ItemEnergyGhost.h"

#define FREELOOK false
#define SPLASH_TIME 4.0f
#define FADE_SPEED 0.05f

ComponentGame::ComponentGame()
{
    player = new Player();
}

void ComponentGame::onStart()
{
    Logging::Log("ComponentGame onStart", "ComponentGame");
    setGameState(EngineSetup::GameState::NONE);
    fadeToGameState = new FaderToGameStates(
            Color(0, 255, 0),
        0.01f,
        EngineSetup::GameState::SPLASH,
        false
    );

    shaderAutoScrollSpeed = Vertex3D(0, 0.2, 0);
    imageCredits = new Image(SETUP->IMAGES_FOLDER + "credits.png");
    imageHelp = new Image(SETUP->IMAGES_FOLDER + SETUP->DEFAULT_HELP_IMAGE);
    imageSplash = new Image(SETUP->IMAGES_FOLDER + SETUP->LOGO_BRAKEZA);
    splashCounter.setStep(SPLASH_TIME);

    ComponentsManager::get()->getComponentCollisions()->initBulletSystem();

    playerStartPosition = Vertex3D(-40, 2990, Z_COORDINATE_GAMEPLAY);
    ComponentsManager::get()->getComponentCamera()->getCamera()->setPosition(Vertex3D(0, -1000, -1000));

    ComponentsManager::get()->getComponentCamera()->setAutoScroll(false);
    ComponentsManager::get()->getComponentCamera()->setAutoScrollSpeed(Vertex3D(0, -0.0, 0));

    ComponentsManager::get()->getComponentCamera()->setFreeLook(FREELOOK);
    ComponentsManager::get()->getComponentInput()->setEnabled(FREELOOK);
    ComponentsManager::get()->getComponentMenu()->setEnabled(false);

    loadBackgroundImageShader();
    loadPlayer();
    loadWeapons();
    loadLevels();
}

void ComponentGame::preUpdate()
{
    if (getGameState() == EngineSetup::GameState::SPLASH) {
        splashCounter.update();
        if (splashCounter.isFinished() && splashCounter.isEnabled()) {
            splashCounter.setEnabled(false);
            makeFadeToGameState(EngineSetup::GameState::MENU);
        }

        imageSplash->drawFlat(0, 0);
    }
}

void ComponentGame::onUpdate() {
    EngineSetup::GameState state = getGameState();

    if (state == EngineSetup::GameState::GAMING) {
        onUpdateIA();
        blockPlayerPositionInCamera();
        checkForEndLevel();
    }

    if (state == EngineSetup::GameState::PRESSKEY_GAMEOVER) {
        ComponentsManager::get()->getComponentHUD()->writeTextMiddleScreen("congratulations! END GAME...", false);
    }

    if (state == EngineSetup::GameState::PRESSKEY_NEWLEVEL) {
        ComponentsManager::get()->getComponentHUD()->writeTextMiddleScreen("press a key to START...", false);
    }

    if (state == EngineSetup::GameState::PRESSKEY_BY_DEAD) {
        ComponentsManager::get()->getComponentHUD()->writeTextMiddleScreen("you are died...", false);
    }

    if (state == EngineSetup::GameState::HELP) {
        imageHelp->drawFlat(0, 0);
    }

    if (state == EngineSetup::GameState::CREDITS) {
        imageCredits->drawFlat(0, 0);
    }

    getFadeToGameState()->onUpdate();
    if (getFadeToGameState()->isEndFadeOut()) {
        getFadeToGameState()->setEndFadeOut(false);
        setGameState(getFadeToGameState()->getGameStateWhenEnds());
    }

    if (getFadeToGameState()->isFinished()) {
        ComponentsManager::get()->getComponentGameInput()->setEnabled(true);
    }
}

void ComponentGame::checkForEndLevel() const
{
    if (getPlayer()->getKillsCounter() >= getLevelInfo()->getNumberLevelEnemies()) {
        getPlayer()->increaseLevelsCompleted();
        getPlayer()->setKillsCounter(0);
        getLevelInfo()->setLevelStartedToPlay(false);
        removeProjectiles();
        getFadeToGameState()->setSpeed(0.01);
        ComponentsManager::get()->getComponentSound()->playSound(
            BUFFERS->soundPackage->getByLabel("levelCompleted"),
            EngineSetup::SoundChannels::SND_GLOBAL,
            0
        );

        if (getPlayer()->getLevelCompletedCounter() >= getLevelInfo()->size()) {
            makeFadeToGameState(EngineSetup::PRESSKEY_GAMEOVER);
        } else {
            makeFadeToGameState(EngineSetup::PRESSKEY_NEWLEVEL);
        }
    }
}

void ComponentGame::setGameState(EngineSetup::GameState state) {
    Logging::getInstance()->Log("GameState changed to: " + std::to_string(state));

    if (state == EngineSetup::GameState::NONE) {
        startWaterShader();
    }

    if (getGameState() == EngineSetup::GameState::NONE && state == EngineSetup::GameState::SPLASH) {

        Logging::getInstance()->Log("GameState changed to SPLASH");

        splashCounter.setEnabled(true);
        ComponentsManager::get()->getComponentSound()->fadeInMusic(BUFFERS->soundPackage->getMusicByLabel("musicMainMenu"), -1, SPLASH_TIME * 1000);
    }

    if (getGameState() == EngineSetup::GameState::SPLASH && state == EngineSetup::GameState::MENU) {
        Logging::getInstance()->Log("GameState changed to MENU");
    }

    if (state == EngineSetup::GameState::MENU) {
        ComponentsManager::get()->getComponentRender()->setEnabled(true);
        ComponentsManager::get()->getComponentMenu()->setEnabled(true);
        setVisibleInGameObjects(false);
        stopSilhouetteShader();
        startWaterShader();
        //player->setEnabled(false);
        player->stopBlinkForPlayer();
        player->setShieldEnabled(false);
    }

    if (state == EngineSetup::GameState::GAMING) {
        setVisibleInGameObjects(true);
        player->setEnabled(true);
        player->startPlayerBlink();
        ComponentsManager::get()->getComponentHUD()->setEnabled(true);
        ComponentsManager::get()->getComponentMenu()->setEnabled(false);
        ComponentsManager::get()->getComponentRender()->setEnabled(true);
        ComponentsManager::get()->getComponentCollisions()->setEnabled(true);
        startBackgroundShader();
        stopTintScreenShader();
        startSilhouetteShader();
        stopWaterShader();
        ComponentsManager::get()->getComponentGame()->getFadeToGameState()->setSpeed(0.03);
    } else {
        stopBackgroundShader();
        ComponentsManager::get()->getComponentHUD()->setEnabled(false);
        ComponentsManager::get()->getComponentCollisions()->setEnabled(false);
    }

    if (state == EngineSetup::GameState::PRESSKEY_NEWLEVEL) {
        getPlayer()->setGravityShieldsNumber(0);
        getPlayer()->setPosition(playerStartPosition);
        ComponentsManager::get()->getComponentGame()->getLevelInfo()->loadNext();
        setVisibleInGameObjects(false);
        ComponentsManager::get()->getComponentHUD()->setEnabled(true);
        ComponentsManager::get()->getComponentMenu()->setEnabled(false);
        ComponentsManager::get()->getComponentRender()->setEnabled(true);
        startBackgroundShader();
        stopWaterShader();
        ComponentsManager::get()->getComponentGame()->getFadeToGameState()->setSpeed(0.01);
        ComponentsManager::get()->getComponentSound()->fadeInMusic(BUFFERS->soundPackage->getMusicByLabel(getLevelInfo()->getMusic()), -1, 3000);
    }

    if (state == EngineSetup::GameState::PRESSKEY_BY_DEAD) {
        ComponentsManager::get()->getComponentHUD()->setEnabled(true);
        ComponentsManager::get()->getComponentMenu()->setEnabled(false);
        ComponentsManager::get()->getComponentRender()->setEnabled(true);
        startBackgroundShader();
        startTintScreenShader();
        stopWaterShader();
    }

    if (state == EngineSetup::GameState::PRESSKEY_GAMEOVER) {
        ComponentsManager::get()->getComponentSound()->playMusic(BUFFERS->soundPackage->getMusicByLabel("gameOverMusic"), -1);
        setVisibleInGameObjects(false);
        removeInGameObjects();
        player->stopBlinkForPlayer();
        player->setShieldEnabled(false);
    }

    this->gameState = state;
}

void ComponentGame::postUpdate() {
    player->evalStatusMachine();
}

void ComponentGame::onEnd() {
}

void ComponentGame::onSDLPollEvent(SDL_Event *event, bool &finish) {
}

Player *ComponentGame::getPlayer() const {
    return player;
}

void ComponentGame::onUpdateIA() const {
    for (auto object : Brakeza3D::get()->getSceneObjects()) {
        auto *enemy = dynamic_cast<EnemyGhost *> (object);

        if (enemy != nullptr) {
            evalStatusMachine(enemy);
        }
    }
}

void ComponentGame::blockPlayerPositionInCamera() {
    auto *camera = ComponentsManager::get()->getComponentCamera()->getCamera();

    Vertex3D homogeneousPosition;
    Vertex3D destinyPoint = player->getPosition() + player->getVelocity();
    Transforms::cameraSpace(homogeneousPosition, destinyPoint, camera);
    homogeneousPosition = Transforms::PerspectiveNDCSpace(homogeneousPosition, camera->frustum);

    if (homogeneousPosition.y > 1) {
        player->setPosition(player->getPosition() + ComponentsManager::get()->getComponentCamera()->getAutoScrollSpeed());
        if (player->getVelocity().y > 0) {
            Vertex3D newVelocity = player->getVelocity();
            newVelocity.y = -1;
            player->setVelocity(newVelocity);
        }
    }

    // top
    if (homogeneousPosition.y < -1) {
        if (player->getVelocity().y < 0) {
            Vertex3D newVelocity = player->getVelocity();
            newVelocity.y = 0;
            player->setVelocity(newVelocity);
        }
    }

    //right
    if (homogeneousPosition.x > 1) {
        if (player->getVelocity().x > 0) {
            Vertex3D newVelocity = player->getVelocity();
            newVelocity.x = -2;
            player->setVelocity(newVelocity);
        }
    }

    //left;
    if (homogeneousPosition.x < -1) {
        if (player->getVelocity().x < 0) {
            Vertex3D newVelocity = player->getVelocity();
            newVelocity.x = 2;
            player->setVelocity(newVelocity);
        }
    }
}

EngineSetup::GameState ComponentGame::getGameState() {
    return gameState;
}

void ComponentGame::loadPlayer()
{
    player->setLabel("player");
    player->setAlpha(200);
    player->setEnableLights(false);
    player->setPosition(playerStartPosition);
    player->setScale(1);
    player->setStamina(100);
    player->setEnableLights(true);
    player->setStencilBufferEnabled(true);
    player->setAutoRotationToFacingSelectedObjectSpeed(5);
    player->AssimpLoadGeometryFromFile(std::string(EngineSetup::get()->MODELS_FOLDER + "spaceships/red_spaceship_02.fbx"));
    player->makeGhostBody(ComponentsManager::get()->getComponentCollisions()->getDynamicsWorld(), player, EngineSetup::collisionGroups::Player, EngineSetup::collisionGroups::AllFilter);
    Brakeza3D::get()->addObject3D(player, "player");

    player->loadShieldModel();
}

Object3D *ComponentGame::getClosesObject3DFromPosition(Vertex3D to, bool skipPlayer, bool skipCurrentSelected)
{
    Object3D *currentClosestObject = nullptr;
    float currentMinDistance = 0;

    for (auto object : Brakeza3D::get()->getSceneObjects()) {
        if (!object->isEnabled() || player == object) {
            continue;
        }

        if (skipPlayer && player == object) {
            continue;
        }

        if (skipCurrentSelected && ComponentsManager::get()->getComponentRender()->getSelectedObject() == object) {
            continue;
        }

        auto mesh = dynamic_cast<EnemyGhost *>(object);
        if (mesh == nullptr) {
            continue;
        }

        mesh->updateBoundingBox();
        for (auto & vertice : mesh->aabb.vertices) {
            Vector3D v(to, vertice);

            const float distance = v.getComponent().getSquaredLength();
            if (currentClosestObject == nullptr) {
                currentMinDistance = distance;
                currentClosestObject = object;
            } else {
                if (distance < currentMinDistance) {
                    currentMinDistance = distance;
                    currentClosestObject = object;
                }
            }
        }
    }

    return currentClosestObject;
}

void ComponentGame::selectClosestObject3DFromPlayer()
{
    auto currentClosestObject = getClosesObject3DFromPosition(player->getPosition(), true, true);

    if (currentClosestObject != nullptr) {
        ComponentsManager::get()->getComponentSound()->playSound(
            BUFFERS->soundPackage->getByLabel("changedSelectedEnemy"),
            EngineSetup::SoundChannels::SND_GLOBAL,
            0
        );
        ComponentsManager::get()->getComponentRender()->setSelectedObject(currentClosestObject);
        ComponentsManager::get()->getComponentRender()->updateSelectedObject3DInShaders(currentClosestObject);
    }
}

void ComponentGame::evalStatusMachine(EnemyGhost *enemy) const
{
    if (enemy->getState() == EnemyState::ENEMY_STATE_DIE) {

        auto fireworks = new ParticleEmissorFireworks(true, 520, 10, 0.01, Color::red(), 6, 50);
        fireworks->setPosition(enemy->getPosition());
        fireworks->setRotationFrame(0, 4, 5);

        Brakeza3D::get()->addObject3D(fireworks, "fireworks" + ComponentsManager::get()->getComponentRender()->getUniqueGameObjectLabel());

        int typePresent = Tools::random(0, 2);
        switch(typePresent) {
            case 0: {
                auto *healthItem = new ItemHealthGhost();
                healthItem->setLabel("item_health");
                healthItem->setEnableLights(true);
                healthItem->setPosition(enemy->getPosition());
                healthItem->setRotationFrameEnabled(true);
                healthItem->setRotationFrame(Vertex3D(0, 1, 0));
                healthItem->setStencilBufferEnabled(true);
                healthItem->setScale(1);
                healthItem->AssimpLoadGeometryFromFile(std::string(EngineSetup::get()->MODELS_FOLDER + "red_pill.fbx"));
                healthItem->makeGhostBody(ComponentsManager::get()->getComponentCollisions()->getDynamicsWorld(), healthItem, EngineSetup::collisionGroups::Health, EngineSetup::collisionGroups::Player);
                Brakeza3D::get()->addObject3D(healthItem, healthItem->getLabel());
                break;
            }
            case 1: {
                auto *healthItem = new ItemEnergyGhost();
                healthItem->setLabel("item_energy");
                healthItem->setEnableLights(true);
                healthItem->setPosition(enemy->getPosition());
                healthItem->setRotationFrameEnabled(true);
                healthItem->setRotationFrame(Vertex3D(0, 1, 0));
                healthItem->setStencilBufferEnabled(true);
                healthItem->setScale(1);
                healthItem->AssimpLoadGeometryFromFile(std::string(EngineSetup::get()->MODELS_FOLDER + "pill.fbx"));
                healthItem->makeGhostBody(ComponentsManager::get()->getComponentCollisions()->getDynamicsWorld(), healthItem, EngineSetup::collisionGroups::Health, EngineSetup::collisionGroups::Player);
                Brakeza3D::get()->addObject3D(healthItem, healthItem->getLabel());
                break;
            }
            case 2: {
                int randomWeapon = Tools::random(0, 2);
                auto *weaponItem = new ItemWeaponGhost(weapons[randomWeapon]);
                weaponItem->setLabel("item_weapon");
                weaponItem->setEnableLights(false);
                weaponItem->setPosition(enemy->getPosition());
                weaponItem->setRotation(0, 0, 180);
                weaponItem->setRotationFrameEnabled(true);
                weaponItem->setRotationFrame(Vertex3D(0, 1, 0));
                weaponItem->setStencilBufferEnabled(true);
                weaponItem->setScale(1);
                weaponItem->copyFrom(weapons[randomWeapon]->getModel());
                weaponItem->makeGhostBody(ComponentsManager::get()->getComponentCollisions()->getDynamicsWorld(), weaponItem, EngineSetup::collisionGroups::Weapon, EngineSetup::collisionGroups::Player);
                Brakeza3D::get()->addObject3D(weaponItem, weaponItem->getLabel());
                break;
            }
        }
    }

    enemy->shoot(ComponentsManager::get()->getComponentGame()->getPlayer());
}

void ComponentGame::loadLevels()
{
    levelInfo = new LevelLoader(EngineSetup::get()->CONFIG_FOLDER + "level01.json");
    levelInfo->addLevel(EngineSetup::get()->CONFIG_FOLDER + "level02.json");
    levelInfo->addLevel(EngineSetup::get()->CONFIG_FOLDER + "level03.json");
}

void ComponentGame::loadBackgroundImageShader()
{
    auto shaderBackground = dynamic_cast<ShaderImageBackground*> (ComponentsManager::get()->getComponentRender()->getShaderByType(EngineSetup::ShaderTypes::BACKGROUND));
    shaderBackground->setEnabled(true);
    shaderBackground->setType(ShaderImageBackgroundTypes::PORTION);
    stopBackgroundShader();
}

void ComponentGame::stopBackgroundShader()
{
    auto shaderBackground = dynamic_cast<ShaderImageBackground*> (ComponentsManager::get()->getComponentRender()->getShaderByType(EngineSetup::ShaderTypes::BACKGROUND));
    shaderBackground->setAutoScrollSpeed(Vertex3D(0, 0, 0));
    shaderBackground->setAutoScrollEnabled(false);
    shaderBackground->setEnabled(false);
}

void ComponentGame::startBackgroundShader()
{
    auto shaderBackground = dynamic_cast<ShaderImageBackground*> (ComponentsManager::get()->getComponentRender()->getShaderByType(EngineSetup::ShaderTypes::BACKGROUND));
    shaderBackground->setAutoScrollSpeed(this->shaderAutoScrollSpeed);
    shaderBackground->setAutoScrollEnabled(true);
    shaderBackground->setEnabled(true);
}

FaderToGameStates *ComponentGame::getFadeToGameState() const {
    return fadeToGameState;
}

void ComponentGame::startTintScreenShader() {
    auto tintShader = dynamic_cast<ShaderTintScreen*> (ComponentsManager::get()->getComponentRender()->getShaderByType(EngineSetup::ShaderTypes::TINT_SCREEN));
    tintShader->setTintColorIntensity(1, 0, 0);
    tintShader->setPhaseRender(EngineSetup::ShadersPhaseRender::POSTUPDATE);
    tintShader->setEnabled(true);
}

void ComponentGame::stopTintScreenShader() {
    auto tintShader = dynamic_cast<ShaderTintScreen*> (ComponentsManager::get()->getComponentRender()->getShaderByType(EngineSetup::ShaderTypes::TINT_SCREEN));
    tintShader->setEnabled(false);
}

void ComponentGame::startSilhouetteShader() {
    auto shader = dynamic_cast<ShaderObjectSilhouette*> (ComponentsManager::get()->getComponentRender()->getShaderByType(EngineSetup::ShaderTypes::SILHOUETTE));
    shader->setColor(Color::red());
    shader->setEnabled(true);
}

void ComponentGame::stopSilhouetteShader() {
    auto shader = dynamic_cast<ShaderObjectSilhouette*> (ComponentsManager::get()->getComponentRender()->getShaderByType(EngineSetup::ShaderTypes::SILHOUETTE));
    shader->setEnabled(false);
}


void ComponentGame::startWaterShader() {
    auto shader = dynamic_cast<ShaderWater*> (ComponentsManager::get()->getComponentRender()->getShaderByType(EngineSetup::ShaderTypes::WATER));
    shader->setPhaseRender(EngineSetup::ShadersPhaseRender::POSTUPDATE);
    //shader->setEnabled(true);
}

void ComponentGame::stopWaterShader() {
    auto shader = dynamic_cast<ShaderWater*> (ComponentsManager::get()->getComponentRender()->getShaderByType(EngineSetup::ShaderTypes::WATER));
    //shader->setEnabled(false);
}

LevelLoader *ComponentGame::getLevelInfo() const {
    return levelInfo;
}

void ComponentGame::removeProjectiles() const {
    auto objects = Brakeza3D::get()->getSceneObjects();
    for (auto object : objects) {
        auto projectile = dynamic_cast<AmmoProjectileBody *> (object);
        if (projectile != nullptr) {
            projectile->remove();
        }
    }
}

void ComponentGame::makeFadeToGameState(EngineSetup::GameState gameState) const
{
    ComponentsManager::get()->getComponentGameInput()->setEnabled(false);

    getFadeToGameState()->resetTo(gameState);
}

void ComponentGame::removeInGameObjects()
{
    for (auto object : Brakeza3D::get()->getSceneObjects()) {
        auto *enemy = dynamic_cast<EnemyGhost *> (object);
        auto *health = dynamic_cast<ItemHealthGhost *> (object);
        auto *weapon = dynamic_cast<ItemWeaponGhost *> (object);
        auto *projectile = dynamic_cast<Projectile3DBody *> (object);

        if (enemy != nullptr) {
            enemy->remove();
        }
        if (health != nullptr) {
            health->remove();
        }
        if (weapon != nullptr) {
            weapon->remove();
        }

        if (projectile != nullptr) {
            projectile->remove();
        }
    }
}

void ComponentGame::setVisibleInGameObjects(bool value)
{
    for (auto object : Brakeza3D::get()->getSceneObjects()) {
        auto *enemy = dynamic_cast<EnemyGhost *> (object);
        auto *health = dynamic_cast<ItemHealthGhost *> (object);
        auto *weapon = dynamic_cast<ItemWeaponGhost *> (object);
        auto *projectile = dynamic_cast<Projectile3DBody *> (object);
        auto *energy = dynamic_cast<ItemEnergyGhost *> (object);
        auto *gravitational = dynamic_cast<GravitationalGhost *> (object);

        if (enemy != nullptr || health != nullptr || weapon != nullptr || projectile != nullptr || energy != nullptr || gravitational != nullptr) {
            object->setEnabled(value);
        }
    }
}

void ComponentGame::loadWeapons() {
    Logging::Log("Loading Weapons for game...", "ComponentGame");

    std::string sndPath = EngineSetup::get()->SOUNDS_FOLDER;
    std::string filePath = EngineSetup::get()->CONFIG_FOLDER + "playerWeapons.json";

    size_t file_size;
    const char *weaponsFile = Tools::readFile(filePath, file_size);
    cJSON *myDataJSON = cJSON_Parse(weaponsFile);

    if (myDataJSON == nullptr) {
        Logging::Log(filePath + " can't be loaded", "ERROR");
        return;
    }

    cJSON *weaponsList = cJSON_GetObjectItemCaseSensitive(myDataJSON, "weapons");
    cJSON *currentWeapon;
    cJSON_ArrayForEach(currentWeapon, weaponsList) {
        auto weapon = getLevelInfo()->parseWeaponJSON(currentWeapon);
        weapon->setSoundChannel(1);
        weapons.push_back(weapon);
    }

    for (auto weapon : weapons) {
        player->addWeapon(weapon);
    }

    player->setWeaponType(weapons[0]);
}
