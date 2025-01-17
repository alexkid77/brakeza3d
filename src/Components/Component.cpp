#include "../../include/Components/Component.h"

Component::Component() : SETUP(EngineSetup::get()), BUFFERS(EngineBuffers::getInstance()) {
    this->enabled = false;
}

std::vector<Component *> *Component::getComponents() const {
    return components;
}

void Component::setComponents(std::vector<Component *> *newComponent) {
    Component::components = newComponent;
}

Component *Component::getComponentById(unsigned int componentId) const {
    return this->components->at(componentId);
}

void Component::setSceneObjects(std::vector<Object3D *> *newSceneObjects) {
    Component::sceneObjects = newSceneObjects;
}

std::vector<Object3D *> *Component::getSceneObjects() const {
    return sceneObjects;
}

int Component::getId() const {
    return id;
}

void Component::setId(int newId) {
    Component::id = newId;
}

const std::string &Component::getLabel() const {
    return label;
}

void Component::setLabel(const std::string &newLabel) {
    Component::label = newLabel;
}

bool Component::isEnabled() const {
    return enabled;
}

void Component::setEnabled(bool enabled) {
    Component::enabled = enabled;
}
