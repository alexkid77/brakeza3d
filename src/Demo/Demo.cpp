#include "../../include/Demo/Demo.h"
#include "../../include/Objects/LightPoint3D.h"
#include "../../include/Brakeza3D.h"
#include "../../include/Objects/Decal.h"

Demo::Demo() {
    auto *lp1 = new LightPoint3D();
    lp1->setEnabled(false);
    lp1->setLabel("LightPoint1");
    lp1->setPosition(Vertex3D(1, 1.5f, -1));
    lp1->setColor(255, 0, 0);

    auto *lp2 = new LightPoint3D();
    lp2->setEnabled(false);
    lp2->setLabel("LightPoint2");
    lp2->setPosition(Vertex3D(-0.4, 1, -1));
    lp2->setColor(0, 255, 0);

    auto *lp3 = new LightPoint3D();
    lp3->setEnabled(false);
    lp3->setLabel("LightPoint3");
    lp3->setPosition(Vertex3D(2, 1, -1));
    lp3->setColor(0, 0, 255);



    // marine (sprite directional)
    auto *marine = new SpriteDirectional3D();
    marine->setEnabled(true);
    marine->setPosition(Vertex3D(2, 0, 10));
    marine->setRotation(M3(0, -90, 0));
    marine->addAnimationDirectional2D("enemies/soldier/walk", 4, 20, false, -1);
    marine->addAnimationDirectional2D("enemies/soldier/fire", 2, 20, false, -1);
    marine->addAnimationDirectional2D("enemies/soldier/injuried", 1, 20, false, -1);
    marine->addAnimationDirectional2D("enemies/soldier/dead", 5, 20, true, 1);
    marine->addAnimationDirectional2D("enemies/soldier/explosion", 8, 20, true, 1);
    marine->setAnimation(EngineSetup::get()->SpriteSoldierAnimations::SOLDIER_WALK);
    Brakeza3D::get()->addObject3D(marine, "marine");

    // skull (sprite directional)
    auto *skull = new SpriteDirectional3D();
    skull->setEnabled(false);
    skull->setPosition(Vertex3D(5, 0, -10));
    skull->addAnimationDirectional2D("enemies/skull/idle", 5, 20, false, -1);
    skull->setAnimation(EngineSetup::get()->SpriteSoldierAnimations::SOLDIER_WALK);
    Brakeza3D::get()->addObject3D(skull, "skull");

    // caco (sprite directional)
    auto *caco = new SpriteDirectional3D();
    caco->setEnabled(false);
    caco->setPosition(Vertex3D(20, 0, -10));
    caco->addAnimationDirectional2D("enemies/cacodemon/walk", 6, 20, false, -1);
    caco->addAnimationDirectional2D("enemies/cacodemon/dead", 6, 20, false, -1);
    caco->setAnimation(EngineSetup::get()->SpriteDoom2CacodemonAnimations::FLY);
    Brakeza3D::get()->addObject3D(caco, "caco");


    /*Mesh3DGhost *cuboPhysicGhost = new Mesh3DGhost();
    cuboPhysicGhost->setEnabled(true);
    cuboPhysicGhost->setLightPoints(Engine::lightPoints, Engine::numberLightPoints);
    cuboPhysicGhost->setPosition(Vertex3D(52, -0.2, 87));
    cuboPhysicGhost->loadOBJBlender("../assets/models/cubo.obj");
    cuboPhysicGhost->makeGhostBody(Engine::camera, this->dynamicsWorld, true);
    this->addObject3D(cuboPhysicGhost, "cuboPhysicGhost");*/

    // cube3d
    auto *oCube = new Cube3D(1, 7, 5);
    oCube->setEnabled(false);
    oCube->setPosition(Vertex3D(1, 1, 1));
    Brakeza3D::get()->addObject3D(oCube, "oCube");

    // Lighting example
    /*Vertex3D startPoint = Vertex3D(45, -2, 40);
    Vertex3D endPoint   = Vertex3D(70, -2, 40);
    Drawable::drawLightning(camera, start, endPoint);
     */

    auto *mesh = new Mesh3DAnimated();
    Brakeza3D::get()->addObject3D(mesh, "hellknight");

    if (mesh->AssimpLoadAnimation(EngineSetup::get()->MODELS_FOLDER + "hellknight.md5mesh")) {
        mesh->setScale(0.25);
        Vertex3D p = Vertex3D(1, 1, 20);
        p.y -= -1;
        p.z -= -10;
        M3 r = M3::getMatrixRotationForEulerAngles(90, 1800, 0);
        mesh->setRotation(r);
        mesh->setPosition(p);
    }

    auto *mesh2 = new Mesh3DAnimated();
    Brakeza3D::get()->addObject3D(mesh2, "bob_lamp");
    if (mesh2->AssimpLoadAnimation(EngineSetup::get()->MODELS_FOLDER + "bob/bob_lamp_update.md5mesh")) {
        mesh2->setScale(1);
        Vertex3D p = Vertex3D(1, 1, 1);
        p.y -= -1;
        p.z -= -10;
        M3 r = M3::getMatrixRotationForEulerAngles(90, 0, 0);
        mesh2->setRotation(r);
        mesh2->setPosition(p);
    }

    /*auto *gun = new Mesh3DAnimated();
    Brakeza3D::get()->addObject3D(gun, gun->getLabel());
    gun->setScale(0.05);
    gun->setFollowPointLabel("CameraTracker");
    gun->setFollowCamera(true );
    //gun->setDrawOffset(Vertex3D(0, 0.6, -0.7));
    gun->setPosition(Vertex3D(50, -2, 8));
    gun->setRotation(M3::getMatrixRotationForEulerAngles(90, 0, 0));
    gun->setFixedRotation(M3::getMatrixRotationForEulerAngles(90, 0, 0));
    gun->setLabel("hands");
    gun->AssimpLoad(EngineSetup::get()->MODELS_FOLDER + "glock.Mesh3DBody");
    ComponentsManager::get()->getComponentCamera()->getCamera()->setFollowTo(gun->getFollowMePointObject() );
    */


    // demo object
    auto *sample = new Mesh3D();
    sample->setLabel("mono");
    sample->setPosition(Vertex3D(100, 100, 100));
    sample->setScale(10);
    sample->AssimpLoadGeometryFromFile(std::string(EngineSetup::get()->MODELS_FOLDER + "mono.obj"));
    sample->buildOctree();
    sample->buildGrid3DForEmptyContainsStrategy(10, 10, 10);
    Brakeza3D::get()->addObject3D(sample, "mono");

}
