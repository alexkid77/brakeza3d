
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
#include "../Game/ShaderFadeBetweenGameStates.h"
#include "../../src/Game/LevelsLoader.h"

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

    ShaderFadeBetweenGameStates *fadeToGameState;
    Player *player;
    Mesh3DBody *axisPlanes;
    PathFinder *pathFinder;

    Image *imageHelp;
    Image *imageSplash;
    Counter splashCounter;

    LevelsLoader *levelInfo;
    Vertex3D shaderAutoScrollSpeed;

    void setGameState(EngineSetup::GameState state);
    EngineSetup::GameState getGameState();
    void selectClosestObject3DFromPlayer();

    ShaderFadeBetweenGameStates *getFadeToGameState() const;

    int Z_COORDINATE_GAMEPLAY = 10000;
private:

    EngineSetup::GameState gameState;

    void loadPlayerWeapons();

    void evalStatusMachine(EnemyGhost *pGhost) const;

    void loadLevels();

    void loadBackgroundImageShader();

    void stopBackgroundShader();
    void startBackgroundShader();
    void startWaterShader();
    void stopWaterShader();

public:
    LevelsLoader *getLevelInfo() const;

    void checkForEndLevel() const;
};


#endif //BRAKEDA3D_COMPONENTGAME_H
