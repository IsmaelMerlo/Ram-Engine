#include "Engine.h"

int main() {
    Engine engine;

    if (engine.init()) {
        engine.run();
    }

    engine.cleanup();
    return 0;
}