#include <iostream>
#include <cstring>
#include <tuple>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <map>
#include <unistd.h> 
using namespace std;

// κανω declare απο πριν τις κλάσεις που θα χρειαστω
class GridWorld;
class Object;

//struct για την θεση αντικειμενων στον κοσμο
struct Position {
    int x;                                  
    int y;
    
    Position(int x = 0, int y = 0) : x(x), y(y) {}
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
    
    int distanceTo(const Position& other) const {
        return abs(x - other.x) + abs(y - other.y);
    }
};

//struct για την ταυτοτητα των αντικειμενων
struct ID {
    string type;
    int num;
    
    string toString() const {
        return type + ":" + to_string(num);
    }
};
//struct για την αποθηκευση των αναγνωσεων των αισθητηρων
struct SensorReading {
    string objectType;
    Position position;
    string objectId;
    double confidence;
    int distance;
    int speed;
    string direction;
    string signText;
    string trafficLight;
    
    SensorReading() : confidence(0.0), distance(0), speed(0) {}
    
    void print() const {
        cout << "  Object: " << objectId << " at (" << position.x << "," << position.y 
             << "), type: " << objectType << ", distance: " << distance 
             << ", confidence: " << confidence;
        if (!trafficLight.empty() && trafficLight != "N/A") cout << ", light: " << trafficLight;
        if (!signText.empty()) cout << ", sign: " << signText;
        if (speed > 0) cout << ", speed: " << speed << ", dir: " << direction;
        cout << endl;
    }
};

//βασικη κλαση για τους αισθητηρες 
class Sensor {
protected:
    Position position;
    string type;
    string sensorId;
    static int sensorCounter;
public:
    Sensor(string t, int x, int y) : type(t), position(x, y) {
        sensorCounter++;
        sensorId = t + ":" + to_string(sensorCounter);
    }
    
    virtual ~Sensor() {}
    
    void setPosition(int x, int y) {
        position.x = x;
        position.y = y;
    }
    
    virtual vector<SensorReading> scan(const GridWorld& world, int carX, int carY, string carDir) = 0;
    
    string getType() const { return type; }
    string getId() const { return sensorId; }
};

int Sensor::sensorCounter = 0;

//βασικη κλαση για ολα τα αντικειμενα στον κοσμο
class Object {
protected:
    Position position;
    ID id;
    string glyph;
    static map<string, int> objectCounters;
public:
    Object() : id{"", 0}, glyph(""), position(0, 0) {}
    
    Object(string type, int num, string g, Position pos) 
        : id{type, num}, glyph(g), position(pos) {
    }
    
    virtual ~Object() {}
    
    void setPosition(int x, int y) {
        position.x = x;
        position.y = y;
    }
    
    virtual Position getPosition() const { 
        return position;
    }

    //ολα τα getters
    virtual string getType() const = 0;
    virtual string getID() const { return id.toString(); }
    virtual string getGlyph() const { return glyph; }
    virtual int getSpeed() const { return 0; }
    virtual string getDirection() const { return ""; }
    virtual string getSignText() const { return ""; }
    virtual string getTrafficLight() const { return ""; }
    
    virtual void update(int tick) {}
    
    static int getNextId(string type) {
        return ++objectCounters[type];
    }
};

map<string, int> Object::objectCounters;

//κλασση που αντιπροσοπευει τον κοσμο της προσομοιωσης
class GridWorld {
private:
    int dimX, dimY;
    vector<Object*> objects;

public:
    GridWorld(int x, int y) : dimX(x), dimY(y) {
        cout << "[+WORLD: GRID] Reticulating splines – Hello, world!" << endl;
    }

    ~GridWorld() {
        for (auto obj : objects) {
            delete obj;
        }
        cout << "[-WORLD: GRID] Goodbye, cruel world!" << endl;
    }

    void addObject(Object* obj) {
        objects.push_back(obj);
    }

    vector<Object*> getObjects() const {
        return objects;
    }
    
    Object* getObjectAt(int x, int y) const {
        for (auto obj : objects) {
            Position pos = obj->getPosition();
            if (pos.x == x && pos.y == y) {
                return obj;
            }
        }
        return nullptr;
    }

    bool inBounds(int x, int y) const {
        return x >= 0 && x < dimX && y >= 0 && y < dimY;
    }
    
    int getDimX() const { return dimX; }
    int getDimY() const { return dimY; }
    
