Ονοματεπώνυμο: Ευγένιος Γεωργατζάς  Αριθμός Μητρώου: sdi2400023
Ονοματεπώνυμο: Εμμανουήλ Ζέρβας Αριθμός Μητρώου: sdi2400051

Εντολή μεταγλώτησης:

g++ project.cpp -o project

Εντολή εκτέλεσης του προγράμματος:

./project gps <x1,x2> [x1 y1 ...]

--options:

--seed                      Random seed (default current time)
--dimX                      World width (default 40)
--dimY                      World height (default 40)
--numMovingCars             Number of moving cars (default 3)
--numMovingBikes            Number of moving bikes (default 4)
--numParkedCars             Number of parked cars (default 7)
--numStopSigns              Number of signs STOP (default 1)
--numTrafficLights          Number of traffic lights (default 2)
--simulationTicks           Maximum simulation ticks (default 100)
--minConfidenceThreshold    Minimum confidence threshold (default 40)
--gps <x1> <y1> [x2 y2  ...]   GPS target coordinates (required)
--help                         Showing this message
\nUsage:
./project --seed 12 --dimX 40 --dimY 40 --gps 10 20 30 15

Μεθοδολογία:

Ο κόσμος είναι βασισμένος σε μια βάσικη κλάση Grid.Τα αντικειμενα ειναι βασισμενα σε μια κλασση Object η οποια υποδιαιρειται 
σε δυο μερη movingobject-staticobject.Με την σειρα τους η movingobject υποδιαιρειται στα κινουμενα αυτοκινητα, στα ποδηλατα καθως 
και στο ιδιο το αυτοκινουμενο οχημα, ενω η staticobject υποδιαιρειται στα φαναρια,στα παρκαρισμενα αυτοκινητα και στα σηματα STOP.
Επιπλέον υπάρχει το struct Position το οποίο χρεισημοποιειται για την αποθήκευση των συντεταγμένων των αντικειμένων στον κόσμο.
To Struct ID το οποίο αποθηκέυει την ταυτότητα των αντικειμένων και τέλος το Struct SensorReading το οποίο απόθηκέυι της μετρήσεις 
των αισθητηρλών.Για πολλα τυχαια πραγματα στο προγραμμα (θεσεις αντικειμενων, κατευθηνση οχηματος κ.α.) εχουμε χρησημοποιησει την συναρτηση rand().  

