#include <iostream>
#include <cstring>
#include <tuple>
#include <vector>
using namespace std;

//Struct για την θέση των αντικειμένων στον κόσμο.
struct Position {
    int x;                                  
    int y;
};

//Βασική κλάση για όλα τα αντικείμενα στον κόσμο.
class Object {
    protected:
        Position position;
        string id;
        int glyph;
    public:

        Object(string id, int glyph, Position pos = {0,0}) : id(id), glyph(glyph), position(pos) {
            cout << "Object initialized" << endl;
        }
        ~Object() {
            cout << "Object destroyed" << endl;
        }

        Position getPosition() { 
            return position;
        }

        void setPosition(int x, int y){
            position.x = x;
            position.y = y;
        }
};

//Κλάση για στατικά αντικείμενα.
class StaticObject : public Object {
public:
    StaticObject(string id="", int glyph = 0, Position pos={0,0}) : Object(id, glyph, pos) {
        cout << "StaticObject initialized" << endl;
    }
    ~StaticObject() {
        cout << "StaticObject destroyed" << endl;
    }
};

//Κλάση για παρκαρισμένα αυτοκίνητα.
class ParkedCar : public StaticObject {
    public:
        ParkedCar(Position pos = {0,0}) : StaticObject("ParkedCar", 'P', pos) {
            cout << "ParkedCar initialized" << endl;
        }
        ~ParkedCar() {
            cout << "ParkedCar destroyed" << endl;
        }
};

//Κλάση για τα σήματα STOP.
class StopSign : public StaticObject {
    public:
        StopSign(Position pos = {0,0}) : StaticObject("StopSign", 'S', pos) {
            cout << "StopSign initialized" << endl;
        }
        ~StopSign() {
            cout << "StopSign destroyed" << endl;
        }
};

//Κλάση για τα φανάρια.
class TrafficLight : public StaticObject {
    private:
       string state;
    public:
        TrafficLight(Position pos = {0,0}, string State="RED") : StaticObject("TrafficLight", 'T', pos), state(State) {
            cout << "TrafficLight initialized" << endl;
        }
        ~TrafficLight() {
            cout << "TrafficLight destroyed" << endl;
        }
};

//Κλάση για κινητά αντικείμενα.
class MovingObject : public Object { 
    protected:
        int speed;
        int direction;
    public:
        MovingObject(string id="", int glyph = 0, Position pos={0,0}, int Speed = 0, int Direction = 0) 
        : Object(id, glyph, pos), speed(Speed), direction(Direction) {
            cout << "MovingObject initialized" << endl;
        }
        ~MovingObject() {
            cout << "MovingObject destroyed" << endl;
        }
};

//Κλάση για αυτόνομο αυτοκίνητο.
class SelfDrivingCar : public MovingObject {
    private: 
        CameraSensor camera;
        LidarSensor lidar;
        RadarSensor radar;
    public:
        SelfDrivingCar(Position pos = {0,0}) 
        : MovingObject("SelfDrivingCar", 'C', pos), camera(pos.x, pos.y), lidar(pos.x, pos.y), radar(pos.x, pos.y) {
            cout << "Car initialized" << endl;
        }
        ~SelfDrivingCar() {
            cout << "Car destroyed" << endl;
        }
        void accelerate(){
            speed++;
        }
        void decelerate(){
            speed--;
        }
};

//κλάση για τα ποδήλατα.
class Bike : public MovingObject {
    public:
        Bike(Position pos = {0,0} ) : MovingObject("Bike", 'B', pos) {
            cout << "Bike initialized" << endl;
        }
        ~Bike() {
            cout << "Bike destroyed" << endl;
        }
};

//Βασική κλάση για τους αισθητήρες.
class Sensor{
    protected:
        Position position;
        string type;
    public:
        Sensor(string type, int x, int y)
        :type(type), x(x), y(y){
            cout << "constructoe called" << endl;
        }
        virtual ~Sensor() {
            cout << "destructor called" << endl;
        }
};

//Κλάση για τον αισθητήρα Lidar.
class LidarSensor : public Sensor{
    public:
        LidarSensor(int x, int y) : Sensor("lidar", x, y){
            cout << "lidar sensor created" << endl;
        }
        virtual ~LidarSensor() {
            cout << "lidar sensor destroyed" << endl;
        }

        vector<vector<string>> scaner(string world){  
            vector<vector<string>> area(9, vector<string>(9,"")) ;   
            for(int i = (x-4) ; i < x+4 ; i++){
                for(int j = (y-4) ; j < y+4 ; j++){
                    if (i > 0 && i <= )
                    area[i%(x-4)][j%(y-4)] = world[i][j];
                }
            }
            return area;
        }
};

//Κλάση για τον αισθητήρα Radar.
class RadarSensor : public Sensor{
    public:
        RadarSensor(int x, int y) : Sensor("radar", x, y){
            cout << "radar sensor created" << endl;
        }
        virtual ~RadarSensor() {
            cout << "radar sensor destroyed" << endl;
        }
        string scaner(string world) {
            vector<string> area(12);
            for(int i = x+1 ; i <= x+12 ; i++){
                area [i%(x+1)] = world[i][y];
            }
            return area;
        }
};


