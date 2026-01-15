#include <iostream>
#include <cstring>
#include <tuple>
#include <vector>
#include <cstdlib>  // για rand()
#include <ctime>    // για time()
#include <cmath>    // για abs()
#include <algorithm>
#include <map>
using namespace std;

// Forward declarations
class GridWorld;
class Object;

//Struct για την θέση των αντικειμένων στον κόσμο.
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
    
    void print() const {
        cout << "  Object: " << objectId << " at (" << position.x << "," << position.y 
             << "), type: " << objectType << ", distance: " << distance 
             << ", confidence: " << confidence;
        if (!trafficLight.empty()) cout << ", light: " << trafficLight;
        if (!signText.empty()) cout << ", sign: " << signText;
        if (speed > 0) cout << ", speed: " << speed;
        cout << endl;
    }
};

//Βασική κλάση για τους αισθητήρες.
class Sensor {
protected:
    Position position;
    string type;
    string sensorId;
public:
    Sensor(string t, int x, int y, string id = "")
        : type(t), position(x, y), sensorId(id) {
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
    
    virtual void update(int tick) {}
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
    
    void updateAll(int tick) {
        for (auto obj : objects) {
            obj->update(tick);
        }
    }
    
    void removeObject(Object* obj) {
        auto it = find(objects.begin(), objects.end(), obj);
        if (it != objects.end()) {
            delete *it;
            objects.erase(it);
        }
    }
};

// LIDAR SENSOR
class LidarSensor : public Sensor {
public:
    LidarSensor(int x, int y) : Sensor("lidar", x, y, "LIDAR:0") {
        cout << "[+LIDAR: " << sensorId << "] Lidar sensor ready – Sensing with pew pews!" << endl;
    }
    
    virtual ~LidarSensor() { 
        cout << "[-SENSOR: " << sensorId << "] Sensor destroyed – No further data from me!" << endl;
    }

    vector<SensorReading> scan(const GridWorld& world, int carX, int carY, string carDir) override {
        vector<SensorReading> results;
        int range = 4; // 9x9 γύρω από το αυτοκίνητο

        for (auto obj : world.getObjects()) {
            Position pos = obj->getPosition();
            int dx = abs(pos.x - carX);
            int dy = abs(pos.y - carY);

            if (dx <= range && dy <= range) {
                SensorReading reading;
                reading.objectType = obj->getType();
                reading.objectId = obj->getID();
                reading.position = pos;
                reading.distance = dx + dy; // Manhattan distance
                
                // Βασική βεβαιότητα με θόρυβο ±0.05
                double baseConfidence = 0.99;
                double distanceFactor = 1.0 - (reading.distance / (double)(range * 2));
                reading.confidence = baseConfidence * distanceFactor;
                reading.confidence += ((rand() % 11) - 5) * 0.01; // ±0.05 θόρυβο
                if (reading.confidence > 1.0) reading.confidence = 1.0;
                if (reading.confidence < 0.0) reading.confidence = 0.0;
                
                reading.speed = obj->getSpeed();
                reading.direction = obj->getDirection();
                // Lidar doesn't read sign text or traffic lights well
                if (obj->getType() == "StopSign") reading.signText = obj->getSignText();
                results.push_back(reading);
            }
        }

        return results;
    }
};

// RADAR SENSOR
class RadarSensor : public Sensor {
public:
    RadarSensor(int x, int y) : Sensor("radar", x, y, "RADAR:0") {
        cout << "[+RADAR: " << sensorId << "] Radar sensor ready – I'm a Radio star!" << endl;
    }
    
    virtual ~RadarSensor() { 
        cout << "[-SENSOR: " << sensorId << "] Sensor destroyed – No further data from me!" << endl;
    }

