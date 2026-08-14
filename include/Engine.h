#pragma once

class Engine {

public:
    void init();
    void run();
    void cleanup();

private:
    bool isRunning = false;
};