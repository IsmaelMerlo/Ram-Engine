#include "Engine.h"

#include <iostream>

void Engine::init() {

    isRunning = true;
    std::cout << "Ram Engine initialized \n";

}

void Engine::run() {
    while (isRunning) {
        // Game loop principal (pendiente)
        isRunning = false;
    }
}

void Engine::cleanup() {
    std::cout << "Ram Engine apagado \n";
}