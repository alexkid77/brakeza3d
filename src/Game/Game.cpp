#include "../../headers/Game/Game.h"
#include "../../headers/Render/Maths.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "../../headers/Physics/Sprite3DBody.h"
#include "../../headers/Brakeza3D.h"
#include "../../headers/Render/EngineBuffers.h"
#include "../../headers/Collisions/CollisionResolverBetweenProjectileAndBSPMap.h"
#include "../../headers/Collisions/CollisionResolverBetweenProjectileAndNPCEnemy.h"
#include "../../headers/Collisions/CollisionResolverBetweenCamera3DAndFuncDoor.h"
#include "../../headers/Collisions/CollisionResolverBetweenCamera3DAndFuncButton.h"
#include "../../headers/Collisions/CollisionResolverBetweenEnemyPartAndBSPMap.h"
#include "../../headers/Collisions/CollisionResolverBetweenProjectileAndPlayer.h"
#include "../../headers/Render/Drawable.h"

Game* Game::instance = 0;

Game::Game()
{
    player     = new Player();
    controller = new GameInputController( player );

    Brakeza3D::get()->setController( controller );

    Texture *t1 = new Texture();
    t1->loadTGA(std::string(EngineSetup::getInstance()->HUD_FOLDER + "hud_health.png").c_str(), 1);
    Texture *t2 = new Texture();
    t2->loadTGA(std::string(EngineSetup::getInstance()->HUD_FOLDER + "hud_ammo.png").c_str(), 1);
    Texture *t3 = new Texture();
    t3->loadTGA(std::string(EngineSetup::getInstance()->HUD_FOLDER + "hud_lives.png").c_str(), 1);

    HUDTextures.push_back(t1);
    HUDTextures.push_back(t2);
    HUDTextures.push_back(t3);
}

Game* Game::get()
{
    if (instance == 0) {
        instance = new Game();
    }

    return instance;
}

void Game::start()
{
    // Start Engine cycle
    onStart();
    mainLoop();
    onEnd();
}

void Game::onStart()
{
    Engine::onStart();
}

void Game::mainLoop()
{
    ImGuiIO& io = ImGui::GetIO();
    while(!finish) {

        this->preUpdate();

        this->onUpdateInputController();

        if (!finish) {
            // game level update
            this->onUpdate();


            // Update window
            Engine::updateWindow();
        }
    }
}

void Game::onUpdate()
{
    // Core onUpdate
    Engine::onUpdate();

    onUpdateIA();

    drawHUD();

    if (EngineSetup::getInstance()->MENU_ACTIVE) {
        drawMenuScreen();
    }
}

void Game::preUpdate()
{
    // Core preUpdate
    Engine::preUpdate();
}

void Game::onEnd()
{
    Engine::onEnd();
    Close();
}

void Game::onUpdateInputController()
{
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);

        Brakeza3D::get()->getController()->updateKeyboardMapping();
        Brakeza3D::get()->getController()->updateMouseStates(&this->e);

        if (EngineSetup::getInstance()->CAMERA_MOUSE_ROTATION) {
            Brakeza3D::get()->getController()->handleMouse(&this->e);
        }
        Brakeza3D::get()->getController()->handleKeyboard(&this->e, this->finish);
    }

    // Check array Uint8 *keyboard
    Brakeza3D::get()->getController()->handleKeyboardContinuous(&this->e, this->finish);
}

void Game::drawHUD()
{
    if (EngineSetup::getInstance()->SHOW_WEAPON) {
        Brakeza3D::get()->getWeaponsManager()->onUpdate(Brakeza3D::get()->getCamera(), Brakeza3D::get()->screenSurface );

        SDL_Rect r1, r2, r3;

        r1.x = 10; r1.y = 225;
        SDL_BlitSurface(this->HUDTextures[0]->getSurface(1), NULL, Brakeza3D::get()->screenSurface, &r1);
        Tools::writeText(Brakeza3D::get()->renderer, Brakeza3D::get()->font, 30, 223, Color::green(), std::to_string(this->player->getStamina()));

        r2.x = 60; r2.y = 225;
        SDL_BlitSurface(this->HUDTextures[1]->getSurface(1), NULL, Brakeza3D::get()->screenSurface, &r2);
        Tools::writeText(Brakeza3D::get()->renderer, Brakeza3D::get()->font, 80, 223, Color::green(), std::to_string(Brakeza3D::get()->getWeaponsManager()->getCurrentWeaponType()->ammo));

        r3.x = 7; r3.y = 7;
        for (int i = 0; i < this->player->getLives(); i++) {
            SDL_BlitSurface(this->HUDTextures[2]->getSurface(1), NULL, Brakeza3D::get()->screenSurface, &r3);
            r3.x+=10;
        }
    }

    if (EngineSetup::getInstance()->DRAW_CROSSHAIR) {
        Drawable::drawCrossHair();
    }

    if (player->isDead()) {
        redScreen();
    }
}

