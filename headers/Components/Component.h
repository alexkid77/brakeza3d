//
// Created by darkhead on 8/1/20.
//

#ifndef BRAKEDA3D_COMPONENT_H
#define BRAKEDA3D_COMPONENT_H

#include <SDL2/SDL.h>
#include <vector>
#include "../Objects/Object3D.h"
#include "../EngineSetup.h"
#include "../EngineBuffers.h"

class ComponentsManager;

class Component {
private:
    std::vector<Object3D *> *sceneObjects;
    int id;
    std::string label;
public:

    Component();

    // Identification functions
    int getId() const;

    void setId(int id);

    const std::string &getLabel() const;

    void setLabel(const std::string &label);

    // Life cycle functions
    virtual void onStart() = 0;

    virtual void preUpdate() = 0;

    virtual void onUpdate() = 0;

    virtual void postUpdate() = 0;

    virtual void onEnd() = 0;

    virtual void onSDLPollEvent(SDL_Event *event, bool &finish) = 0;

    // get another components functions
    std::vector<Component *> *getComponents() const;

    Component *getComponentById(int id);

    void setComponents(std::vector<Component *> *components);

    // Accesor to scene objects
    std::vector<Object3D *> *getSceneObjects() const;

    void setSceneObjects(std::vector<Object3D *> *sceneObjects);

    std::vector<Component *> *components;

    // Accessors to Setup and Buffers
    EngineBuffers *BUFFERS;
    EngineSetup *SETUP;


};


#endif //BRAKEDA3D_COMPONENT_H