    //συναρτηση που καλει την update ολων των αντικειμενων
    void updateAll(int tick) {
        for (auto obj : objects) {
            obj->update(tick);
        }
    }
    
    //συναρτηση για την αφαιρεση αντικειμενων απο τον κοσμο
    void removeObject(Object* obj) {
        auto it = find(objects.begin(), objects.end(), obj);
        if (it != objects.end()) {
            delete *it;
            objects.erase(it);
        }
    }
};

//κλάση για αισθητηρα lidar
class LidarSensor : public Sensor {
public:
    LidarSensor(int x, int y) : Sensor("LIDAR", x, y) {
        cout << "[+LIDAR: " << sensorId << "] Lidar sensor ready – Sensing with pew pews!" << endl;
    }
    
    virtual ~LidarSensor() { 
        cout << "[-SENSOR: " << sensorId << "] Sensor destroyed – No further data from me!" << endl;
    }

    vector<SensorReading> scan(const GridWorld& world, int carX, int carY, string carDir) override {
        vector<SensorReading> results;
        int range = 4; // 9x9 area

        for (auto obj : world.getObjects()) {
            Position pos = obj->getPosition();
            int dx = pos.x - carX;
            int dy = pos.y - carY;

            if (abs(dx) <= range && abs(dy) <= range) {
                SensorReading reading;
                reading.objectType = obj->getType();
                reading.objectId = obj->getID();
                reading.position = pos;
                reading.distance = abs(dx) + abs(dy);
                
                // Base confidence with noise
                double baseConfidence = 0.99;
                double distanceFactor = 1.0 - (reading.distance / (double)(range * 2));
                reading.confidence = baseConfidence * distanceFactor;
                reading.confidence += ((rand() % 11) - 5) * 0.01;
                reading.confidence = max(0.0, min(1.0, reading.confidence));
                
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

//κλάση για αισθητηρα radar
class RadarSensor : public Sensor {
public:
    RadarSensor(int x, int y) : Sensor("RADAR", x, y) {
        cout << "[+RADAR: " << sensorId << "] Radar sensor ready – I'm a Radio star!" << endl;
    }
    
    virtual ~RadarSensor() { 
        cout << "[-SENSOR: " << sensorId << "] Sensor destroyed – No further data from me!" << endl;
    }

    vector<SensorReading> scan(const GridWorld& world, int carX, int carY, string carDir) override {
        vector<SensorReading> results;
        int range = 12;

        for (auto obj : world.getObjects()) {
            Position pos = obj->getPosition();
            int dx = pos.x - carX;
            int dy = pos.y - carY;

            //τσεκαρω αν το αντικειμενο ειναι μπροστα απο το αυτοκινητο
            bool inFront = false;
            int frontDistance = 0;
            
            if (carDir == "E" && dx > 0 && abs(dy) <= 1) {
                inFront = true;
                frontDistance = dx;
            } else if (carDir == "W" && dx < 0 && abs(dy) <= 1) {
                inFront = true;
                frontDistance = abs(dx);
            } else if (carDir == "N" && dy > 0 && abs(dx) <= 1) {
                inFront = true;
                frontDistance = dy;
            } else if (carDir == "S" && dy < 0 && abs(dx) <= 1) {
                inFront = true;
                frontDistance = abs(dy);
            }
            
            //αν ειναι μπροστα και μεσα στο range και κινητο το εντοπιζω
            if (inFront && frontDistance <= range && obj->getSpeed() > 0) {
                SensorReading reading;
                reading.objectType = obj->getType();
                reading.objectId = obj->getID();
                reading.position = pos;
                reading.distance = frontDistance;
                
                double baseConfidence = 0.95;
                double distanceFactor = 1.0 - (reading.distance / (double)range);
                reading.confidence = baseConfidence * distanceFactor;
                reading.confidence += ((rand() % 11) - 5) * 0.01;
                reading.confidence = max(0.0, min(1.0, reading.confidence));
                
                reading.speed = obj->getSpeed();
                reading.direction = obj->getDirection();
                results.push_back(reading);
            }
        }

        return results;
    }
};

//κλάση για αισθητηρα camera
class CameraSensor : public Sensor {
public:
    CameraSensor(int x, int y) : Sensor("CAMERA", x, y) {
        cout << "[+CAMERA: " << sensorId << "] Camera sensor ready – Say cheese!" << endl;
    }
    
    virtual ~CameraSensor() { 
        cout << "[-SENSOR: " << sensorId << "] Sensor destroyed – No further data from me!" << endl;
    }

    vector<SensorReading> scan(const GridWorld& world, int carX, int carY, string carDir) override {
        vector<SensorReading> results;
        int range = 3; // 7x7 area in front

        for (auto obj : world.getObjects()) {
            Position pos = obj->getPosition();
            int dx = pos.x - carX;
            int dy = pos.y - carY;
            
            //τσεκαρω αν το αντικειμενο ειναι μπροστα απο το αυτοκινητο
            bool inView = false;
            if (carDir == "E" && dx > 0 && dx <= range && abs(dy) <= range) inView = true;
            else if (carDir == "W" && dx < 0 && abs(dx) <= range && abs(dy) <= range) inView = true;
            else if (carDir == "N" && dy > 0 && dy <= range && abs(dx) <= range) inView = true;
            else if (carDir == "S" && dy < 0 && abs(dy) <= range && abs(dx) <= range) inView = true;

            //αν ειναι μπροστα και μεσα στο range το εντοπιζω
            if (inView) {
                SensorReading reading;
                reading.objectType = obj->getType();
                reading.objectId = obj->getID();
                reading.position = pos;
                reading.distance = abs(dx) + abs(dy);
                
                double baseConfidence = 0.87;
                double distanceFactor = 1.0 - (reading.distance / (double)(range * 2));
                reading.confidence = baseConfidence * distanceFactor;
                reading.confidence += ((rand() % 11) - 5) * 0.01;
                reading.confidence = max(0.0, min(1.0, reading.confidence));
                
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

//βασικη κλαση για κινητα αντικειμενα
class StaticObject : public Object {
public:
    StaticObject(string type, int num, string glyph, Position pos) 
        : Object(type, num, glyph, pos) {
    }
    
    ~StaticObject() {}
};

//κλάση για παρκαρισμενα αυτοκινητα
class ParkedCar : public StaticObject {
public:
    ParkedCar(Position pos) 
        : StaticObject("ParkedCar", Object::getNextId("ParkedCar"), "P", pos) {
        cout << "[+PARKED: " << getID() << "] Parked at (" << pos.x << "," << pos.y << ")" << endl;
    }
    
    ~ParkedCar() {
        cout << "[-PARKED: " << getID() << "] I'm being towed away!" << endl;
    }
    
    string getType() const override { return "ParkedCar"; }
};

class StopSign : public StaticObject {
public:
    StopSign(Position pos) 
        : StaticObject("StopSign", Object::getNextId("StopSign"), "S", pos) {
    }
    
    ~StopSign() {}
    
    string getType() const override { return "StopSign"; }
    string getSignText() const override { return "STOP"; }
};

class TrafficLight : public StaticObject {
private:
    string state;
    int tickCounter;
public:
    TrafficLight(Position pos) 
        : StaticObject("TrafficLight", Object::getNextId("TrafficLight"), "R", pos), 
          state("RED"), tickCounter(rand() % 14) { // Random starting point in cycle
        cout << "[+LIGHT: " << getID() << "] Initialized at (" << pos.x << "," << pos.y << ") to " << state << endl;
    }
    
    ~TrafficLight() {
        cout << "[-LIGHT: " << getID() << "] Turning off" << endl;
    }
    
    string getType() const override { return "TrafficLight"; }
    string getTrafficLight() const override { return state; }
    string getGlyph() const override { 
        if (state == "RED") return "R";
        if (state == "YELLOW") return "Y";
        if (state == "GREEN") return "G";
        return "?";
    }
    
    void update(int tick) override {
        tickCounter++;
        // κυκλοι φαναριου : κοκκινο -> 4 ticks - πρασινο -> 8 ticks - κιτρινο -> 2 ticks
        if (state == "RED" && tickCounter >= 4) {
            state = "GREEN";
            tickCounter = 0;
        } else if (state == "GREEN" && tickCounter >= 8) {
            state = "YELLOW";
            tickCounter = 0;
        } else if (state == "YELLOW" && tickCounter >= 2) {
            state = "RED";
            tickCounter = 0;
        }
    }
};

class MovingObject : public Object { 
protected:
    int speed;
    string direction;
public:
    MovingObject(string type, int num, string glyph, Position pos, 
                int Speed = 0, string Direction = "") 
        : Object(type, num, glyph, pos), speed(Speed), direction(Direction) {
    }
    
    ~MovingObject() {}
    
    int getSpeed() const override { return speed; }
    string getDirection() const override { return direction; }
    
    void setSpeed(int s) { speed = s; }
    void setDirection(const string& dir) { direction = dir; }
    
    virtual bool move(GridWorld& world) {
        int newX = position.x;
        int newY = position.y;
        
        if (direction == "E") newX += speed;
        else if (direction == "W") newX -= speed;
        else if (direction == "N") newY += speed;
        else if (direction == "S") newY -= speed;
        
        if (world.inBounds(newX, newY)) {
            position.x = newX;
            position.y = newY;
            return true;
        } else {
            return false;
        }
    }
    
    void update(int tick) override {}
};

class Bike : public MovingObject {
public:
    Bike(Position pos) 
        : MovingObject("Bike", Object::getNextId("Bike"), "B", pos, 1, "N") {
        // χρησημοποιω το rand() για να εχει τυχαι αρχικη κατευθηνση το ποδηλατο
        vector<string> dirs = {"N", "S", "E", "W"};
        direction = dirs[rand() % 4];
        cout << "[+BIKE: " << getID() << "] Created at (" << pos.x << "," << pos.y 
             << "), heading " << direction << " at " << speed << " units/tick" << endl;
    }
    
    ~Bike() {
        cout << "[-BIKE: " << getID() << "] Being locked away..." << endl;
    }
    
    string getType() const override { return "Bike"; }
};

class OtherCar : public MovingObject {
public:
    OtherCar(Position pos) 
        : MovingObject("Car", Object::getNextId("Car"), "C", pos, 1, "N") {
        // χρησημοποιω το rand() για να εχει τυχαια αρχικη κατευθηνση το κινημενο αυτοκινητο
        vector<string> dirs = {"N", "S", "E", "W"};
        direction = dirs[rand() % 4];
        cout << "[+CAR: " << getID() << "] Initialized at (" << pos.x << "," << pos.y 
             << ") facing " << direction << " – No driver's license required!" << endl;
    }
    
    ~OtherCar() {
        cout << "[-CAR: " << getID() << "] Our journey is complete!" << endl;
    }
    
    string getType() const override { return "Car"; }
};

class SensorFusionEngine {
private:
    double minConfidenceThreshold;
    
public:
    SensorFusionEngine(double threshold = 0.4) : minConfidenceThreshold(threshold) {}
    
    vector<SensorReading> fuseSensorData(const vector<SensorReading>& allReadings) {
        map<string, vector<SensorReading>> readingsByObject;
        vector<SensorReading> fusedResults;
        
        for (const auto& reading : allReadings) {
            readingsByObject[reading.objectId].push_back(reading);
        }
        
        for (const auto& pair : readingsByObject) {
            const vector<SensorReading>& readings = pair.second;
            
            if (readings.empty()) continue;
            
            SensorReading fused = readings[0];
            double totalConfidence = 0.0;
            int count = 0;
            
            // εννονω τα δεδομενα για το ιδιο αντικειμενο
            for (const auto& r : readings) {
                totalConfidence += r.confidence;
                count++;
                
            
                if (r.confidence > fused.confidence) {
                    if (!r.trafficLight.empty()) fused.trafficLight = r.trafficLight;
                    if (!r.signText.empty()) fused.signText = r.signText;
                    if (r.speed > 0) {
                        fused.speed = r.speed;
                        fused.direction = r.direction;
                    }
                }
            }
            
            fused.confidence = totalConfidence / count;
            
            bool isBike = readings[0].objectType == "Bike";
            if (fused.confidence >= minConfidenceThreshold || isBike) {
                fusedResults.push_back(fused);
            }
        }
        
        return fusedResults;
    }
};

class NavigationSystem {
private:
    vector<Position> gpsTargets;
    int currentTargetIndex;
    SensorFusionEngine fusionEngine;
    
public:
    NavigationSystem(double confidenceThreshold = 0.4) 
        : currentTargetIndex(0), fusionEngine(confidenceThreshold) {
        cout << "[+NAV: GPS] Hello, I'll be your GPS today" << endl;
    }
    
    ~NavigationSystem() {
        cout << "[-NAV: GPS] You've arrived! Shutting down..." << endl;
    }
    
    void setGPSTargets(const vector<Position>& targets) {
        gpsTargets = targets;
        currentTargetIndex = 0;
        cout << "Navigation set with " << targets.size() << " targets" << endl;
    }
    
    Position getCurrentTarget() const {
        if (currentTargetIndex < gpsTargets.size()) {
            return gpsTargets[currentTargetIndex];
        }
        return Position(-1, -1);
    }
    
    void nextTarget() {
        if (currentTargetIndex < gpsTargets.size() - 1) {
            currentTargetIndex++;
            cout << "Moving to next target: (" << getCurrentTarget().x 
                 << "," << getCurrentTarget().y << ")" << endl;
        } else {
            cout << "Reached final destination!" << endl;
        }
    }
    
    bool hasReachedTarget(const Position& carPos) const {
        if (currentTargetIndex >= gpsTargets.size()) return true;
        
        Position target = gpsTargets[currentTargetIndex];
        return carPos.distanceTo(target) == 0;
    }
    
    bool hasMoreTargets() const {
        return currentTargetIndex < gpsTargets.size();
    }
    
    string makeDecision(const Position& carPos, const string& carDir, 
                       const vector<SensorReading>& fusedReadings, int& carSpeed) {
        if (!hasMoreTargets()) return "STOP";
        
        Position target = getCurrentTarget();
        
        // ελεγχω αν εχουμε ηδη φτασει στον στοχο μας
        if (hasReachedTarget(carPos)) {
            return "NEXT_TARGET";
        }
        
        for (const auto& reading : fusedReadings) {
            if (reading.distance <= 2 && reading.speed > 0) {
                //κινουμενο αντικειμενο κοντα μας
                return "DECELERATE";
            }
            
            if (reading.distance <= 3 && 
                (reading.trafficLight == "RED" || reading.trafficLight == "YELLOW")) {
                // κοκκινο ή κιτρινο φαναρι
                return "DECELERATE";
            }
            
            if (reading.distance <= 5 && reading.objectType == "StopSign") {
                // STOP
                return "DECELERATE";
            }
        }
        
        // ελεγχω την απποσταση απο τον στοχο μηπως χρειαζεται να αλλαξω ταχυτητα
        int distanceToTarget = carPos.distanceTo(target);
        if (distanceToTarget <= 5 && carSpeed == 2) {
            return "DECELERATE";
        }
        else if (distanceToTarget <= 2 && carSpeed == 1){
            return "DECELERATE";
        }

        
        // καθοριζω την αποσταση οριζοντια και καθετα απο το αυτονομο οχημα μεχρι τον στοχο 
        int dx = target.x - carPos.x;
        int dy = target.y - carPos.y;
        
        // ελεγχω για αλλαγη κατευθηνσης 
        if (abs(dx) > abs(dy)) {
            // αναλογα με το προσημο της αποστασης αλαξω κατευθηνση προς τα δεξια(EAST) ή προς τα αριστερα(WEST)  
            if (dx > 0 && carDir != "E") return "TURN_E";
            if (dx < 0 && carDir != "W") return "TURN_W";
        } else {
            // αναλογα με το προσημο της αποστασης αλαξω κατευθηνση προς τα πανω(NORTH) ή προς τα κατω(SOUTH)  
            if (dy > 0 && carDir != "N") return "TURN_N";
            if (dy < 0 && carDir != "S") return "TURN_S";
        }
        
        // αν ειμαστε στην σωστη κατευθηνση για τον στοχο μας τοτε κανουμε accelerate
        if (carSpeed < 2) {
            return "ACCELERATE";
        }
        // αν δεν εγινε καμια αλλαγη στα παραπανω απλα συνεχιζουμε
        return "CONTINUE";
    }
    
    vector<SensorReading> processSensorData(const vector<SensorReading>& allReadings) {
        return fusionEngine.fuseSensorData(allReadings);
    }
};

class SelfDrivingCar : public MovingObject {
private:
    CameraSensor camera;
    LidarSensor lidar;
    RadarSensor radar;
    NavigationSystem navigation;
    vector<SensorReading> lastReadings;
    vector<SensorReading> fusedReadings;
    
public:
    SelfDrivingCar(Position pos = Position(0, 0)) 
        : MovingObject("SelfDrivingCar", 0, "@", pos, 0, "E"),
          camera(pos.x, pos.y), lidar(pos.x, pos.y), radar(pos.x, pos.y), navigation() {
        cout << "[+VEHICLE: " << getID() << "] Created at (" << pos.x << "," << pos.y 
             << "), heading " << direction << " at " << speed << " units/tick" << endl;
    }
    
    ~SelfDrivingCar() {
        cout << "[-CAR: " << getID() << "] Being scrapped..." << endl;
    }

    void setPosition(int x, int y) {
        position.x = x;
        position.y = y;
        camera.setPosition(x, y);
        lidar.setPosition(x, y);
        radar.setPosition(x, y);
    }
    
    void setNavigationTargets(const vector<Position>& targets) {
        navigation.setGPSTargets(targets);
    }

    CameraSensor& get_camera() { return camera; }
    LidarSensor& get_lidar() { return lidar; }
    RadarSensor& get_radar() { return radar; }
    NavigationSystem& get_navigation() { return navigation; }

    void accelerate() {
        if (speed < 2)
        speed ++;
    }
    
    void decelerate() {
        if (speed > 0) {
            speed--;
        }
    }
    
    void turn(const string& newDir) {
        if (direction != newDir) {
            cout << "  Turning from " << direction << " to " << newDir << endl;
            direction = newDir;
        }
    }
    
    void collectSensorData(const GridWorld& world) {
        lastReadings.clear();
        
        vector<SensorReading> cameraReadings = camera.scan(world, position.x, position.y, direction);
        vector<SensorReading> lidarReadings = lidar.scan(world, position.x, position.y, direction);
        vector<SensorReading> radarReadings = radar.scan(world, position.x, position.y, direction);
        
        lastReadings.insert(lastReadings.end(), cameraReadings.begin(), cameraReadings.end());
        lastReadings.insert(lastReadings.end(), lidarReadings.begin(), lidarReadings.end());
        lastReadings.insert(lastReadings.end(), radarReadings.begin(), radarReadings.end());
    }
    
    void syncNavigationSystem() {
        fusedReadings = navigation.processSensorData(lastReadings);
    }
    
    // σηναρτηση για την εκτελεση των αποφασεων 
    bool executeMovement(GridWorld& world) {
        string decision = navigation.makeDecision(position, direction, fusedReadings, speed);
        
        cout << "  Decision: " << decision << endl;
        
        if (decision == "ACCELERATE") {
            accelerate();
        } else if (decision == "DECELERATE") {
            decelerate();
        } else if (decision == "TURN_E") {
            turn("E");
        } else if (decision == "TURN_W") {
            turn("W");
        } else if (decision == "TURN_N") {
            turn("N");
        } else if (decision == "TURN_S") {
            turn("S");
        } else if (decision == "NEXT_TARGET") {
            navigation.nextTarget();
            return true;
        } else if (decision == "STOP") {
            speed = 0;
            cout << "  Final destination reached!" << endl;
            return false;
        }
        
        // Move the car
        if (speed > 0) {
            if (!move(world)) {
                cout << "!!! CAR WENT OUT OF BOUNDS !!!" << endl;
                return false;
            }
        }
        
        return true;
    }
    
    const vector<SensorReading>& getLastReadings() const {
        return lastReadings;
    }
    
    const vector<SensorReading>& getFusedReadings() const {
        return fusedReadings;
    }
    
    string getType() const override { return "SelfDrivingCar"; }
};

//συναρτηση για την εκτηποση του κοσμου
void visualization_full(const GridWorld& world, const SelfDrivingCar& car) {
    int dimX = world.getDimX();
    int dimY = world.getDimY();
    
    //βαζω στα ορια τον χαρακτηρα "Χ" οπως λεει η εκγωνηση της εργασιας 
    cout << "X";
    for (int x = 0; x < dimX; x++) cout << "X";
    cout << "X" << endl;
    
    for (int y = dimY - 1; y >= 0; y--) {
        cout << "X";
        for (int x = 0; x < dimX; x++) {
            Position pos(x, y);
            Position carPos = car.getPosition();
            
            //στην θεση του αυτονομου αυτοκινητου βαζω το συμβολο @ συμφωνα με την εκφωνηση
            if (pos.x == carPos.x && pos.y == carPos.y) {
                cout << "@";
                continue;
            }
            
            Object* obj = world.getObjectAt(x, y);
            if (obj) {
                cout << obj->getGlyph();
            } else {
                cout << ".";
            }
        }
        cout << "X" << endl;
    }
    

    cout << "X";
    for (int x = 0; x < dimX; x++) cout << "X";
    cout << "X" << endl;
}

// υλοποιω μια συναρτηση για την μερικη οπτικοποιηση  και γινεται σε καθε κυκλο (tick)
void visualization_pov(const GridWorld& world, const SelfDrivingCar& car, int radius = 5) {
    Position carPos = car.getPosition();
    
    cout << "\n=== CAR'S POINT OF VIEW (radius: " << radius << ") ===" << endl;
    cout << "Car at (" << carPos.x << "," << carPos.y << ") facing " << car.getDirection() << endl;
    
    for (int y = carPos.y + radius; y >= carPos.y - radius; y--) {
        for (int x = carPos.x - radius; x <= carPos.x + radius; x++) {
            if (!world.inBounds(x, y)) {
                cout << " ";
                continue;
            }
            
            if (x == carPos.x && y == carPos.y) {
                cout << "@";
            } else {
                Object* obj = world.getObjectAt(x, y);
                if (obj) {
                    cout << obj->getGlyph();
                } else {
                    cout << ".";
                }
            }
        }
        cout << endl;
    }
}


//βοηθητικη συναρτηση για διευκοληνση του χρηστη να καλεσει σωστα το προγραμμα
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
    cout << "--minConfidenceThreshold <n>   Minimum confidence threshold (default 40)" << endl;
    cout << "--gps <x1> <y1> [x2 y2  ...]   GPS target coordinates (required)" << endl;
    cout << "--help                         Showing this message" << endl;
    cout << "\nUsage:" << endl;
    cout << "./project --seed 12 --dimX 40 --dimY 40 --gps 10 20 30 15" << endl;
}

int main(int argc, char* argv[]) {
    // αρχικοποιω της τιμες συμφωνα με την εκφωνηση 
    int dimX = 40;
    int dimY = 40;
    int MovingCars = 3;
    int ParkedCars = 7;
    int MovingBikes = 4;
    int STOP = 1;
    int TrafficLights = 2;
    int ticks = 100;
    int seed ;
    double minConfidenceThreshold = 0.4;

    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }

    int i = 1; 
    vector<Position> destinations;
    bool gpsProvided = false;
    bool seedProvided = false;
    while(i < argc) {
        if (strcmp(argv[i], "--seed") == 0 && i+1 < argc) {
            seed = stoi(argv[i+1]);
            seedProvided = true;
            i++;
        }
        else if (strcmp(argv[i], "--dimX") == 0 && i+1 < argc) {
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
        else if (strcmp(argv[i], "--minConfidenceThreshold") == 0 && i+1 < argc) {
            minConfidenceThreshold = stoi(argv[i+1]) / 100.0;
            i++;
        }
        else if (strcmp(argv[i], "--gps") == 0 && i+1 < argc) {
            // περνω τις συντεταγμενες gps που μου δινει ο χρηστης κατα την κληση του προγραμματος
            //για την θεση του αυτοκινουμενου οχηματος αλλα και για τον(-ους) στοχο(-ους)
            for (int j = i + 1; j + 1 < argc; j += 2) {
                try {
                    int x = stoi(argv[j]);
                    int y = stoi(argv[j+1]);
                    destinations.push_back(Position(x, y));
                    gpsProvided = true;
                } catch (...) {
                    break;
                }
            }
            break;
        }
        i++;
    }

    if (!gpsProvided) {
        cout << "ERROR: GPS coordinates required!" << endl;
        cout << "Use --gps <x1> <y1> [x2 y2 ...]" << endl;
        cout << "Try --help for usage information" << endl;
        return 1;
    }
    
    if (destinations.empty()) {
        cout << "ERROR: At least one GPS coordinate required!" << endl;
        return 1;
    }
    
    // ελεγχω ασν εχει δωθει απο τον χρηστη τιμη seed αν δεν εχει με την βοηθεια της srand περναω μια τυχαια τιμη στην μεταβλητη seed 
    if(seedProvided){
        srand(seed); 
    }

    // δημιουργω τον κοσμο 
    GridWorld world(dimX, dimY);
    
    // δημιουργω το αυτοκινουμενο οχημα με της συντεταγμενες που εδωσε ο χρηστης
    SelfDrivingCar car;
    car.setPosition(destinations[0].x, destinations[0].y);
    
    // αν υπαρχουν παραπανω απο ενας στοχος τους περναω σε vector
    if (destinations.size() > 1) {
        vector<Position> remainingTargets(destinations.begin() + 1, destinations.end());
        car.setNavigationTargets(remainingTargets);
    } 
    else {
        //αλλιως εχω ηδη φτασει στοιν τελικο προορισμο και τερματιζω το προγραμμα αφου κανω εκτυπωση της τελικης καταστασης
        visualization_full(world, car);
        return 0;
    }
    
    vector<string> directions = {"N", "S", "E", "W"};
    
    // προσθετω στον κοσμο τα ποδηλατα
    for (int i = 0; i < MovingBikes; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        // Ensure not on car position
        while (pos.x == car.getPosition().x && pos.y == car.getPosition().y) {
            pos = Position(rand() % dimX, rand() % dimY);
        }
        world.addObject(new Bike(pos));
    }
    
    // προσθετω στον κοσμο τα υπολοιπα κινουμενα αυτοκινητα
    for (int i = 0; i < MovingCars; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        while (pos.x == car.getPosition().x && pos.y == car.getPosition().y) {
            pos = Position(rand() % dimX, rand() % dimY);
        }
        world.addObject(new OtherCar(pos));
    }
    
    // προσθετω στον κοσμο τα παρκαριμενα αυτοκινητα
    for (int i = 0; i < ParkedCars; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        while (pos.x == car.getPosition().x && pos.y == car.getPosition().y) {
            pos = Position(rand() % dimX, rand() % dimY);
        }
        world.addObject(new ParkedCar(pos));
    }
    
    // προσθετω στον κοσμο τα σηματα STOP
    for (int i = 0; i < STOP; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        while (pos.x == car.getPosition().x && pos.y == car.getPosition().y) {
            pos = Position(rand() % dimX, rand() % dimY);
        }
        world.addObject(new StopSign(pos));
    }
    
    // προσθετω στιν κοσμο τα φαναρια
    for (int i = 0; i < TrafficLights; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        while (pos.x == car.getPosition().x && pos.y == car.getPosition().y) {
            pos = Position(rand() % dimX, rand() % dimY);
        }
        world.addObject(new TrafficLight(pos));
    }
    
    // εκτυπωνωψ την πληρη εικονα του κοσμου με την συναρτηση visualization_full
    visualization_full(world, car);
    
    bool simulationRunning = true;
    bool carRunning = true;
    
    for (int tick = 0; tick < ticks && simulationRunning && carRunning; tick++) {
        
        // ενημερωση των αντικειμενων του κοσμου 
        world.updateAll(tick);
        
        vector<Object*> objects = world.getObjects();
        for (auto obj : objects) {
            if (Bike* bike = dynamic_cast<Bike*>(obj)) {
                if (!bike->move(world)) {
                    world.removeObject(obj);
                }
            } else if (OtherCar* otherCar = dynamic_cast<OtherCar*>(obj)) {
                if (!otherCar->move(world)) {
                    world.removeObject(obj);
                }
            }
        }
        
        // εδω το αμαξι εκτελει τις βασικες του λειτουργιες(συλεγει πληροφοριες απο τους αισθητηρες και ενεργοποιει το σθστημα πλοηγησης)
        car.collectSensorData(world);
        car.syncNavigationSystem();
        
        //εμφανιζω τα στοιχεια των αιθητηρων
        const vector<SensorReading>& readings = car.getLastReadings();
        if (readings.empty()) {
            cout << "  No objects detected" << endl;
        } else {
            for (const auto& reading : readings) {
                reading.print();
            }
        }
        
        cout << "\nFused Sensor Readings:" << endl;
        const vector<SensorReading>& fused = car.getFusedReadings();
        if (fused.empty()) {
            cout << "  No fused readings" << endl;
        } else {
            for (const auto& reading : fused) {
                reading.print();
            }
        }
        
        //το αμαξι εδω εκτελει τις κινησεις για να κατευθηνθει προς τον στοχο
        carRunning = car.executeMovement(world);
        
        // ελεγχω αν το αμαξι ειναι εκτος οριων 
        if (!world.inBounds(car.getPosition().x, car.getPosition().y)) {
            cout << "\n!!! CAR WENT OUT OF BOUNDS !!!" << endl;
            simulationRunning = false;
            break;
        }
        
        // εμφανιζω το κομματι του κοσμου που ειναι γυρο απο το οχημα
        if (tick % 10 == 0 || tick == ticks - 1) {
            visualization_pov(world, car, 5);
        }
    }
    
    // εκτυπωνων προς τον χρηστη την τελικη θεση του αυτονομου αυτοκινητου στον κοσμο
    cout << "Final Position: (" << car.getPosition().x << "," << car.getPosition().y << ")" << endl;
    
    if (car.get_navigation().hasMoreTargets()) {
        cout << "Remaining targets: Yes" << endl;
    } else {
        cout << "All targets reached!" << endl;
    }
    
    visualization_full(world, car);
    
    return 0;
}