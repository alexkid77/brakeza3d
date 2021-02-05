
#include "../../src/Game/Player.h"
#include "../Brakeza3D.h"

Player::Player() : defaultLives(5), oxygen(100), state(PlayerState::GAMEOVER), dead(false), stamina(EngineSetup::getInstance()->GAME_PLAYER_STAMINA_INITIAL), lives(defaultLives), tookDamage(false), stooped(false)
{
    this->counterStep       = new Counter(0.30);
    this->counterSoundTakeDamage = new Counter(0.30);
}

int Player::getStamina() const {
    return stamina;
}

void Player::setStamina(int stamina) {
    Player::stamina = stamina;
}

int Player::getLives() const {
    return lives;
}

void Player::setLives(int lives) {
    Player::lives = lives;
}

bool Player::isDead() const {
    return dead;
}

void Player::setDead(bool dead)
{
    if (this->dead != dead && dead) {
        ComponentsManager::get()->getComponentInput()->setEnabled( false);
        int rndPlayerDead = Tools::random(1, 6);
        Tools::playMixedSound( EngineBuffers::getInstance()->soundPackage->getSoundByLabel("playerDead" + std::to_string(rndPlayerDead)), EngineSetup::SoundChannels::SND_PLAYER, 0);
    }

    ComponentsManager::get()->getComponentHUD()->setStatusFaceAnimation(ComponentHUD::StatusFace::DEAD);

    this->dead = dead;
}

void Player::evalStatusMachine()
{

    this->counterStep->update();
    this->counterSoundTakeDamage->update();

    switch (state) {
        case PlayerState::LIVE:
            break;
        case PlayerState::DEAD:
            break;
    }
}

void Player::takeDamage(float dmg)
{
    if (dead) return;

    this->stamina -= dmg;
    this->tookDamage = true;

    ComponentsManager::get()->getComponentHUD()->setStatusFaceAnimation(ComponentHUD::StatusFace::OUCH);

    if (counterSoundTakeDamage->isFinished()) {
        counterSoundTakeDamage->setEnabled(true);
        int rndPlayerPain = Tools::random(1, 4);
        Tools::playMixedSound( EngineBuffers::getInstance()->soundPackage->getSoundByLabel("playerPain" + std::to_string(rndPlayerPain)), EngineSetup::SoundChannels::SND_PLAYER, 0);
    }

    if (stamina <= 0) {
        state = PlayerState::DEAD;
        setDead( true );
        lives--;

        if (lives <= 0) {
            state = PlayerState::GAMEOVER;
            EngineSetup::getInstance()->MENU_ACTIVE = true;
        }
    }
}

void Player::newGame()
{
    ComponentsManager::get()->getComponentInput()->setEnabled( true );
    setLives(defaultLives);
    //SDL_SetRelativeMouseMode(SDL_TRUE);
    EngineSetup::getInstance()->MENU_ACTIVE = false;
    EngineSetup::getInstance()->DRAW_WEAPON = true;
    EngineSetup::getInstance()->DRAW_HUD = true;

    this->state = PlayerState::LIVE;
}

void Player::respawn()
{
    ComponentsManager::get()->getComponentBSP()->setCameraInBSPStartPosition();
    if ( Tools::isValidVector( Brakeza3D::get()->getComponentsManager()->getComponentCamera()->getCamera()->getPosition() )) {
        Logging::getInstance()->Log("error position start");
    }

    ComponentsManager::get()->getComponentInput()->setEnabled( true );
    ComponentsManager::get()->getComponentHUD()->setStatusFaceAnimation(ComponentHUD::StatusFace::STAND);
    setDead(false);
    state = PlayerState::LIVE;
    setStamina(100);
    oxygen = 100;
    Tools::playMixedSound( EngineBuffers::getInstance()->soundPackage->getSoundByLabel("startGame"), EngineSetup::SoundChannels::SND_ENVIRONMENT, 0);
}

void Player::shoot()
{
    Logging::getInstance()->Log("Player shoot!");
    Brakeza3D::get()->getComponentsManager()->getComponentWeapons()->shoot();
}

void Player::jump()
{
    if ( !Brakeza3D::get()->getComponentsManager()->getComponentBSP()->getBSPCollider()->isPlayerOnGround() ) {
        return;
    }

    // sound
    int rndJump = Tools::random(1, 4);
    Tools::playMixedSound( EngineBuffers::getInstance()->soundPackage->getSoundByLabel("playerJump" + std::to_string(rndJump)), EngineSetup::SoundChannels::SND_PLAYER, 0);

    // apply force
    Vertex3D jump(0, -150, 0);
    Vertex3D current = Brakeza3D::get()->getComponentsManager()->getComponentCamera()->getCamera()->velocity.vertex2;
    Brakeza3D::get()->getComponentsManager()->getComponentCamera()->getCamera()->velocity.vertex2 = current + jump;
}

void Player::reload()
{
    Logging::getInstance()->Log("Player reload!");
    Brakeza3D::get()->getComponentsManager()->getComponentWeapons()->reload();
}

void Player::respawnNPCS()
{
    std::vector<Object3D *>::iterator it;
    for (it = Brakeza3D::get()->getSceneObjects().begin(); it != Brakeza3D::get()->getSceneObjects().end(); it++) {
        Object3D *object = *(it);
        auto *enemy = dynamic_cast<NPCEnemyBody *> (object);

        if (enemy != NULL) {
            enemy->respawn();
        }
    }
}

bool Player::isCrouch() const {
    return stooped;
}

void Player::setCrouch(bool stooped) {
    Player::stooped = stooped;
}

float Player::getOxygen() const {
    return oxygen;
}

void Player::setOxygen(float air)
{
    if (air < 0) {
        this->takeDamage(this->stamina);
        return;
    }
    Player::oxygen = air;
}

void Player::getAid(float aid)
{
    this->stamina = stamina + aid;

    if (stamina > EngineSetup::getInstance()->GAME_PLAYER_STAMINA_INITIAL) {
        this->stamina = EngineSetup::getInstance()->GAME_PLAYER_STAMINA_INITIAL;
    }
}
