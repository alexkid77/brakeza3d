//
// Created by eduardo on 16/3/22.
//

#ifndef BRAKEDA3D_FADERTOGAMESTATES_H
#define BRAKEDA3D_FADERTOGAMESTATES_H

#include "../include/Render/Shader.h"
#include "../include/Misc/Color.h"
#include "../include/EngineBuffers.h"

enum ShaderFadeToColorDirection {
    STOP = 0,
    OUT = 1,
    IN = -1,
};

class FaderToGameStates {
private:
    bool enabled;
    bool finished;
    bool endFadeOut;
    Color color;
    float speed;
    float progress;
    int screenWidth;
    int screenHeight;
    EngineSetup::GameState gameStateWhenEnds;
    ShaderFadeToColorDirection direction;
public:
    FaderToGameStates(Color color, float speed, EngineSetup::GameState gameStateWhenEnds, bool onlyFadeIn) {
        setEnabled(true);
        setColor(color);
        setProgress(0);
        setSpeed(speed);
        setFinished(false);
        setEndFadeOut(false);
        setGameStateWhenEnds(gameStateWhenEnds);
        setDirection(ShaderFadeToColorDirection::OUT);

        if (onlyFadeIn) {
            setupForFadeIn();
        }

        screenHeight = EngineSetup::get()->screenHeight;
        screenWidth = EngineSetup::get()->screenWidth;
    }

    void onUpdate() {

        if (!isEnabled() || isFinished()) {
            return;
        }

        auto buffer = EngineBuffers::getInstance();

        for (int y = 0; y < screenHeight; y++) {
            for (int x = 0; x < screenWidth; x++) {
                Color r = Tools::mixColor(Color(buffer->getVideoBuffer(x, y)), color, progress);
                buffer->setVideoBuffer(x, y, r.getColor());
            }
        }
        progress += speed * direction;

        if (progress <= 0)  {
            setProgress(0);
            setDirection(ShaderFadeToColorDirection::STOP);
            setFinished(true);
        }

        if (progress >= 1) {
            setProgress(1);
            setDirection(ShaderFadeToColorDirection::IN);
            setEndFadeOut(true);
        }
    }

    void resetTo(EngineSetup::GameState gameState) {
        setEnabled(true);
        setProgress(0);
        setFinished(false);
        setEndFadeOut(false);
        setGameStateWhenEnds(gameState);
        setDirection(ShaderFadeToColorDirection::OUT);
    }

    void setupForFadeIn() {
        setProgress(1);
        setEndFadeOut(true);
        setDirection(ShaderFadeToColorDirection::IN);
    }

    [[nodiscard]] ShaderFadeToColorDirection getDirection() const {
        return direction;
    }

    void setDirection(ShaderFadeToColorDirection direction) {
        FaderToGameStates::direction = direction;
    }

    EngineSetup::GameState getGameStateWhenEnds() const {
        return gameStateWhenEnds;
    }

    void setGameStateWhenEnds(EngineSetup::GameState gameStateWhenEnds) {
        FaderToGameStates::gameStateWhenEnds = gameStateWhenEnds;
    }

    [[nodiscard]] const Color &getColor() const {
        return color;
    }

    void setColor(const Color &color) {
        FaderToGameStates::color = color;
    }

    [[nodiscard]] float getSpeed() const {
        return speed;
    }

    void setSpeed(float speed) {
        FaderToGameStates::speed = speed;
    }

    [[nodiscard]] float getProgress() const {
        return progress;
    }

    void setProgress(float progress) {
        FaderToGameStates::progress = progress;
    }

    bool isEnabled() const {
        return enabled;
    }

    void setEnabled(bool enabled) {
        FaderToGameStates::enabled = enabled;
    }

    bool isFinished() const {
        return finished;
    }

    void setFinished(bool isFinished) {
        FaderToGameStates::finished = isFinished;
    }

    bool isEndFadeOut() const {
        return endFadeOut;
    }

    void setEndFadeOut(bool endFadeOut) {
        FaderToGameStates::endFadeOut = endFadeOut;
    }
};
#endif //BRAKEDA3D_FADERTOGAMESTATES_H