void Game::onUpdateIA()
{
    if (player->isDead()) return;

    std::vector<Object3D *>::iterator itObject3D;
    for ( itObject3D = Brakeza3D::get()->getSceneObjects().begin(); itObject3D != Brakeza3D::get()->getSceneObjects().end(); itObject3D++) {
        Object3D *object = *(itObject3D);

        if (!Brakeza3D::get()->getCamera()->frustum->isPointInFrustum(*object->getPosition())) {
            continue;
        }

        NPCEnemyBody *enemy = dynamic_cast<NPCEnemyBody*> (object);
        if (enemy != NULL) {
            if (enemy->isDead()) continue;

            Vertex3D A = *object->getPosition();
            Vertex3D B = *Brakeza3D::get()->getCamera()->getPosition();

            enemy->points.clear();
            Brakeza3D::get()->getBSP()->recastWrapper->getPathBetween(A, B, enemy->points );
            enemy->evalStatusMachine(
                    Brakeza3D::get()->getBSP()->recastWrapper->rayCasting(A, B),
                    Vector3D(A, B).getComponent().getModule(),
                    Brakeza3D::get()->getCamera(),
                    Brakeza3D::get()->getCollisionManager()->getDynamicsWorld(),
                    Brakeza3D::get()->getSceneObjects()
            );
        }
    }
}

void Game::resolveCollisions()
{
    CollisionsManager *cm = Brakeza3D::get()->getCollisionManager();

    std::vector<CollisionResolver *>::iterator itCollision;
    for ( itCollision = cm->getCollisions().begin(); itCollision != cm->getCollisions().end(); itCollision++) {
        CollisionResolver *collision = *(itCollision);
        int collisionType = collision->getTypeCollision();

        if (!collisionType) continue;

        if ( collisionType == EngineSetup::getInstance()->CollisionResolverTypes::COLLISION_RESOLVER_PROJECTILE_AND_BSPMAP ) {
            auto *resolver = new CollisionResolverBetweenProjectileAndBSPMap(
                    collision->contactManifold,
                    collision->objA,
                    collision->objB,
                    cm->getBspMap(),
                    cm->getGameObjects(),
                    cm->getDynamicsWorld(),
                    cm->getWeaponManager(),
                    cm->getVisibleTriangles()
            );
            resolver->dispatch();
            continue;
        }

        if ( collisionType == EngineSetup::getInstance()->CollisionResolverTypes::COLLISION_RESOLVER_PROJECTILE_AND_NPCENEMY ) {
            auto *resolver = new CollisionResolverBetweenProjectileAndNPCEnemy(
                    collision->contactManifold,
                    collision->objA,
                    collision->objB,
                    cm->getBspMap(),
                    cm->getGameObjects(),
                    cm->getDynamicsWorld(),
                    cm->getWeaponManager(),
                    cm->getVisibleTriangles()
            );
            resolver->dispatch();
            continue;
        }

        if ( collisionType == EngineSetup::getInstance()->CollisionResolverTypes::COLLISION_RESOLVER_CAMERA_AND_FUNCDOOR ) {
            auto *resolver = new CollisionResolverBetweenCamera3DAndFuncDoor(
                    collision->contactManifold,
                    collision->objA,
                    collision->objB,
                    cm->getBspMap(),
                    cm->getGameObjects(),
                    cm->getVisibleTriangles()
            );
            resolver->dispatch();
            continue;
        }

        if ( collisionType == EngineSetup::getInstance()->CollisionResolverTypes::COLLISION_RESOLVER_CAMERA_AND_FUNCBUTTON ) {
            auto *resolver = new CollisionResolverBetweenCamera3DAndFuncButton(
                    collision->contactManifold,
                    collision->objA,
                    collision->objB,
                    cm->getBspMap(),
                    cm->getGameObjects(),
                    cm->getVisibleTriangles()
            );
            resolver->dispatch();
            continue;
        }

        if ( collisionType == EngineSetup::getInstance()->CollisionResolverTypes::COLLISION_RESOLVER_NPCENEMYPART_AND_BSPMAP ) {
            auto *resolver = new CollisionResolverBetweenEnemyPartAndBSPMap(
                    collision->contactManifold,
                    collision->objA,
                    collision->objB,
                    cm->getBspMap(),
                    cm->getGameObjects(),
                    cm->getDynamicsWorld(),
                    cm->getWeaponManager(),
                    cm->getVisibleTriangles()
            );
            resolver->dispatch();
            continue;
        }

        if ( collisionType == EngineSetup::getInstance()->CollisionResolverTypes::COLLISION_RESOLVER_PROJECTILE_AND_CAMERA ) {
            auto *resolver = new CollisionResolverBetweenProjectileAndPlayer(
                    collision->contactManifold,
                    collision->objA,
                    collision->objB,
                    cm->getBspMap(),
                    cm->getGameObjects(),
                    cm->getDynamicsWorld(),
                    cm->getWeaponManager(),
                    cm->getVisibleTriangles(),
                    player
            );
            resolver->dispatch();
            continue;
        }
    }

    cm->getCollisions().clear();
}

void Game::redScreen()
{
    float intensity_r = 1;
    float intensity_g = 0.5;
    float intensity_b = 0.5;

    for (int y = 0; y < EngineSetup::getInstance()->screenHeight; y++) {
        for (int x = 0; x < EngineSetup::getInstance()->screenWidth; x++) {
            Uint32 currentPixelColor = EngineBuffers::getInstance()->getVideoBuffer(x, y);

            int r_light = (int) (Tools::getRedValueFromColor(currentPixelColor) * intensity_r);
            int g_light = (int) (Tools::getGreenValueFromColor(currentPixelColor) * intensity_g);
            int b_light = (int) (Tools::getBlueValueFromColor(currentPixelColor) * intensity_b);

            currentPixelColor = Tools::createRGB(r_light, g_light, b_light);
            EngineBuffers::getInstance()->setVideoBuffer( x, y, currentPixelColor );
        }
    }
}

void Game::drawMenuScreen()
{
    //this->waterShader();
    Brakeza3D::get()->getMenuManager()->drawOptions(Brakeza3D::get()->screenSurface);
    Drawable::drawFireShader();
}