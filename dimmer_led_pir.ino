/***
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * 
 * DESCRIPTION
 * This sketch provides a Dimmable LED Light using PWM and based Henrik Ekblad 
 * <henrik.ekblad@gmail.com> Vera Arduino Sensor project.  
 * Developed by Bruce Lacey, inspired by Hek's MySensor's example sketches.
 * 
 * The circuit uses a MOSFET for Pulse-Wave-Modulation to dim the attached LED or LED strip.  
 * The MOSFET Gate pin is connected to Arduino pin 3 (MIRROR_LED_PIN), the MOSFET Drain pin is connected
 * to the LED negative terminal and the MOSFET Source pin is connected to ground.  
 *
 * This sketch is extensible to support more than one MOSFET/PWM dimmer per circuit.
 *
 * REVISION HISTORY
 * Version 1.0 - February 15, 2014 - Bruce Lacey
 * Version 1.1 - February 4, 2014 - Pete B - added buttons to control on/off and dim level and a motion sensor
 ***/
#define SN "Mirror LED"
#define SV "1.1"
#define NODE_ID AUTO  //change to a number to assign a specific ID

#include <SPI.h>
#include <MySensor.h>
#include <Bounce2.h>

#define MIRROR_LED_CHILD 0    //ID of the LED child
#define MOTION_CHILD 1  //ID of the motion sensor child

#define MIRROR_LED_PIN 3      // Arduino pin attached to MOSFET Gate pin
#define UP_BUTTON_PIN 8  // Arduino Digital I/O pin number for the fade up button 
#define DOWN_BUTTON_PIN 7  // Arduino Digital I/O pin number for the fade down button 
#define POWER_BUTTON_PIN 4  // Arduino Digital I/O pin number for the power button 
#define MOTION_PIN  6  // Arduino pin tied to trigger pin on the motion sensor.

#define FADE_DELAY 10  // Delay in ms for each percentage fade up/down (10ms = 1s full-range dim)
#define FADE_PERCENTAGE 10 //The percentage the fade level will be changed when a button is pressed


MySensor gw; //Don't need to define pins unless they are changing from the default

static int currentLevel = 0;  // Current dim level...
uint8_t fadeLevel = 0; //used to store the fade level when using the buttons
uint8_t upPreviousValue;
uint8_t downPreviousValue;
uint8_t powerPreviousValue;

Bounce upDebouncer = Bounce();
Bounce downDebouncer = Bounce();
Bounce powerDebouncer = Bounce();

//motion sensor
uint8_t lastMotion = 0;


unsigned long previousMillis = 0; // last time update //see http://stackoverflow.com/questions/10773425/performing-a-function-after-x-time for more details on this
unsigned long motionDelay = 10000; // interval at which to keep motion sensor trippped (milliseconds).  Used to prevent too frequent updates to Vera. 
unsigned long upPreviousMillis = 0;
unsigned long downPreviousMillis = 0; 
unsigned long buttonFadeDelay = 200; 


boolean metric = true; 

MyMessage motionMsg(MOTION_CHILD, V_TRIPPED);
MyMessage dimmerMsg(MIRROR_LED_CHILD, V_DIMMER);
//MyMessage lightMsg(LED_CHILD, V_LIGHT);  removed, updating with dimmer values only

/***
 * Dimmable LED initialization method
 */
void setup()  
{ 
  Serial.println( SN ); 

  gw.begin( incomingMessage,  NODE_ID);

  // Register the LED Dimmable Light with the gateway
  gw.present( MIRROR_LED_CHILD, S_DIMMER );
  gw.present(MOTION_CHILD, S_MOTION);
  
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo(SN, SV);
  
  metric = gw.getConfig().isMetric;
  
 
   // Setup the button
  pinMode(UP_BUTTON_PIN,INPUT);
  pinMode(DOWN_BUTTON_PIN,INPUT);
  pinMode(POWER_BUTTON_PIN,INPUT);
  
  // Activate internal pull-up
  digitalWrite(UP_BUTTON_PIN,HIGH);
  digitalWrite(DOWN_BUTTON_PIN,HIGH);
  digitalWrite(POWER_BUTTON_PIN,HIGH);
  
  // After setting up the button, setup debouncer
  upDebouncer.attach(UP_BUTTON_PIN);
  downDebouncer.attach(DOWN_BUTTON_PIN);
  powerDebouncer.attach(POWER_BUTTON_PIN);
  upDebouncer.interval(5);
  downDebouncer.interval(5);
  powerDebouncer.interval(5);
  

}

/***
 *  Dimmable LED main processing loop 
 */
