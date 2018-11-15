#include "Game.h"
#include "headers/Render/Drawable.h"
#include "headers/Render/EngineBuffers.h"
#include "headers/Render/M3.h"
#include "headers/Render/Transforms.h"
#include "headers/Render/Logging.h"
#include "headers/Objects/SpriteDirectional3D.h"
#include "headers/Objects/Sprite3D.h"
#include "headers/Objects/Weapon3D.h"
#include "WAD/WAD.h"
#include "Map.h"

int reductor = 100;

enum SpriteDoom2SoldierAnimations {
    IDLE = 0,
    WALK = 1,
    JUMP = 2,
};

enum SpriteShotgunAnimations {
    READY,
    RELOAD,
    SHOT
};

enum SpriteGuyAnimations {
    NORMAL,
};

void Game::run()
{
    this->onStart();
    this->mainLoop();
    this->onEnd();
    Close();
}


void Game::onStart()
{
    Engine::onStart();

    Engine::cam->head[0] = 544;
    Engine::cam->head[1] = -32;
    Engine::cam->head[2] = 288;

    /*LightPoint3D *lp1 = new LightPoint3D();
    lp1->setEnabled(false);
    lp1->setLabel("LightPoint1");
    lp1->setPosition(Vertex3D(1, 1.5f, -1));
    lp1->setColor( 255, 0, 0 );
    this->addLightPoint(lp1, "l1");

    LightPoint3D *lp2 = new LightPoint3D();
    lp2->setEnabled(false);
    lp2->setLabel("LightPoint2");
    lp2->setPosition(Vertex3D(-0.4, 1, -1));
    lp2->setColor( 0, 255, 0 );
    this->addLightPoint(lp2, "l2");

    LightPoint3D *lp3 = new LightPoint3D();
    lp3->setEnabled(false);
    lp3->setLabel("LightPoint3");
    lp3->setPosition(Vertex3D(2, 1, -1));
    lp3->setColor( 0, 0, 255 );
    this->addLightPoint(lp3, "l3");
    */

    // mono
    Mesh3D *mono = new Mesh3D();
    mono->setEnabled(true);
    mono->setLightPoints(Engine::lightPoints, Engine::numberLightPoints);
    //mono->rotation.x = 180;
    mono->getPosition()->x = 544;
    mono->getPosition()->y = 288;
    mono->getPosition()->z = 38;

    mono->loadOBJBlender("../models/mono.obj");
    mono->setShadowCaster(true);
    this->addObject3D(mono, "mono");

    // cubo
    Mesh3D *cubo = new Mesh3D();
    cubo->scale = 500;
    cubo->setEnabled(true);
    cubo->setLightPoints(Engine::lightPoints, Engine::numberLightPoints);
    cubo->getRotation()->x = 180;
    cubo->setPosition( Vertex3D(1, 1, 20) );
    cubo->loadOBJBlender("../models/cubo.obj");
    this->addObject3D(cubo, "cubo");

    // q3 q1map
    /*Mesh3D *q3map = new Mesh3D();
    q3map->setEnabled(false);
    q3map->setLightPoints(Engine::lightPoints, Engine::numberLightPoints);
    q3map->setPosition( Vertex3D(1, 1, 5) );
    q3map->loadQ3Map("../pak0/maps/q3dm17.bsp");
    this->addObject3D(q3map, "q3map");
    */

    // triangle
    Mesh3D *triangle = new Mesh3D();
    triangle->setEnabled(false);
    triangle->getRotation()->x-=90;
    triangle->setLightPoints(Engine::lightPoints, Engine::numberLightPoints);
    triangle->setPosition( Vertex3D(1, 1, 5) );
    triangle->loadOBJBlender("../models/triangle_2uv.obj");
    this->addObject3D(triangle, "triangle");

    // marine (sprite directional)
    SpriteDirectional3D *marine = new SpriteDirectional3D();
    marine->setEnabled(true);
    marine->setPosition( Vertex3D(1, 1, 5) );
    marine->setTimer(Engine::getTimer());
    marine->addAnimationDirectional2D("marine/idle", 1);
    marine->addAnimationDirectional2D("marine/walk", 4);
    marine->addAnimationDirectional2D("marine/jump", 1);
    marine->setAnimation(SpriteDoom2SoldierAnimations::WALK);
    this->addObject3D(marine, "marine");

    // gun ( sprite )
    Sprite3D *guy = new Sprite3D();
    guy->setEnabled(true);
    guy->setPosition( Vertex3D(2, 1, 5) );
    guy->setTimer(Engine::getTimer());
    guy->addAnimation("guy/face", 3);
    guy->setAnimation(SpriteGuyAnimations::NORMAL);
    this->addObject3D(guy, "guy");

    // weapon
    /*Weapon3D *weapon = new Weapon3D();
    weapon->setEnabled(false);
    weapon->setPosition( Vertex3D(1, 1, 5) );
    weapon->setTimer(Engine::getTimer());
    weapon->addAnimation("gun/ready", 1);
    weapon->addAnimation("gun/reload", 3);
    weapon->addAnimation("gun/shot", 2);
    weapon->setAnimation(SpriteShotgunAnimations::RELOAD);
    this->addObject3D(weapon, "weapon");
    */

    // WAD Parser
    /*
    char* wadLocation = "../models/freedoom1.wad";
    WAD* testWad = new WAD(wadLocation);
    testWad->loadMap("E1M1");
    testWad->draw2D();
    testWad->render();
    */

    Map *q1map = new Map();
    q1map->setPosition( Vertex3D(0, 0, 0) );
    q1map->setRotation( Rotation3D(180, 90, 0) );

    if (!q1map->Initialize("../assets/start.bsp", "../assets/palette.lmp")) {
        Logging::getInstance()->Log("Quake::main() Unable to initialize q1map", "QUAKE");
    }
    q1map->InitializeSurfaces();
    q1map->InitializeTextures();
    this->addObject3D(q1map, "q1map");
}

void Game::mainLoop()
{
    fpsTimer.start();

    ImGuiIO& io = ImGui::GetIO();

    while(!finish) {
        while (SDL_PollEvent(&e)) {
            // GUI Events
            ImGui_ImplSDL2_ProcessEvent(&e);

            // Keyboard Reading
            Engine::getController()->updateKeyboardRead(&e);

            // Camera Update (Mouse & Keyboard)
            Engine::cameraUpdate();

            this->onUpdateEvent();
        }

        this->onUpdate();

        Engine::windowUpdate();
        Engine::processFPS();
    }

    fpsTimer.stop();
}

void Game::onUpdateEvent()
{
    Engine::onUpdateEvent();
}

void Game::onUpdate()
{
    Engine::onUpdate();

    //Mesh3D *marine= (Mesh3D*) getObjectByLabel("marine");
    //marine->rotation.y+=0.5f;

    Map *q1map = (Map*) getObjectByLabel("q1map");

    bspleaf_t *leaf = q1map->FindLeaf(Engine::cam);
    q1map->DrawLeafVisibleSet( leaf, Engine::cam);
    //q1map->drawTriangles(Engine::cam);


}

void Game::onEnd()
{
    Engine::onEnd();
}
