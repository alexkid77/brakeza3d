
#include <utility>

#include "../../headers/Render/Logging.h"
#include "../../headers/EngineSetup.h"

Logging *Logging::instance = nullptr;

Logging *Logging::getInstance() {
    if (instance == nullptr) {
        instance = new Logging();
    }

    return instance;
}

void Logging::Log(const std::string& message, const std::string& type) {
    if (!EngineSetup::getInstance()->LOGGING) return;

    if (EngineSetup::getInstance()->LOGGING_TO_FILE) {
        FILE *f = fopen("brakeza.log", "a");
        if (f == nullptr) {
            std::cout << "Error opening log file!" << std::endl;
            exit(1);
        }
        fprintf(f, "%s\n", message.c_str());
        fclose(f);
    }

    std::cout << '[' << type << ']' << ' ' << message << std::endl;
}

void Logging::Log(std::string message) {
    this->Log(std::move(message), "DEBUG");
}