void loop() 
{

  //process the LED commands from gateway
  gw.process();
 
  //motion sensor code
  unsigned long currentMillis = millis();
  
     if(currentMillis - previousMillis > motionDelay){
      uint8_t motionDetect = digitalRead(MOTION_PIN);
  
      if(motionDetect != lastMotion){
//        Serial.print("motionDetect Value: ");
//        Serial.println(motionDetect);
        gw.send(motionMsg.set(motionDetect));  // Send tripped value to gw
        
        if(motionDetect == 1){
          previousMillis = currentMillis;  //"Tripped" delay 
        }
        else{
          previousMillis = currentMillis - motionDelay + 1000; //"Not tripped" delay for 1 second to stop rapid "not tripped" and "tripped" updates to Vera
        }
  
         lastMotion = motionDetect; 
      }    
    }

  
/*
There are 3 buttons attached to the mirror.  One is an on/off button and the other two will fade up/fade down.  
The sensor will remember the last fade state and fade the lights to that level next time they are turned on.

If fade up or fade down button is pressed it should store that value into a variable and when the ON button is pressed
it will fade to that previous value.
 */ 

    upDebouncer.update();
    // Get the update value
    uint8_t upValue = upDebouncer.read();
    
    unsigned long upCurrentMillis = millis();
    
    if(upCurrentMillis - upPreviousMillis > buttonFadeDelay){
      if ((upValue == LOW) && (fadeLevel<100)) { //Because of the internal pullup resistors LOW = button is presssed
      
        fadeLevel += FADE_PERCENTAGE;
        fadeLevel = fadeLevel > 100 ? 100 : fadeLevel;
        fadeToLevel( fadeLevel );
        
      }
      upPreviousMillis = upCurrentMillis;
    }
  
    downDebouncer.update();
    // Get the update value
    uint8_t downValue = downDebouncer.read();
    
    unsigned long downCurrentMillis = millis();
    
    if(downCurrentMillis - downPreviousMillis > buttonFadeDelay){
      if ((downValue == LOW) && (fadeLevel>0)) {
        
        fadeLevel -= FADE_PERCENTAGE;
        fadeLevel = fadeLevel < 0 ? 0 : fadeLevel;
        //powerState = fadeLevel; //Remember fade level for when power button is pressed
        fadeToLevel( fadeLevel );
        
      }
      downPreviousMillis = downCurrentMillis;
    }
      
    powerDebouncer.update();
    // Get the update value
    uint8_t powerValue = powerDebouncer.read();
    
    if(powerValue != powerPreviousValue){
      if (powerValue == LOW) {
      Serial.print("Power Button Pressed. fadeLevel is ");
      Serial.println(fadeLevel);
        if (currentLevel > 0) {
          fadeToLevel( 0 );
        }
        else{
          if (fadeLevel == 0) {
            fadeToLevel(50);
            fadeLevel = 50;
          }
          else{
            fadeToLevel(fadeLevel);
          }
        }
      }
      powerPreviousValue = powerValue;
    }
  
}

void incomingMessage(const MyMessage &message) {
  if (message.type == V_LIGHT || message.type == V_DIMMER) {
    
    //  Retrieve the power or dim level from the incoming request message
    int requestedLevel = atoi( message.data );
    
    // Adjust incoming level if this is a V_LIGHT variable update [0 == off, 1 == on]
    requestedLevel *= ( message.type == V_LIGHT ? 100 : 1 );
    
    // Clip incoming level to valid range of 0 to 100
    requestedLevel = requestedLevel > 100 ? 100 : requestedLevel;
    requestedLevel = requestedLevel < 0   ? 0   : requestedLevel;
    
//    Serial.print( "Changing level to " );
//    Serial.print( requestedLevel );
//    Serial.print( ", from " ); 
//    Serial.println( currentLevel );

    fadeToLevel( requestedLevel );
    }
}

/***
 *  This method provides a graceful fade up/down effect
 */
void fadeToLevel( int toLevel ) {

//  Serial.print("toLevel Value: ");
//  Serial.println(toLevel);
//  Serial.print("currentLevel Value: ");
//  Serial.println(currentLevel);
  
  int delta = ( toLevel - currentLevel ) < 0 ? -1 : 1;
  
  while ( currentLevel != toLevel ) {
    currentLevel += delta;
    analogWrite( MIRROR_LED_PIN, (int)(currentLevel / 100. * 255) );
    delay( FADE_DELAY );
  }

   // Inform the gateway of the current DimmableLED's SwitchPower1 and LoadLevelStatus value...
  //gw.send(lightMsg.set(currentLevel > 0 ? 1 : 0));  //used to send status of light (on/off) to Vera
  gw.send( dimmerMsg.set(currentLevel) );
}
