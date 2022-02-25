#include "../../include/Components/ComponentCollisions.h"
#include "../../include/Brakeza3D.h"

ComponentCollisions::ComponentCollisions() = default;

void ComponentCollisions::onStart() {
    Logging::Log("ComponentCollisions onStart", "ComponentCollisions");
}

void ComponentCollisions::preUpdate() {
}

void ComponentCollisions::onUpdate() {
    this->stepSimulation();
}

void ComponentCollisions::postUpdate() {
}

void ComponentCollisions::onEnd() {

}

void ComponentCollisions::onSDLPollEvent(SDL_Event *e, bool &finish) {
}

void ComponentCollisions::initBulletSystem() {
    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    this->collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    this->dispatcher = new btCollisionDispatcher(collisionConfiguration);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    this->overlappingPairCache = new btDbvtBroadphase();

    btVector3 worldMin(-50000, -50000, -50000);
    btVector3 worldMax(50000, 50000, 50000);

    auto *sweepBP = new btAxisSweep3(worldMin, worldMax);
    this->overlappingPairCache = sweepBP;

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    this->solver = new btSequentialImpulseConstraintSolver;

    /// Debug drawer
    this->debugDraw = new PhysicsDebugDraw(ComponentsManager::get()->getComponentCamera()->getCamera());

    this->dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    this->dynamicsWorld->setGravity(btVector3(0, -EngineSetup::get()->gravity.y, 0));

    this->dynamicsWorld->setDebugDrawer(debugDraw);
    this->dynamicsWorld->getDebugDrawer()->setDebugMode(PhysicsDebugDraw::DBG_DrawWireframe);

    this->overlappingPairCache->getOverlappingPairCache()->setInternalGhostPairCallback( new btGhostPairCallback() );

    /*this->dynamicsWorld->addCollisionObject(
        this->camera->m_ghostObject,
        btBroadphaseProxy::CharacterFilter,
        btBroadphaseProxy::StaticFilter
    );*/

    //this->makeGhostForCamera();
}

btDiscreteDynamicsWorld *ComponentCollisions::getDynamicsWorld() const {
    return dynamicsWorld;
}

void ComponentCollisions::setDynamicsWorld(btDiscreteDynamicsWorld *world) {
    ComponentCollisions::dynamicsWorld = world;
}

BSPMap *ComponentCollisions::getBspMap() const {
    return bspMap;
}

void ComponentCollisions::setBSPMap(BSPMap *map) {
    ComponentCollisions::bspMap = map;
}

void ComponentCollisions::checkCollisionsForAll() {
    if (!SETUP->BULLET_CHECK_ALL_PAIRS) return;

    int numManifolds = this->dynamicsWorld->getDispatcher()->getNumManifolds();

    for (int i = 0; i < numManifolds; i++) {

        btPersistentManifold *contactManifold = this->dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);

        if (contactManifold->getNumContacts() > 0) {
            const btCollisionObject *obA = contactManifold->getBody0();
            const btCollisionObject *obB = contactManifold->getBody1();

            auto *brkObjectA = (Collisionable *) obA->getUserPointer();
            auto *brkObjectB = (Collisionable *) obB->getUserPointer();

            brkObjectA->resolveCollision(brkObjectB);
            brkObjectB->resolveCollision(brkObjectA);
        }
    }
}

void ComponentCollisions::updatePhysicObjects() {
    std::vector<Object3D *>::iterator it;
    for (it = getSceneObjects()->begin(); it != getSceneObjects()->end(); it++) {
        auto *collisionable = dynamic_cast<Collisionable *> (*it);
        if (collisionable != nullptr) {
            if (!collisionable->isCollisionsEnabled()) continue;
            collisionable->integrate();
        }
    }
}

void ComponentCollisions::stepSimulation() {
    checkCollisionsForAll();

    if (SETUP->BULLET_STEP_SIMULATION) {

        getDynamicsWorld()->stepSimulation(Brakeza3D::get()->getDeltaTime());

        this->updatePhysicObjects();

    }
}

std::vector<Triangle *> &ComponentCollisions::getVisibleTriangles() const {
    return *visibleTriangles;
}

void ComponentCollisions::setVisibleTriangles(std::vector<Triangle *> &newVisibleTriangles) {
    ComponentCollisions::visibleTriangles = &newVisibleTriangles;
}

void ComponentCollisions::demoProjectile() {
    auto *projectile = new Projectile3DBody();
    Camera3D *camera = ComponentsManager::get()->getComponentCamera()->getCamera();
    Vertex3D direction = camera->getRotation().getTranspose() * EngineSetup::get()->forward;
    projectile->AssimpLoadGeometryFromFile(std::string(EngineSetup::get()->MODELS_FOLDER + "planet_cube.fbx"));
    projectile->setRotation(Tools::random(0, 180), Tools::random(0, 180),Tools::random(0, 180));
    projectile->setFlatTextureColor(true);
    projectile->setPosition( camera->getPosition());
    projectile->setLabel("projectile_" + ComponentsManager::get()->getComponentRender()->getUniqueGameObjectLabel());
    projectile->setEnabled(true);
    projectile->setTTL(EngineSetup::get()->PROJECTILE_DEMO_TTL);
    projectile->makeProjectileRigidBody(
        EngineSetup::get()->PROJECTILE_DEMO_MASS,
        direction,
        EngineSetup::get()->PROJECTILE_DEMO_IMPULSE,
        EngineSetup::get()->PROJECTILE_DEMO_ACCURACY,
        ComponentsManager::get()->getComponentCollisions()->getDynamicsWorld()
    );
    Brakeza3D::get()->addObject3D(projectile, projectile->getLabel());
}

