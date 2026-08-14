#include <iostream>
#include <string>

// Player class with caracteristics
class Player {

    public:
    std::string name;
    int health = 100;

    void takeDamage(int amount) {
        health -= amount;
        if (health <= 0) {
            std::cout << name << "Ha caído en batalla... \n";
        }
        else {
            std::cout << name << " tiene " << health << " HP restante. \n";
        }
    }
};

// Principal function
int main() {

    Player hero;
    hero.name = "Aragorn";
    hero.takeDamage(40);

    return 0;
}