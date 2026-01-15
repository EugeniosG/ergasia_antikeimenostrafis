#include <iostream>
#include <cstring>
#include <tuple>
#include <vector>
#include <cstdlib>  // για rand()
#include <ctime>    // για time()
#include <cmath>    // για abs()
using namespace std;

// Forward declarations
class GridWorld;
class Object;

//Struct για την θέση των αντικειμένων στον κόσμο.
struct Position {
    int x;                                  
    int y;
    
    Position(int x = 0, int y = 0) : x(x), y(y) {}
};

struct ID {
    string type;
    int num;
    
    string toString() const {
        return type + ":" + to_string(num);
    }
};

//Struct για την αποθήκευση των αναγνώσεων των αισθητήρων.
struct SensorReading {
    string objectType;     // "Car", "Bike", "StopSign", etc.
    Position position;     // absolute position in the world
    string objectId;       // e.g., "CAR:3"
    double confidence;     // 0.0 to 1.0
    int distance;          // Manhattan distance from car
    int speed;             // if moving object, else 0
    string direction;      // "N", "S", "E", "W" (moving objects)
    string signText;       // if stop sign, yield, etc.
    string trafficLight;   // RED, YELLOW, GREEN, else ""
    
    SensorReading() : confidence(0.0), distance(0), speed(0) {}
};

//Βασική κλάση για τους αισθητήρες.
class Sensor {
protected:
    Position position;
    string type;
public:
    Sensor(string t, int x, int y)
        : type(t), position(x, y) {
    }
    
    virtual ~Sensor() {}
    
    void setPosition(int x, int y) {
        position.x = x;
        position.y = y;
    }
    
    virtual vector<SensorReading> scan(const GridWorld& world, int carX, int carY) = 0;
};

//Βασική κλάση για όλα τα αντικείμενα στον κόσμο.
class Object {
protected:
    Position position;
    ID id;
    string glyph;
public:
    Object() : id{"", 0}, glyph(""), position(0, 0) {}
    
    Object(string type, int num, string g, Position pos) 
        : id{type, num}, glyph(g), position(pos) {
    }
    
    virtual ~Object() {
    }

    void setPosition(int x, int y) {
        position.x = x;
        position.y = y;
    }
    
    virtual Position getPosition() const { 
        return position;
    }
    
    virtual string getType() const = 0;
    virtual string getID() const { return id.toString(); }
    virtual string getGlyph() const { return glyph; }
    virtual int getSpeed() const { return 0; }
    virtual string getDirection() const { return ""; }
    virtual string getSignText() const { return ""; }
    virtual string getTrafficLight() const { return ""; }
};

//κλασση που αντιπροσοπευει τον κοσμο της προσομοιωσης 
class GridWorld {
private:
    int dimX, dimY;
    vector<Object*> objects;

public:
    GridWorld(int x, int y) : dimX(x), dimY(y) {}

    ~GridWorld() {
        for (auto obj : objects) {
            delete obj;
        }
    }

    void addObject(Object* obj) {
        objects.push_back(obj);
    }

    vector<Object*> getObjects() const {
        return objects;
    }

    bool inBounds(int x, int y) const {
        return x >= 0 && x < dimX && y >= 0 && y < dimY;
    }
    
    int getDimX() const { return dimX; }
    int getDimY() const { return dimY; }
};

// LIDAR SENSOR
class LidarSensor : public Sensor {
public:
    LidarSensor(int x, int y) : Sensor("lidar", x, y) {
    }
    
    virtual ~LidarSensor() { 
    }

