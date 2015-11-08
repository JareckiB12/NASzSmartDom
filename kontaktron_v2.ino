//Czujnik z kontaktronem - podpisywanie, MySensors lib v1.5

#include <MySigningAtsha204Soft.h>
#include <SPI.h>
#include <MySensor.h>
#include <avr/sleep.h>
//#include <Bounce2.h>

#define NUMER_NODA 4  //ZMIENIC NA ODPOWIEDNI!!!!!!!!!!!!!!!!!
#define SKETCH_NAME "Kontaktron"
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "0"

#define CHILD_ID 3
#define BUTTON_PIN 3
#define INTERRUPT_SENSOR 1  //przerwanie 1 jest digital pin 3 Arduino

MyTransportNRF24 radio;  // NRFRF24L01 radio driver
MyHwATMega328 hw; // Select AtMega328 hardware profile
MySigningAtsha204Soft signer; // Select ATSHA204A physical signing circuit
MySensor gw(radio, hw, signer);

//Bounce debouncer = Bounce();
unsigned long SLEEP_TIME = 30000;
int oldBatLevel = -1;
int oldValue = -1;

MyMessage msg(CHILD_ID, V_TRIPPED);

void setup()  
{ 
  gw.begin(NULL,NUMER_NODA);
  gw.sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER"."SKETCH_MINOR_VER);

  pinMode(BUTTON_PIN,INPUT);
  digitalWrite (BUTTON_PIN, LOW);   
//  debouncer.attach(BUTTON_PIN);
//  debouncer.interval(5);

//  Serial.println(digitalRead(BUTTON_PIN));

  pinMode(2,INPUT);
  digitalWrite (2, LOW); 

  pinMode(4,INPUT);
  digitalWrite (4, LOW); 

  pinMode(5,INPUT);
  digitalWrite (5, LOW); 

  pinMode(6,INPUT);
  digitalWrite (6, LOW); 

  pinMode(7,INPUT);
  digitalWrite (7, LOW); 

  pinMode(8,INPUT);
  digitalWrite (8, LOW); 

  gw.present(CHILD_ID, S_DOOR);  
  
}

void loop()      
{

//  debouncer.update();
//  int value = debouncer.read();
  delay(500);
  int value = digitalRead(BUTTON_PIN);
//  if (value != oldValue) {
  gw.send(msg.set(value==HIGH ? 1 : 0));
//    oldValue=value;
//  }
//  Serial.print(digitalRead(BUTTON_PIN));
//  Serial.println(" - digitalRead");
//  Serial.print(value);
//  Serial.println(" - value");
//  Serial.print(value);
//  Serial.println(" - debouncer.read");


  int batLevel = getBatteryLevel();
  if (oldBatLevel != batLevel)
  {
    gw.sendBatteryLevel(batLevel);    
    oldBatLevel = batLevel;
  }

  
  gw.sleep(INTERRUPT_SENSOR, CHANGE, SLEEP_TIME);
  

}




// Battery measure
int getBatteryLevel () 
{
  int results = (readVcc() - 2000)  / 10;   

  if (results > 100)
    results = 100;
  if (results < 0)
    results = 0;
  return results;
} // end of getBandgap

// when ADC completed, take an interrupt 
EMPTY_INTERRUPT (ADC_vect);

long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  noInterrupts ();
  // start the conversion
  ADCSRA |= _BV (ADSC) | _BV (ADIE);
  //set_sleep_mode (SLEEP_MODE_ADC);    // sleep during sample
  interrupts ();
  //sleep_mode (); 
  // reading should be done, but better make sure
  // maybe the timer interrupt fired 
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV

  return result;
}