    vector<SensorReading> scan(const GridWorld& world, int carX, int carY, string carDir) override {
        vector<SensorReading> results;
        int range = 12; // 12 θέσεις μπροστά

        for (auto obj : world.getObjects()) {
            Position pos = obj->getPosition();
            int dx = pos.x - carX;
            int dy = pos.y - carY;

            // radar βλέπει μόνο μπροστά (βάσει κατεύθυνσης)
            bool inFront = false;
            if (carDir == "E" && dx > 0 && dx <= range && abs(dy) <= 1) inFront = true;
            else if (carDir == "W" && dx < 0 && abs(dx) <= range && abs(dy) <= 1) inFront = true;
            else if (carDir == "N" && dy > 0 && dy <= range && abs(dx) <= 1) inFront = true;
            else if (carDir == "S" && dy < 0 && abs(dy) <= range && abs(dx) <= 1) inFront = true;
            
            if (inFront && obj->getSpeed() > 0) { // Radar sees only moving objects
                SensorReading reading;
                reading.objectType = obj->getType();
                reading.objectId = obj->getID();
                reading.position = pos;
                reading.distance = abs(dx) + abs(dy);
                
                // Βασική βεβαιότητα με θόρυβο
                double baseConfidence = 0.95;
                double distanceFactor = 1.0 - (reading.distance / (double)(range * 2));
                reading.confidence = baseConfidence * distanceFactor;
                reading.confidence += ((rand() % 11) - 5) * 0.01;
                if (reading.confidence > 1.0) reading.confidence = 1.0;
                if (reading.confidence < 0.0) reading.confidence = 0.0;
                
                reading.speed = obj->getSpeed();
                reading.direction = obj->getDirection();
                results.push_back(reading);
            }
        }

        return results;
    }
};

// CAMERA SENSOR
class CameraSensor : public Sensor {
public:
    CameraSensor(int x, int y) : Sensor("camera", x, y, "CAMERA:0") {
        cout << "[+CAMERA: " << sensorId << "] Camera sensor ready – Say cheese!" << endl;
    }
    
    virtual ~CameraSensor() { 
        cout << "[-SENSOR: " << sensorId << "] Sensor destroyed – No further data from me!" << endl;
    }