    vector<SensorReading> scan(const GridWorld& world, int carX, int carY) override {
        vector<SensorReading> results;
        int range = 4; // 9x9 γύρω από το αυτοκίνητο

        for (auto obj : world.getObjects()) {
            Position pos = obj->getPosition();
            int dx = pos.x - carX;
            int dy = pos.y - carY;

            if (abs(dx) <= range && abs(dy) <= range) {
                SensorReading reading;
                reading.objectType = obj->getType();
                reading.objectId = obj->getID();
                reading.position = pos;
                reading.distance = abs(dx) + abs(dy); // Manhattan distance
                reading.confidence = 0.99; // εξαιρετική ακρίβεια για απόσταση
                reading.speed = obj->getSpeed();
                reading.direction = obj->getDirection();
                reading.signText = obj->getSignText();
                reading.trafficLight = obj->getTrafficLight();
                results.push_back(reading);
            }
        }

        return results;
    }
};

// RADAR SENSOR
class RadarSensor : public Sensor {
public:
    RadarSensor(int x, int y) : Sensor("radar", x, y) {
    }
    
    virtual ~RadarSensor() { 
    }

    vector<SensorReading> scan(const GridWorld& world, int carX, int carY) override {
        vector<SensorReading> results;
        int range = 12; // 12 θέσεις μπροστά

        for (auto obj : world.getObjects()) {
            Position pos = obj->getPosition();
            int dx = pos.x - carX;
            int dy = pos.y - carY;

            // radar βλέπει μόνο μπροστά (προς τα δεξιά)
            if (dx > 0 && dx <= range && dy == 0) { 
                SensorReading reading;
                reading.objectType = obj->getType();
                reading.objectId = obj->getID();
                reading.position = pos;
                reading.distance = dx; 
                reading.confidence = 0.95; // υψηλή ακρίβεια
                reading.speed = obj->getSpeed();
                reading.direction = obj->getDirection();
                reading.signText = obj->getSignText();
                reading.trafficLight = obj->getTrafficLight();
                results.push_back(reading);
            }
        }

        return results;
    }
};

// CAMERA SENSOR
class CameraSensor : public Sensor {
public:
    CameraSensor(int x, int y) : Sensor("camera", x, y) {
    }
    
    virtual ~CameraSensor() { 
    }

    vector<SensorReading> scan(const GridWorld& world, int carX, int carY) override {
        vector<SensorReading> results;
        int range = 3; // 7x7 μπροστά από το αυτοκίνητο

        for (auto obj : world.getObjects()) {
            Position pos = obj->getPosition();
            int dx = pos.x - carX;
            int dy = pos.y - carY;

            // camera βλέπει μπροστά (προς τα δεξιά)
            if (dx > 0 && dx <= range && abs(dy) <= range) {
                SensorReading reading;
                reading.objectType = obj->getType();
                reading.objectId = obj->getID();
                reading.position = pos;
                reading.distance = abs(dx) + abs(dy);
                reading.confidence = 0.87; // μέτρια ακρίβεια
                reading.speed = obj->getSpeed();
                reading.direction = obj->getDirection();
                reading.signText = obj->getSignText();
                reading.trafficLight = obj->getTrafficLight();
                results.push_back(reading);
            }
        }

        return results;
    }
};

//Κλάση για στατικά αντικείμενα.
class StaticObject : public Object {
public:
    StaticObject(string type, int num, string glyph, Position pos) 
        : Object(type, num, glyph, pos) {
    }
    
    ~StaticObject() {
    }
};

//Κλάση για παρκαρισμένα αυτοκίνητα.
class ParkedCar : public StaticObject {
public:
    ParkedCar(int num, Position pos) 
        : StaticObject("ParkedCar", num, "P", pos) {
    }
    
    ~ParkedCar() {
    }
    
    string getType() const override { return "ParkedCar"; }
};

//Κλάση για τα σήματα STOP.
class StopSign : public StaticObject {
public:
    StopSign(int num, Position pos) 
        : StaticObject("StopSign", num, "S", pos) {
    }
    
    ~StopSign() {
    }
    
    string getType() const override { return "StopSign"; }
    string getSignText() const override { return "STOP"; }
};

