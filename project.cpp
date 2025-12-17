#include <iostream>
#include <string>

using namespace std;

void print_help() {

}
//Κλάση για στατικά αντικείμενα
class StaticObject {
private:
    string id;
    char glyph;
public:
    StaticObject() {
        cout << "StaticObject initialized" << endl;
    }
    ~StaticObject() {
        cout << "StaticObject destroyed" << endl;
    }
};

//Κλάση για κινητά αντικείμενα
class MovingObject { 
private:
    int speed;
    int direction;
public:
    MovingObject() {
        cout << "MovingObject initialized" << endl;
    }
    ~MovingObject() {
        cout << "MovingObject destroyed" << endl;
    }
};

//Κλάση για αυτόνομο αυτοκίνητο
class Car : public MovingObject {
    private: 
        int speed;
        int direction;
    public:
        Car() {
            cout << "Car initialized" << endl;
        }
        ~Car() {
            cout << "Car destroyed" << endl;
        }
};


//Αρχή της main 
int main(int argc, char* argv[]){
    
    if (argv[1] == "--help") {
        print_help();
    }

    return 0;
}