#include "../../include/Components/ComponentInput.h"
#include "../../include/ComponentsManager.h"

ComponentInput::ComponentInput() {
    setEnabled(true);
}

void ComponentInput::onStart() {
    Logging::Log("ComponentInput onStart", "ComponentInput");
}

void ComponentInput::preUpdate() {
    updateKeyboardMapping();

}

void ComponentInput::onUpdate() {
    if (!isEnabled()) return;
    handleKeyboardMovingCamera();
}

void ComponentInput::postUpdate() {

}

void ComponentInput::onEnd() {

}

void ComponentInput::onSDLPollEvent(SDL_Event *e, bool &finish) {
    updateMouseStates(e);
    handleWindowEvents(e, finish);

    if (!isEnabled()) return;

    handleMouse(e);
}


void ComponentInput::handleMouse(SDL_Event *event) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    // Camera rotation
    if (MouseMotion && MousePressed) {
        MouseMotion = false;
        if (event->type == SDL_MOUSEMOTION) {
            ComponentsManager::get()->getComponentCamera()->getCamera()->Yaw(event->motion.xrel);
            ComponentsManager::get()->getComponentCamera()->getCamera()->Pitch(event->motion.yrel);
        }
    }

    // Firing
    if (MousePressed) {
    }
}

void ComponentInput::handleKeyboardMovingCamera() const {
    if (keyboard[SDL_SCANCODE_W]) {
        ComponentsManager::get()->getComponentCamera()->getCamera()->MoveForward();
    }
    if (keyboard[SDL_SCANCODE_S]) {
        ComponentsManager::get()->getComponentCamera()->getCamera()->MoveBackward();
    }
    if (keyboard[SDL_SCANCODE_A]) {
        ComponentsManager::get()->getComponentCamera()->getCamera()->StrafeLeft();
    }
    if (keyboard[SDL_SCANCODE_D]) {
        ComponentsManager::get()->getComponentCamera()->getCamera()->StrafeRight();
    }
    if (keyboard[SDL_SCANCODE_RIGHT]) {
        ComponentsManager::get()->getComponentCamera()->getCamera()->TurnRight();
    }
    if (keyboard[SDL_SCANCODE_LEFT]) {
        ComponentsManager::get()->getComponentCamera()->getCamera()->TurnLeft();
    }
    if (keyboard[SDL_SCANCODE_DOWN]) {
        ComponentsManager::get()->getComponentCamera()->getCamera()->PitchUp();
    }
    if (keyboard[SDL_SCANCODE_UP]) {
        ComponentsManager::get()->getComponentCamera()->getCamera()->PitchDown();
    }
}

void ComponentInput::handleWindowEvents(SDL_Event *e, bool &end) {
    if (e->type == SDL_WINDOWEVENT) {
        switch (e->window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                end = true;
                break;
#if SDL_VERSION_ATLEAST(2, 0, 5)
            case SDL_WINDOWEVENT_TAKE_FOCUS:
                break;
            case SDL_WINDOWEVENT_HIT_TEST:
                break;
#endif
            default:
                break;
        }
    }
}

void ComponentInput::updateKeyboardMapping() {
    this->keyboard = (unsigned char *) SDL_GetKeyboardState(nullptr);
}

void ComponentInput::updateMouseStates(SDL_Event *event) {
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        MousePressed = true;
    }
    if (event->button.button == SDL_BUTTON_LEFT) {
        leftButton = true;
        rightButton = false;
    }

    if (event->button.button == SDL_BUTTON_RIGHT) {
        rightButton = true;
        leftButton = false;
    }

    if (event->type == SDL_MOUSEBUTTONUP) {
        MousePressed = false;
    }

    if (event->type == SDL_MOUSEMOTION) {
        MouseMotion = true;
    }
}

bool ComponentInput::isEnabled() const {
    return enabled;
}

void ComponentInput::setEnabled(bool enabled) {
    ComponentInput::enabled = enabled;
}