    vector<SensorReading> scan(const GridWorld& world, int carX, int carY, string carDir) override {
        vector<SensorReading> results;
        int range = 3; // 7x7 μπροστά από το αυτοκίνητο

        for (auto obj : world.getObjects()) {
            Position pos = obj->getPosition();
            int dx = pos.x - carX;
            int dy = pos.y - carY;
            
            bool inView = false;
            if (carDir == "E" && dx > 0 && dx <= range && abs(dy) <= range) inView = true;
            else if (carDir == "W" && dx < 0 && abs(dx) <= range && abs(dy) <= range) inView = true;
            else if (carDir == "N" && dy > 0 && dy <= range && abs(dx) <= range) inView = true;
            else if (carDir == "S" && dy < 0 && abs(dy) <= range && abs(dx) <= range) inView = true;

            if (inView) {
                SensorReading reading;
                reading.objectType = obj->getType();
                reading.objectId = obj->getID();
                reading.position = pos;
                reading.distance = abs(dx) + abs(dy);
                
                // Βασική βεβαιότητα με θόρυβο
                double baseConfidence = 0.87;
                double distanceFactor = 1.0 - (reading.distance / (double)(range * 2));
                reading.confidence = baseConfidence * distanceFactor;
                reading.confidence += ((rand() % 11) - 5) * 0.01;
                if (reading.confidence > 1.0) reading.confidence = 1.0;
                if (reading.confidence < 0.0) reading.confidence = 0.0;
                
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
        cout << "[+PARKED: " << getID() << "] Parked at (" << pos.x << "," << pos.y << ")" << endl;
    }
    
    ~ParkedCar() {
        cout << "[-PARKED: " << getID() << "] I'm being towed away!" << endl;
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
    int tickCounter;
public:
    TrafficLight(int num, Position pos, string initialState = "RED") 
        : StaticObject("TrafficLight", num, "T", pos), state(initialState), tickCounter(0) {
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
        return "T";
    }
    
    void update(int tick) override {
        tickCounter++;
        // Light cycle: RED(4) -> GREEN(8) -> YELLOW(2) -> RED
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
    
    virtual void move(GridWorld& world) {
        int newX = position.x;
        int newY = position.y;
        
        if (direction == "E") newX += speed;
        else if (direction == "W") newX -= speed;
        else if (direction == "N") newY += speed;
        else if (direction == "S") newY -= speed;
        
        if (world.inBounds(newX, newY)) {
            position.x = newX;
            position.y = newY;
        } else {
            // Remove from world if out of bounds
            world.removeObject(this);
        }
    }
    
    void update(int tick) override {
        // Moving objects don't need tick-based updates
    }
};

// Κλάση για τα ποδήλατα.
class Bike : public MovingObject {
public:
    Bike(int num, Position pos, string dir = "E", int spd = 1)
        : MovingObject("Bike", num, "B", pos, spd, dir) {
        cout << "[+BIKE: " << getID() << "] Created at (" << pos.x << "," << pos.y 
             << "), heading " << dir << " at " << spd << " units/tick" << endl;
    }
    
    ~Bike() {
        cout << "[-BIKE: " << getID() << "] Being locked away..." << endl;
    }
    
    string getType() const override { return "Bike"; }
    
    void update(int tick) override {
        // Bike movement will be handled in main loop
    }
};

// Κλάση για άλλα αυτοκίνητα
class OtherCar : public MovingObject {
public:
    OtherCar(int num, Position pos, string dir = "E", int spd = 1)
        : MovingObject("Car", num, "C", pos, spd, dir) {
        cout << "[+CAR: " << getID() << "] Initialized at (" << pos.x << "," << pos.y 
             << ") facing " << dir << " – No driver's license required!" << endl;
    }
    
    ~OtherCar() {
        cout << "[-CAR: " << getID() << "] Our journey is complete!" << endl;
    }
    
    string getType() const override { return "Car"; }
    
    void update(int tick) override {
        // Car movement will be handled in main loop
    }
};

// Μηχανή συγχώνευσης αισθητήρων
class SensorFusionEngine {
private:
    double minConfidenceThreshold;
    
public:
    SensorFusionEngine(double threshold = 0.4) : minConfidenceThreshold(threshold) {}
    
    vector<SensorReading> fuseSensorData(const vector<SensorReading>& allReadings) {
        map<string, vector<SensorReading>> readingsByObject;
        vector<SensorReading> fusedResults;
        
        // Ομαδοποίηση αναγνωρίσεων ανά ID αντικειμένου
        for (const auto& reading : allReadings) {
            readingsByObject[reading.objectId].push_back(reading);
        }
        
        // Συγχώνευση για κάθε αντικείμενο
        for (const auto& pair : readingsByObject) {
            const vector<SensorReading>& readings = pair.second;
            
            if (readings.empty()) continue;
            
            // Υπολογισμός σταθμισμένου μέσου όρου
            SensorReading fused;
            double totalWeight = 0.0;
            
            for (const auto& r : readings) {
                double weight = r.confidence;
                totalWeight += weight;
                
                // Αρχικοποίηση με την πρώτη ανάγνωση
                if (totalWeight == weight) {
                    fused = r;
                    fused.confidence = 0; // θα υπολογιστεί ξανά
                }
                
                // Σταθμισμένος μέσος όρος για confidence
                fused.confidence += r.confidence * weight;
            }
            
            if (totalWeight > 0) {
                fused.confidence /= totalWeight;
                
                // Ειδική περίπτωση για ποδήλατα - μην απορρίπτουμε ποτέ
                bool isBike = false;
                for (const auto& r : readings) {
                    if (r.objectType == "Bike") {
                        isBike = true;
                        break;
                    }
                }
                
                // Αποδοχή ανάγνωσης αν ξεπερνά το κατώφλι ή είναι ποδήλατο
                if (fused.confidence >= minConfidenceThreshold || isBike) {
                    fusedResults.push_back(fused);
                }
            }
        }
        
        return fusedResults;
    }
};

// Σύστημα Πλοήγησης
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
    }
    
    Position getCurrentTarget() const {
        if (currentTargetIndex < gpsTargets.size()) {
            return gpsTargets[currentTargetIndex];
        }
        return Position(-1, -1); // No target
    }
    
    void nextTarget() {
        if (currentTargetIndex < gpsTargets.size() - 1) {
            currentTargetIndex++;
        }
    }
    
    bool hasReachedTarget(const Position& carPos) const {
        if (currentTargetIndex >= gpsTargets.size()) return true;
        
        Position target = gpsTargets[currentTargetIndex];
        int distance = abs(carPos.x - target.x) + abs(carPos.y - target.y);
        return distance <= 1; // Θεωρούμε ότι έφτασε όταν είναι σε διπλανό κελί
    }
    
    string makeDecision(const Position& carPos, const string& carDir, 
                       const vector<SensorReading>& fusedReadings) {
        Position target = getCurrentTarget();
        
        if (target.x == -1) return "STOP"; // No more targets
        
        // Έλεγχος για εμπόδια
        for (const auto& reading : fusedReadings) {
            if (reading.distance <= 2 && reading.speed > 0) {
                // Κινούμενο αντικείμενο πολύ κοντά
                return "DECELERATE";
            }
            
            if (reading.distance <= 3 && 
                (reading.trafficLight == "RED" || reading.trafficLight == "YELLOW")) {
                // Κόκκινο ή κίτρινο φανάρι
                return "DECELERATE";
            }
            
            if (reading.distance <= 5 && reading.objectType == "StopSign") {
                // Σήμα STOP
                return "DECELERATE";
            }
        }
        
        // Έλεγχος απόστασης από στόχο
        int distanceToTarget = abs(carPos.x - target.x) + abs(carPos.y - target.y);
        if (distanceToTarget <= 5) {
            return "DECELERATE";
        }
        
        // Απόφαση κατεύθυνσης
        int dx = target.x - carPos.x;
        int dy = target.y - carPos.y;
        
        if (abs(dx) > abs(dy)) {
            // Προτεραιότητα στον οριζόντιο άξονα
            if (dx > 0 && carDir != "E") return "TURN_E";
            if (dx < 0 && carDir != "W") return "TURN_W";
        } else {
            // Προτεραιότητα στον κάθετο άξονα
            if (dy > 0 && carDir != "N") return "TURN_N";
            if (dy < 0 && carDir != "S") return "TURN_S";
        }
        
        // Αν είμαστε στη σωστή κατεύθυνση, επιτάχυνση
        return "ACCELERATE";
    }
    
    vector<SensorReading> processSensorData(const vector<SensorReading>& allReadings) {
        return fusionEngine.fuseSensorData(allReadings);
    }
};

//Κλάση για αυτόνομο αυτοκίνητο.
class SelfDrivingCar : public MovingObject {
private:
    CameraSensor camera;
    LidarSensor lidar;
    RadarSensor radar;
    NavigationSystem navigation;
    vector<SensorReading> lastReadings;
    
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

    CameraSensor& get_camera() {
        return camera;
    }

    LidarSensor& get_lidar() {
        return lidar;
    }

    RadarSensor& get_radar() {
        return radar;
    }
    
    NavigationSystem& get_navigation() {
        return navigation;
    }

    void accelerate() {
        if (speed == 0) speed = 1; // HALF_SPEED
        else if (speed == 1) speed = 2; // FULL_SPEED
    }
    
    void decelerate() {
        if (speed > 0) speed--;
    }
    
    void turn(const string& newDir) {
        direction = newDir;
    }
    
    void collectSensorData(const GridWorld& world) {
        lastReadings.clear();
        
        // Σάρωση από όλους τους αισθητήρες
        vector<SensorReading> cameraReadings = camera.scan(world, position.x, position.y, direction);
        vector<SensorReading> lidarReadings = lidar.scan(world, position.x, position.y, direction);
        vector<SensorReading> radarReadings = radar.scan(world, position.x, position.y, direction);
        
        // Συνδυασμός όλων των αναγνώσεων
        lastReadings.insert(lastReadings.end(), cameraReadings.begin(), cameraReadings.end());
        lastReadings.insert(lastReadings.end(), lidarReadings.begin(), lidarReadings.end());
        lastReadings.insert(lastReadings.end(), radarReadings.begin(), radarReadings.end());
    }
    
    void syncNavigationSystem() {
        // Δε χρειάζεται να κάνουμε τίποτα εδώ, τα δεδομένα είναι ήδη στις lastReadings
    }
    
    void executeMovement(GridWorld& world) {
        // Επεξεργασία δεδομένων και λήψη απόφασης
        vector<SensorReading> fusedReadings = navigation.processSensorData(lastReadings);
        string decision = navigation.makeDecision(position, direction, fusedReadings);
        
        // Εκτέλεση απόφασης
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
        } else if (decision == "STOP") {
            speed = 0;
            return;
        }
        
        // Μετακίνηση
        if (speed > 0) {
            move(world);
        }
        
        // Έλεγχος αν φτάσαμε στον στόχο
        if (navigation.hasReachedTarget(position)) {
            navigation.nextTarget();
        }
    }
    
    const vector<SensorReading>& getLastReadings() const {
        return lastReadings;
    }
    
    string getType() const override { return "SelfDrivingCar"; }
};

// Συναρτήσεις οπτικοποίησης
void visualization_full(const GridWorld& world, const SelfDrivingCar& car) {
    int dimX = world.getDimX();
    int dimY = world.getDimY();
    
    cout << "\n=== FULL WORLD VISUALIZATION ===" << endl;
    
    for (int y = dimY - 1; y >= 0; y--) {
        cout << "|";
        for (int x = 0; x < dimX; x++) {
            Position pos(x, y);
            Position carPos = car.getPosition();
            
            // Έλεγχος για το αυτόνομο όχημα
            if (pos.x == carPos.x && pos.y == carPos.y) {
                cout << "@";
                continue;
            }
            
            // Έλεγχος για άλλα αντικείμενα
            Object* obj = world.getObjectAt(x, y);
            if (obj) {
                cout << obj->getGlyph();
            } else {
                cout << ".";
            }
        }
        cout << "|" << endl;
    }
    cout << "================================\n" << endl;
}

void visualization_pov(const GridWorld& world, const SelfDrivingCar& car, int radius = 5) {
    Position carPos = car.getPosition();
    
    cout << "\n=== CAR'S POINT OF VIEW (radius: " << radius << ") ===" << endl;
    
    for (int y = carPos.y + radius; y >= carPos.y - radius; y--) {
        for (int x = carPos.x - radius; x <= carPos.x + radius; x++) {
            // Έλεγχος όρια κόσμου
            if (!world.inBounds(x, y)) {
                cout << "X";
                continue;
            }
            
            // Έλεγχος για το αυτόνομο όχημα
            if (x == carPos.x && y == carPos.y) {
                cout << "@";
                continue;
            }
            
            // Έλεγχος για άλλα αντικείμενα
            Object* obj = world.getObjectAt(x, y);
            if (obj) {
                cout << obj->getGlyph();
            } else {
                cout << ".";
            }
            cout << " ";
        }
        cout << endl;
    }
    cout << "==================================\n" << endl;
}

void print_logo() {
    cout << "==========================================" << endl;
    cout << "   SELF-DRIVING CAR SIMULATION 2025" << endl;
    cout << "==========================================" << endl;
}

//Η βοηθιτικη συναρτηση που μπορει ο χρηστης να καλεσει απο την γραμμη εντολων.
void print_help() {
    cout << "==========================================" << endl;
    cout << "   SELF-DRIVING CAR SIMULATION - HELP" << endl;
    cout << "==========================================" << endl;
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

//Αρχή της main
int main(int argc, char* argv[]) {
    print_logo();
    
    // Default τιμές
    int dimX = 40;
    int dimY = 40;
    int MovingCars = 3;
    int ParkedCars = 7;
    int MovingBikes = 4;
    int STOP = 1;
    int TrafficLights = 2;
    int ticks = 100;
    int seed = time(NULL);
    double minConfidenceThreshold = 0.4;

    // εκτυπώνω το βοηθητικό προς τον χρήστη αν το ζητήσει.
    if (argc > 1 && strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }

    // παίρνω τις παραμέτρους από την γραμμή εντολών
    int i = 1; 
    int x, y;
    vector<Position> destinations;
    bool flag = false;
    
    while(i < argc) {
        if (strcmp(argv[i], "--seed") == 0 && i+1 < argc) {
            seed = stoi(argv[i+1]);
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
        else if (strcmp(argv[i], "--gps") == 0 && i+2 < argc) {
            // Ο πρώτος στόχος είναι η αρχική θέση
            x = stoi(argv[i+1]);
            y = stoi(argv[i+2]);
            
            // Άλλοι στόχοι (αν υπάρχουν)
            for(int j = i+3; j+1 < argc; j += 2) {
                destinations.push_back(Position(stoi(argv[j]), stoi(argv[j+1])));
            }
            
            // Προσθήκη του πρώτου στόχου στην αρχή της λίστας
            destinations.insert(destinations.begin(), Position(x, y));
            flag = true;
            i += 2; // Προσπερνάμε τις συντεταγμένες
        }
        i++;
    }

    // Σε περίπτωση που δεν μου δώσει ο χρήστης συντεταγμένες
    if (!flag) {
        cout << "ERROR. GPS coordinates required" << endl;
        cout << "Try --help for usage information!" << endl;
        return 1;
    }
    
    // Ρύθμιση seed
    srand(seed);
    
    cout << "[+WORLD: GRID] Reticulating splines – Hello, world!" << endl;
    
    // Δημιουργία κόσμου
    GridWorld world(dimX, dimY);
    
    // Δημιουργία αυτόνομου οχήματος
    SelfDrivingCar car;
    car.setPosition(destinations[0].x, destinations[0].y);
    car.setNavigationTargets(destinations);
    
    // Πληθυσμός κόσμου με αντικείμενα
    vector<string> directions = {"N", "S", "E", "W"};
    
    // Προσθήκη ποδηλάτων
    for (int i = 0; i < MovingBikes; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        string dir = directions[rand() % 4];
        world.addObject(new Bike(i+1, pos, dir, 1));
    }
    
    // Προσθήκη άλλων αυτοκινήτων
    for (int i = 0; i < MovingCars; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        string dir = directions[rand() % 4];
        world.addObject(new OtherCar(i+1, pos, dir, 1));
    }
    
    // Προσθήκη παρκαρισμένων αυτοκινήτων
    for (int i = 0; i < ParkedCars; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        world.addObject(new ParkedCar(i+1, pos));
    }
    
    // Προσθήκη STOP signs
    for (int i = 0; i < STOP; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        world.addObject(new StopSign(i+1, pos));
    }
    
    // Προσθήκη φανάρια
    for (int i = 0; i < TrafficLights; i++) {
        Position pos(rand() % dimX, rand() % dimY);
        string initialState = directions[rand() % 3]; // RED, YELLOW, GREEN
        world.addObject(new TrafficLight(i+1, pos, initialState));
    }
    
    // Εκτέλεση προσομοίωσης
    cout << "\n=== STARTING SIMULATION ===" << endl;
    cout << "World Size: " << dimX << "x" << dimY << endl;
    cout << "GPS Targets: " << destinations.size() << endl;
    cout << "Starting Position: (" << destinations[0].x << "," << destinations[0].y << ")" << endl;
    cout << "Simulation Ticks: " << ticks << endl;
    cout << "Min Confidence Threshold: " << (minConfidenceThreshold * 100) << "%" << endl;
    
    // Αρχική οπτικοποίηση
    visualization_full(world, car);
    
    bool simulationRunning = true;
    
    for (int tick = 0; tick < ticks && simulationRunning; tick++) {
        cout << "\n=== TICK " << tick << " ===" << endl;
        cout << "Car at: (" << car.getPosition().x << "," << car.getPosition().y 
             << ") facing " << car.getDirection() << " speed: " << car.getSpeed() << endl;
        
        // Ενημέρωση όλων των αντικειμένων
        world.updateAll(tick);
        
        // Μετακίνηση κινούμενων αντικειμένων
        vector<Object*> objects = world.getObjects();
        for (auto obj : objects) {
            if (Bike* bike = dynamic_cast<Bike*>(obj)) {
                bike->move(world);
            } else if (OtherCar* otherCar = dynamic_cast<OtherCar*>(obj)) {
                otherCar->move(world);
            }
        }
        
        // Αυτόνομο όχημα: συλλογή δεδομένων
        car.collectSensorData(world);
        
        // Αυτόνομο όχημα: συγχρονισμός με σύστημα πλοήγησης
        car.syncNavigationSystem();
        
        // Εμφάνιση αναγνώσεων αισθητήρων
        cout << "\nSensor Readings:" << endl;
        const vector<SensorReading>& readings = car.getLastReadings();
        if (readings.empty()) {
            cout << "  No objects detected" << endl;
        } else {
            for (const auto& reading : readings) {
                reading.print();
            }
        }
        
        // Αυτόνομο όχημα: εκτέλεση κίνησης
        car.executeMovement(world);
        
        // Έλεγχος αν το αυτόνομο όχημα βγήκε εκτός ορίων
        Position carPos = car.getPosition();
        if (!world.inBounds(carPos.x, carPos.y)) {
            cout << "\n!!! CAR WENT OUT OF BOUNDS !!!" << endl;
            simulationRunning = false;
            break;
        }
        
        // Μερική οπτικοποίηση
        if (tick % 10 == 0 || tick == ticks - 1) {
            visualization_pov(world, car, 5);
        }
        
        // Μικρή καθυστέρηση για καλύτερη οπτικοποίηση
        // usleep(100000); // 100ms
    }
    
    // Τελική οπτικοποίηση
    cout << "\n=== SIMULATION COMPLETE ===" << endl;
    cout << "Final Position: (" << car.getPosition().x << "," << car.getPosition().y << ")" << endl;
    visualization_full(world, car);
    
    cout << "[-WORLD: GRID] Goodbye, cruel world!" << endl;
    
    return 0;
}