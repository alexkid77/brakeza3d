#include "../../include/Components/ComponentGame.h"
#include "../../include/Components/ComponentCollisions.h"
#include "../../include/Brakeza3D.h"
#include "../../include/Render/Transforms.h"

#define FREELOOK false

ComponentGame::ComponentGame() {
    player = new Player();
    axisPlanes = new Mesh3DBody();
}

void ComponentGame::onStart() {
    Logging::Log("ComponentGame onStart", "ComponentGame");
    setGameState(GameState::MENU);

    Mix_PlayMusic(BUFFERS->soundPackage->getMusicByLabel("musicMainMenu"), -1);

    ComponentsManager::get()->getComponentCollisions()->initBulletSystem();

    ComponentsManager::get()->getComponentCamera()->getCamera()->setPosition(Vertex3D(0, -1000,0));

    ComponentsManager::get()->getComponentCamera()->setAutoScroll(false);
    ComponentsManager::get()->getComponentCamera()->setAutoScrollSpeed(Vertex3D(0, -1.0, 0));

    ComponentsManager::get()->getComponentCamera()->setFreeLook(FREELOOK);
    ComponentsManager::get()->getComponentInput()->setEnabled(FREELOOK);

    setupWeapons();
    loadPlayer();
}

void ComponentGame::preUpdate() {
}

void ComponentGame::onUpdate() {

    ComponentHUD *componentHUD = ComponentsManager::get()->getComponentHUD();

    if (player->state != PlayerState::GAMEOVER) {
        if (SETUP->ENABLE_IA) {
            onUpdateIA();
        }
    }

    GameState state = getGameState();

    if (state == GameState::LOADING) {
        componentHUD->writeTextMiddleScreen("Loading", false);
    }

    if (state == GameState::MENU) {
    }

    if (state == GameState::GAMING) {
        blockPlayerPositionInCamera();
    }
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
    if (player->isDead()) return;

    std::vector<Object3D *>::iterator itObject3D;
    for (itObject3D = Brakeza3D::get()->getSceneObjects().begin();
        itObject3D != Brakeza3D::get()->getSceneObjects().end(); itObject3D++) {
        Object3D *object = *(itObject3D);

        auto *enemy = dynamic_cast<EnemyGhost *> (object);

        if (enemy != nullptr) {

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

void ComponentGame::setGameState(GameState state) {
    if (state == GameState::GAMING) {
        ComponentsManager::get()->getComponentCamera()->setAutoScroll(true);
    } else {
        ComponentsManager::get()->getComponentCamera()->setAutoScroll(false);
    }

    this->gameState = state;
}

GameState ComponentGame::getGameState() {
    return gameState;
}

void ComponentGame::loadPlayer()
{
    player->setLabel("player");
    player->setEnableLights(false);
    player->setPosition(Vertex3D(1115, -700, 5000));
    player->setScale(1);
    player->setStamina(100);
    player->setStencilBufferEnabled(true);
    player->AssimpLoadGeometryFromFile(std::string(EngineSetup::get()->MODELS_FOLDER + "spaceship03.fbx"));
    player->makeGhostBody(ComponentsManager::get()->getComponentCollisions()->getDynamicsWorld(), player);
    Brakeza3D::get()->addObject3D(player, "player");

    auto *enemyOne = new EnemyGhost();
    enemyOne->setLabel("enemyOne");
    enemyOne->setEnableLights(false);
    enemyOne->setPosition(Vertex3D(1515, -3200, 5000));
    enemyOne->setRotation(0, 0, 180);
    enemyOne->setStencilBufferEnabled(true);
    enemyOne->setScale(1);
    enemyOne->AssimpLoadGeometryFromFile(std::string(EngineSetup::get()->MODELS_FOLDER + "spaceship03.fbx"));
    enemyOne->makeGhostBody(ComponentsManager::get()->getComponentCollisions()->getDynamicsWorld(), enemyOne);
    Brakeza3D::get()->addObject3D(enemyOne, "enemyOne");

    auto *sprite = new Sprite3D();
    sprite->setEnabled(true);
    sprite->setPosition(Vertex3D(0, -1000, 200));
    sprite->setScale(500);
    sprite->addAnimation(EngineSetup::get()->SPRITES_FOLDER + "blood1/blood", 3, 1);
    sprite->setAnimation(0);
    Brakeza3D::get()->addObject3D(sprite, "sprite");
}

void ComponentGame::setupWeapons() {
    auto *cw = ComponentsManager::get()->getComponentWeapons();
    cw->addWeaponType("defaultWeapon");
    WeaponType *weaponType = cw->getWeaponTypeByLabel("defaultWeapon");
    weaponType->setSpeed(500);
    weaponType->setDamage(10);
    weaponType->setDispersion(10);
    weaponType->setAvailable(true);
    weaponType->setAccuracy(100);

    auto *ammoType = new AmmoType();
    ammoType->setName("defaultWeapon_ammoType");
    ammoType->getModelProjectile()->AssimpLoadGeometryFromFile(std::string(EngineSetup::get()->MODELS_FOLDER + "basic/icosphere.fbx"));
    ammoType->getModelProjectile()->setLabel("projectile_template");
    ammoType->getModelProjectile()->setScale(1);
    ammoType->setAmount(1000);
    weaponType->setAmmoType(ammoType);

    cw->setCurrentWeaponIndex(WeaponsTypes::DEFAULT);
}


void ComponentGame::selectClosestObject3DFromPlayer() {
    Object3D *currentClosestObject = nullptr;
    Object3D *currentSelectedObject = ComponentsManager::get()->getComponentRender()->getSelectedObject();

    float currentMinDistance = 0;

    for (auto object : Brakeza3D::get()->getSceneObjects()) {
        if (!object->isEnabled())  {
            continue;
        }

        if (player == object) {
            continue;
        }

        if (currentSelectedObject == object) {
            continue;
        }

        Mesh3D *mesh = dynamic_cast<Mesh3D *> (object);
        if (mesh == nullptr) {
            continue;
        }

        mesh->updateBoundingBox();

        for (auto & vertice : mesh->aabb.vertices) {
            Vector3D v(player->getPosition(), vertice);

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

    if (currentClosestObject != nullptr) {
        auto *shader = dynamic_cast<ShaderObjectSilhouette *>(ComponentsManager::get()->getComponentRender()->getShaderByType(EngineSetup::SILHOUETTE));
        shader->setObject(currentClosestObject);
        ComponentsManager::get()->getComponentRender()->setSelectedObject(currentClosestObject);
    }
}