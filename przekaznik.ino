//Przekaznik z podpisem MySensors lib v1.5

#include <MySigningAtsha204Soft.h>
#include <SPI.h>
#include <MySensor.h>  

#define NUMER_NODA 3  //ZMIENIC NA ODPOWIEDNI!!!!!!!!!!!!!!!!!
#define SKETCH_NAME "Przekaznik x 1"
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "0"

#define RELAY_1  3  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)
#define NUMBER_OF_RELAYS 1 // Total number of attached relays
#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay

unsigned long SLEEP_TIME = 30000;

MyTransportNRF24 radio;  // NRFRF24L01 radio driver
MyHwATMega328 hw; // Select AtMega328 hardware profile
MySigningAtsha204Soft signer; // Select ATSHA204A physical signing circuit
MySensor gw(radio, hw, signer);

MyMessage msg(1,V_LIGHT);


void setup()  
{ 
  // Initialize library and add callback for incoming messages
  gw.begin(incomingMessage, NUMER_NODA, true);
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Relay", "1.0");

  // Fetch relay status
  for (int sensor=1, pin=RELAY_1; sensor<=NUMBER_OF_RELAYS;sensor++, pin++) {
    // Register all sensors to gw (they will be created as child devices)
    gw.present(sensor, S_LIGHT);
    // Then set relay pins in output mode
    pinMode(pin, OUTPUT);   
    // Set relay to last known state (using eeprom storage) 
    boolean savedState = gw.loadState(sensor);
    digitalWrite(pin, savedState?RELAY_ON:RELAY_OFF);
    gw.send(msg.set(savedState? 1 : 0));
  }
}

void loop()      
{
  // Alway process incoming messages whenever possible
  gw.process();
}


void incomingMessage(const MyMessage &message)
{
  if (message.type==V_LIGHT) {
     boolean relayState = message.getBool(); 
    digitalWrite(message.sensor-1+RELAY_1, relayState?RELAY_ON:RELAY_OFF);
     gw.saveState(message.sensor, relayState);
    gw.send(msg.set(relayState? 1 : 0));
  }
}