//Κλάση για την κάμερα.
class CameraSensor : public Sensor{
    public:
        CameraSensor(int x, int y) : Sensor("camera", x, y){
            cout << "camera created" << endl;
        }
        virtual ~CameraSensor() {
            cout << "camera destroyed" << endl;
        }
        string scaner(string world) {
             vector<vector<string>> area(7, vector<string>(7,""))
            for(int i = x+1 ; i <= x+7 ; i++){
                for(int j = y-3 ; j <= y+3 ; j++){
                    area[i%(x+1)][j%(y-3)] = world[i][j];
                }
            }
            return area;
        }
};

//Η βοηθιτικη συναρτηση που μπορει ο χρηστης να καλεσει απο την γραμμη εντολων.
void print_help() {
    cout << "--seed <n>                     Randome seed (default current time)" << endl;
    cout << "--dimX <n>                     World width (default 40)" << endl;
    cout << "--dimY <n>                     World height (default 40)" << endl;
    cout << "--numMovingCars <n>            Number of moving cars (default 3)" << endl;
    cout << "--numMovingBikes <n>           Number of moving bikes (default 4)" << endl;
    cout << "--numParkedCars <n>            Number of parked cars (default 7)" << endl;
    cout << "--numStopSigns <n>             Number of signs STOP (deafault 1)" << endl;
    cout << "--numTraficLights <n>          Number of trafic light (default 2)" <<  endl;
    cout << "--simulatorticks <n>           Maximun simulator ticks (default 100)" << endl;
    cout << "--gps <x1> <y1> [x2 y2  ...]   Gps target coordinates (required)" << endl;
    cout << "--help                         Showing this message" << endl;
    cout << "Usage:" << endl;
    cout << "./project --seed <n> --dimX <n> --dimY <n> --numMovingCars <n> --numMovingBikes <n> --numParkedCars <n> --numStopSigns <n> --numTraficLights <n> --simulatorticks <n> --gps <x1> <y1> [x2 y2  ...] " << endl;
}

//Χρησημοποιω δυο συναρτησεις (accelerate - decelerate) για την αλλαγη της ταχυτητας του αυτονομου αυτοκινητου 
//συμφωνα με τα "κοουτακια" που αλαζει σε καθε tick (0 για κατασταση STOPED - 1 για κατασταση HALF SPEED - 2 για κατασταση FULL SPEED)

/*int accerlerate (int &speed){
    return speed++ ;
}
int decelerate (int &speed){
    return speed-- ;
}
*/

tuple<int, int> NavigationSystem (int x, int y){
    tuple<int, int>  destination(x, y);
    return make_tuple(get<0>(destination), get<1>(destination));
}



//Αρχή της main
int main(int argc, char* argv[]){
    //βαζω τις default τιμες στις μεταβλητες και επιτα ελεγχω αν  μου εχει δωσει κατι διαφορετικο ο χρηστης στην γραμμη εντολων
    //κατα την κληση του προγραμματος, και ενημερωνω την αντιστοιχη μεταβλητη.
    int dimX = 40;
    int dimY = 40;
    int MovingCars = 3;
    int ParkedCars = 7;
    int MovingBikes = 4;
    int STOP = 1;
    int TraficLights = 2;
    int ticks = 100;

    //εκτυπωνω το βοηθητικο προς τον χρηστη αν το ζητησει.
    if (strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }

    //παιρνω τις παραμετρους απο την γραμμη εντολων που μου εχει δωσει ο χρηστης.
    int i = 1; 
    bool flag = false;
    while(i< argc){
        if (strcmp(argv[i], "--dimX")){
            dimX = stoi(argv[i+1]);
        }

        if (strcmp(argv[i], "--dimY")){
            dimY = stoi(argv[i+1]);
        }

        if (strcmp(argv[i], "--numMovingCars") == 0){
            MovingCars = stoi(argv[i+1]);
        }

        if (strcmp(argv[i], "--numMovingBikes") == 0){
            MovingBikes = stoi(argv[i+1]);
        }

        if (strcmp(argv[i], "--numParkedCars") == 0){
            ParkedCars = stoi(argv[i+1]);
        }

        if(strcmp(argv[i], "--numStopSigns") == 0){
            STOP = stoi(argv[i+1]);
        }

        if(strcmp(argv[i], "--numTraficLights") == 0){
            TraficLights = stoi(argv[i+1]);
        }

        if(strcmp(argv[i], "--simulatorticks") == 0){
            ticks = stoi(argv[i+1]);
        }

        if (strcmp(argv[i], "--gps") == 0){
            int x = stoi(argv[i+1]);
            int y = stoi(argv[i+2]);
            for(int j=i+3 ; j<argc-i ; j+=2){
                NavigationSystem(stoi(argv[j]), stoi(argv[j++]));
            }
            flag = true;
            break;
        }
        i++;
    }

    //Σε περιπτωση που δεν μου δωσει ο χρηστης συντεταγμενες στην γραμμη εντολων κατα την κληση του προγραμματος 
    //τερματιζω το προγραμμα και του εμφανιζω προειδοποιητικο μυνημα.
    if (flag == false){
        cout << "ERROR. Gps coordinates required" << endl;
        cout << "Try Again !!!" <<endl;
        return 1;
    }
    
    //διμηουργω ενα πινακα με strings για την προσομοιωση του "κοσμου"
    vector<vector<string>> world(dimX, vector<string>(dimY, ""));
    SelfDrivingCar ferrari;
    ferrari.setPosition(x, y);

    return 0;
}