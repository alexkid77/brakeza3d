
#ifndef BRAKEDA3D_COMPONENTGAME_H
#define BRAKEDA3D_COMPONENTGAME_H


#include "Component.h"
#include "../../src/Game/Player.h"
#include "../Misc/cJSON.h"
#include "../Physics/Mesh3DGhost.h"
#include "../Misc/Octree.h"
#include "../Misc/Grid3D.h"
#include "../Misc/PathFinder.h"
#include "../Shaders/ShaderWater.h"
#include "../Shaders/ShaderFire.h"
#include "../Shaders/ShaderImageBackground.h"
#include "../Shaders/ShaderTintScreen.h"
#include "../Shaders/ShaderObjectSilhouette.h"
#include "../Physics/Mesh3DBody.h"

typedef enum {
    MENU, GAMING, LOADING, HELP
} GameState;

class ComponentGame : public Component {
public:
    ComponentGame();

    void onStart();

    void preUpdate();

    void onUpdate();

    void postUpdate();

    void onEnd();

    void onSDLPollEvent(SDL_Event *event, bool &finish);

    Player *getPlayer() const;

    void onUpdateIA() const;

    void loadPlayer();

    void blockPlayerPositionInCamera();

    Player *player;
    Mesh3DBody *axisPlanes;

    PathFinder *pathFinder;

    Image *imageHelp;

    void setGameState(GameState state);
    GameState getGameState();
    void selectClosestObject3DFromPlayer();
private:

    GameState gameState;

    void loadPlayerWeapons();

    void evalStatusMachine(EnemyGhost *pGhost) const;

    void loadEnemy();
};


#endif //BRAKEDA3D_COMPONENTGAME_H
