#include <iostream>
#include <string>

using namespace std;

void print_help() {
    cout << "Usage:" << endl;
    cout << "--seed <n>                     Randome seed (default current time)" << endl;
    cout << "--dimX <n>                     World width (default 40)" << endl;
    cout << "--dimY <n>                     World height (default 40)" << endl;
    cout << "--numMovingCars <n>            Number of moving cars (default 3)" << endl;
    cout << "--numMovingBikes <n>           Number of moving bikes (default 4)" << endl;
    cout << "--numParkedCars <n>            Number of moving bikes (default 7)" << endl;
    cout << "--numStopSigns <n>             Number of signs STOP (deafault 1)" << endl;
    cout << "--numTraficLights <n>          Number of trafic light (default 2)" <<  endl;
    cout << "--simulatorticks <n>           Maximun simulator ticks (default 100)" << endl;
    cout << "--gps <x1> <y1> [x2 y2  ...]   Gps target coordinates (required)" << endl;
    cout << "--help                         Showing this message" << endl;
    cout << "example Usage:" << endl;
    cout << "./project --seed 10 --dimX 40 --simulatorticks 200" << endl;
}

//Κλάση για στατικά αντικείμενα
class StaticObject {
private:
    int x, y;
    string id;
    int glyph;
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
    int x, y;
    string id;
    int glyph;
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
    //εκτυπωνω το βοηθητικο προς τον χρηστη αν το ζητησει
    if (strcmp(argv[1], "--help") == 0) {
        print_help();
    }

    //παιρνω τις συντεταγμενες του αυτοκινουμενου οχημετος που μου εχει δωσει ο χρηστης 
    int i = 1; 
    while(1){
        if (strcmp(argv[i], "--gps") == 0){
            int x = stoi(argv[i+1]);
            int y = stoi(argv[i+2]);
            continue;
        }
        i++;
    }

    return 0;
}