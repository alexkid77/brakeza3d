
#include "../../src/Game/Player.h"
#include "../Brakeza3D.h"

Player::Player() : defaultLives(5), state(PlayerState::GAMEOVER), dead(false),
                   stamina(EngineSetup::getInstance()->GAME_PLAYER_STAMINA_INITIAL), lives(defaultLives),
                   stooped(false) {
    this->counterStep = new Counter(0.30);
    this->counterSoundTakeDamage = new Counter(0.30);
}

int Player::getStamina() const {
    return stamina;
}

void Player::setStamina(int stamina) {
    Player::stamina = stamina;
}

void Player::setLives(int lives) {
    Player::lives = lives;
}

bool Player::isDead() const {
    return dead;
}

void Player::setDead(bool dead) {
    if (this->dead != dead && dead) {
        ComponentsManager::get()->getComponentInput()->setEnabled(false);
        int rndPlayerDead = Tools::random(1, 6);
        Tools::playMixedSound(EngineBuffers::getInstance()->soundPackage->getSoundByLabel(
                "playerDead" + std::to_string(rndPlayerDead)), EngineSetup::SoundChannels::SND_PLAYER, 0);
    }

    this->dead = dead;
}

void Player::evalStatusMachine() {

    this->counterStep->update();
    this->counterSoundTakeDamage->update();

    switch (state) {
        case PlayerState::LIVE:
            break;
        case PlayerState::DEAD:
            break;
    }
}

void Player::takeDamage(float dmg) {
    if (dead) return;

    this->stamina -= dmg;

    if (counterSoundTakeDamage->isFinished()) {
        counterSoundTakeDamage->setEnabled(true);
        int rndPlayerPain = Tools::random(1, 4);
        Tools::playMixedSound(EngineBuffers::getInstance()->soundPackage->getSoundByLabel(
                "playerPain" + std::to_string(rndPlayerPain)), EngineSetup::SoundChannels::SND_PLAYER, 0);
    }

    if (stamina <= 0) {
        state = PlayerState::DEAD;
        setDead(true);
        lives--;

        if (lives <= 0) {
            state = PlayerState::GAMEOVER;
            EngineSetup::getInstance()->MENU_ACTIVE = true;
        }
    }
}

void Player::newGame() {
    ComponentsManager::get()->getComponentInput()->setEnabled(true);
    setLives(defaultLives);
    //SDL_SetRelativeMouseMode(SDL_TRUE);
    EngineSetup::getInstance()->MENU_ACTIVE = false;
    EngineSetup::getInstance()->DRAW_HUD = true;

    this->state = PlayerState::LIVE;
}

void Player::respawn() {
    if (Tools::isValidVector(
            Brakeza3D::get()->getComponentsManager()->getComponentCamera()->getCamera()->getPosition())) {
    }

    ComponentsManager::get()->getComponentInput()->setEnabled(true);
    setDead(false);
    state = PlayerState::LIVE;
    setStamina(100);
    Tools::playMixedSound(EngineBuffers::getInstance()->soundPackage->getSoundByLabel("startGame"),
                          EngineSetup::SoundChannels::SND_ENVIRONMENT, 0);
}

void Player::shoot() {
    Brakeza3D::get()->getComponentsManager()->getComponentWeapons()->shoot();
}

void Player::jump() {
    // sound
    Tools::playMixedSound(EngineBuffers::getInstance()->soundPackage->getSoundByLabel("playerJump"),
                          EngineSetup::SoundChannels::SND_PLAYER, 0);

    // apply force
    Vertex3D jump = EngineSetup::getInstance()->JUMP_FORCE;
    Vertex3D current = Brakeza3D::get()->getComponentsManager()->getComponentCamera()->getCamera()->velocity.vertex2;
    Brakeza3D::get()->getComponentsManager()->getComponentCamera()->getCamera()->velocity.vertex2 = current + jump;
}

void Player::reload() {
    Brakeza3D::get()->getComponentsManager()->getComponentWeapons()->reload();
}

void Player::respawnNPCS() {
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

void Player::getAid(float aid) {
    this->stamina = stamina + aid;

    if (stamina > (float) EngineSetup::getInstance()->GAME_PLAYER_STAMINA_INITIAL) {
        this->stamina = (float) EngineSetup::getInstance()->GAME_PLAYER_STAMINA_INITIAL;
    }
}