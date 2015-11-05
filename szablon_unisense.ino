//Szablon

#include <MySigningAtsha204Soft.h>
#include <SPI.h>
#include <MySensor.h>  

#define NUMER_NODA 1  //ZMIENIC NA ODPOWIEDNI!!!!!!!!!!!!!!!!!
#define SKETCH_NAME ""
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "0"

unsigned long SLEEP_TIME = 30000;

MyTransportNRF24 radio;  // NRFRF24L01 radio driver
MyHwATMega328 hw; // Select AtMega328 hardware profile
MySigningAtsha204Soft signer; // Select ATSHA204A physical signing circuit
MySensor gw(radio, hw, signer);


void setup()  
{ 
  gw.begin(NULL,NUMER_NODA);

  
}

void loop()      
{  

  
  int batLevel = getBatteryLevel();
    Serial.print("Bateria: ");
    Serial.println(batLevel);
    gw.sendBatteryLevel(batLevel);    


   
  gw.sleep(SLEEP_TIME); //sleep a bit
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