//Κλάση για τα φανάρια.
class TrafficLight : public StaticObject {
private:
    string state;
public:
    TrafficLight(int num, Position pos, string State = "RED") 
        : StaticObject("TrafficLight", num, "T", pos), state(State) {
    }
    
    ~TrafficLight() {
    }
    
    string getType() const override { return "TrafficLight"; }
    string getTrafficLight() const override { return state; }
};

//Κλάση για κινητά αντικείμενα.
class MovingObject : public Object { 
protected:
    int speed;
    string direction;
public:
    MovingObject(string type, int num, string glyph, Position pos, 
                int Speed = 0, string Direction = "") 
        : Object(type, num, glyph, pos), speed(Speed), direction(Direction) {
    }
    
    ~MovingObject() {
    }
    
    int getSpeed() const override { return speed; }
    string getDirection() const override { return direction; }
    
    void setSpeed(int s) { speed = s; }
    void setDirection(const string& dir) { direction = dir; }
};

//Κλάση για αυτόνομο αυτοκίνητο.
class SelfDrivingCar : public MovingObject {
private:
    CameraSensor camera;
    LidarSensor lidar;
    RadarSensor radar;
public:
    SelfDrivingCar(Position pos = Position(0, 0)) 
        : MovingObject("SelfDrivingCar", 0, "C", pos, 0, "E"),
          camera(pos.x, pos.y), lidar(pos.x, pos.y), radar(pos.x, pos.y) {
    }
    
    ~SelfDrivingCar() {
    }

    void setPosition(int x, int y) {
        position.x = x;
        position.y = y;
        camera.setPosition(x, y);
        lidar.setPosition(x, y);
        radar.setPosition(x, y);
    }

    CameraSensor& get_camera() {
        return camera;
    }

    LidarSensor& get_lidar() {
        return lidar;
    }

    RadarSensor& get_radar() {
        return radar;
    }

    void accelerate() {
        speed++;
    }
    
    void decelerate() {
        if (speed > 0) speed--;
    }
    
    string getType() const override { return "SelfDrivingCar"; }
};

//κλάση για τα ποδήλατα.
class Bike : public MovingObject {
public:
    Bike(int num, Position pos, string dir = "E", int spd = 1)
        : MovingObject("Bike", num, "B", pos, spd, dir) {
    }
    
    ~Bike() {
    }
    
    string getType() const override { return "Bike"; }
};

//Κλάση για άλλα αυτοκίνητα
class OtherCar : public MovingObject {
public:
    OtherCar(int num, Position pos, string dir = "E", int spd = 1)
        : MovingObject("Car", num, "C", pos, spd, dir) {
    }
    
    ~OtherCar() {
    }
    
    string getType() const override { return "Car"; }
};

//Η βοηθιτικη συναρτηση που μπορει ο χρηστης να καλεσει απο την γραμμη εντολων.
void print_help() {
    cout << "--seed <n>                     Random seed (default current time)" << endl;
    cout << "--dimX <n>                     World width (default 40)" << endl;
    cout << "--dimY <n>                     World height (default 40)" << endl;
    cout << "--numMovingCars <n>            Number of moving cars (default 3)" << endl;
    cout << "--numMovingBikes <n>           Number of moving bikes (default 4)" << endl;
    cout << "--numParkedCars <n>            Number of parked cars (default 7)" << endl;
    cout << "--numStopSigns <n>             Number of signs STOP (default 1)" << endl;
    cout << "--numTrafficLights <n>         Number of traffic lights (default 2)" << endl;
    cout << "--simulationTicks <n>          Maximum simulation ticks (default 100)" << endl;
    cout << "--gps <x1> <y1> [x2 y2  ...]   GPS target coordinates (required)" << endl;
    cout << "--help                         Showing this message" << endl;
    cout << "Usage:" << endl;
    cout << "./project --seed <n> --dimX <n> --dimY <n> --numMovingCars <n> --numMovingBikes <n> --numParkedCars <n> --numStopSigns <n> --numTrafficLights <n> --simulationTicks <n> --gps <x1> <y1> [x2 y2  ...]" << endl;
}

tuple<int, int> NavigationSystem(int x, int y) { 
    return make_tuple(x, y);
}

void SensorFusionEngine(RadarSensor& radar, LidarSensor& lidar, 
                       CameraSensor& camera, tuple<int, int> destination) {
    // Υλοποίηση της συγχώνευσης εδώ
    cout << "Sensor fusion processing..." << endl;
}

//Αρχή της main
int main(int argc, char* argv[]) {
    // Default τιμές
    int dimX = 40;
    int dimY = 40;
    int MovingCars = 3;
    int ParkedCars = 7;
    int MovingBikes = 4;
    int STOP = 1;
    int TrafficLights = 2;
    int ticks = 100;

    // εκτυπώνω το βοηθητικό προς τον χρήστη αν το ζητήσει.
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }

    // παίρνω τις παραμέτρους από την γραμμή εντολών
    int i = 1; 
    int x, y;
    vector<tuple<int, int>> destinations;
    bool flag = false;
    
    while(i < argc) {
        if (strcmp(argv[i], "--dimX") == 0 && i+1 < argc) {
            dimX = stoi(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "--dimY") == 0 && i+1 < argc) {
            dimY = stoi(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "--numMovingCars") == 0 && i+1 < argc) {
            MovingCars = stoi(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "--numMovingBikes") == 0 && i+1 < argc) {
            MovingBikes = stoi(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "--numParkedCars") == 0 && i+1 < argc) {
            ParkedCars = stoi(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "--numStopSigns") == 0 && i+1 < argc) {
            STOP = stoi(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "--numTrafficLights") == 0 && i+1 < argc) {
            TrafficLights = stoi(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "--simulationTicks") == 0 && i+1 < argc) {
            ticks = stoi(argv[i+1]);
            i++;
        }
        else if (strcmp(argv[i], "--gps") == 0 && i+1 < argc) {
            x = stoi(argv[i+1]);
            y = stoi(argv[i+2]);
            for(int j = i+3; j+1 < argc; j += 2) {
                destinations.push_back(make_tuple(stoi(argv[j]), stoi(argv[j+1])));
            }
            flag = true;
            break;
        }
        i++;
    }

    // Σε περίπτωση που δεν μου δώσει ο χρήστης συντεταγμένες
    if (!flag) {
        cout << "ERROR. GPS coordinates required" << endl;
        cout << "Try Again !!!" << endl;
        return 1;
    }
    
    // Δημιουργία κόσμου
    GridWorld world(dimX, dimY);
    
    // Δημιουργία αυτόνομου οχήματος
    SelfDrivingCar ferrari;
    ferrari.setPosition(x, y);
    
    // Πληθυσμός κόσμου με αντικείμενα
    srand(time(NULL));
    
    // Προσθήκη ποδηλάτων
    for (int i = 0; i < MovingBikes; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        world.addObject(new Bike(i+1, pos));
    }
    
    // Προσθήκη άλλων αυτοκινήτων
    for (int i = 0; i < MovingCars; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        world.addObject(new OtherCar(i+1, pos));
    }
    
    // Προσθήκη παρκαρισμένων αυτοκινήτων
    for (int i = 0; i < ParkedCars; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        world.addObject(new ParkedCar(i+1, pos));
    }
    
    // Προσθήκη STOP 
    for (int i = 0; i < STOP; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        world.addObject(new StopSign(i+1, pos));
    }
    
    // Προσθήκη φανάρια
    for (int i = 0; i < TrafficLights; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        world.addObject(new TrafficLight(i+1, pos));
    }

    // Εκτέλεση προσομοίωσης
    cout << "\n=== Starting Simulation ===" << endl;
    cout << "World Size: " << dimX << "x" << dimY << endl;
    cout << "GPS Targets: " << destinations.size() << endl;
    cout << "Simulation Ticks: " << ticks << endl;
    
    return 0;
}