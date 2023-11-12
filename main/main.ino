#include <Arduino.h>

// PIN DEFINITIONS
#define VCCSensPin 1
#define vibPin 3
#define accPin 4
#define RFIDPin 5
////////////////////////////////// led pins for logic configuration
#define OFFPin 7
#define checkPin 8
#define ONPin 9

// GENERAL VARIABLES DEFINITION
#define DELAY_VIB 5000 // slower if the car has start and stop
#define DELAY_ACC 5000 // accelerometer delay must be slower than the others two
#define DELAY_RFID 5000
#define SHUTDOWN_DELAY 100 // (TO BE TUNED TO BE 40 MINUTES)
#define SHUTDOWN_TIME 4400/SHUTDOWN_DELAY // NB: 0.1millisecond scale (TO BE TUNED TO BE 40 MINUTES)



// STATE DEFINITIONS
#define OFF 0
#define ON 1
#define CHILD_DETECTED 2
#define CHECK 3
// system state variable
int state = OFF;
// sensor alimentation state
int VCCSens = 0;
// shutdown timer variable
int shut_t = 0;
// control sensors cicle waiting time
int cont_t = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(vibPin,INPUT);
  pinMode(VCCSensPin,OUTPUT);
  Serial.begin(115200);
  Serial.println("-------STARTING-------");
  shut_t = 0;
  state = OFF;
}



void loop() {
  
  if(state == OFF){
    // checking if the driver is inside the car or the car is moving
    
    if( !(is_vibrating()) ){
      state = delay_and_check(DELAY_VIB,vibPin);
    } else if( !(is_accelerating()) ){
      state = delay_and_check(DELAY_ACC,accPin);
    } else if( !(RFID_check()) ){
      state = delay_and_check(DELAY_RFID,RFIDPin);
    }
    
  } else if(state == ON){


    // time shutdown of the system -> if the system does not detect a child in the given time it shuts down automatically
    delay(SHUTDOWN_DELAY); 
    shut_t = shut_t + 1;
    Serial.print("time: ");
    Serial.println(shut_t);
    if ( state != CHILD_DETECTED && shut_t >= SHUTDOWN_TIME) {
      state = OFF;
      shut_t = 0;
    }
  }
  
  delay(SHUTDOWN_DELAY*10); /////[REMOVE IN FINAL CODE]///// delay introduced for logic monitoring (debugging) [REMOVE IN FINAL CODE]
  led_state(state);  /////[REMOVE IN FINAL CODE]///// led state function to illuminate the leds
 
}


//switch the state of the system
void system_switch(int sswitch){
  if(sswitch = ON){
    state = ON;
    // turning off all the sensors supply
    digitalWrite(VCCSensPin,HIGH);
  }else if(sswitch = OFF){
    state = OFF;
    //turn off all the sensor supply
    digitalWrite(VCCSensPin,LOW);
  }else{
    Serial.println("Error: system not switched");
  }
}


// check for the vibration of the motor
int is_vibrating(void){
  if (digitalRead(vibPin)==LOW){
    Serial.println("vib -> OFF");
    return 0;
  }else if(digitalRead(vibPin==HIGH)){
    Serial.println("vib -> ON");
    return 1;
  }else{
    Serial.println("Error: vibration not detected");
  }
}


// check for the RFID inside the car
int RFID_check(void){
  if (digitalRead(RFIDPin)==LOW){
    Serial.println("REFID -> OFF");
    return 0;
  }else if(digitalRead(RFIDPin==HIGH)){
    Serial.println("RFID -> ON");
    return 1;
  }else{
    Serial.println("Error: RFID not detected");
  }
}


// check for  the acceleration of the car
int is_accelerating(void){
  if (digitalRead(accPin)==LOW){
    Serial.println("acc -> OFF");
    return 0;
  }else if(digitalRead(accPin==HIGH)){
    Serial.println("acc -> ON");
    return 1;
  }else{
    Serial.println("Error: acceleration not detected");
  }
}


int delay_and_check(int T, int type){
  led_state(CHECK);  /////[REMOVE IN FINAL CODE]///// test led for check state
  delay(T);
  Serial.println("--- CONTROL CHECK ---");
  if ( !(is_vibrating()) || !(RFID_check()) || !(is_accelerating()) ) {
    switch (type) {
      case vibPin:
        if ( !(is_vibrating()) ){
          Serial.println("state switched to ON");
          return ON;
        }
      case accPin:
        if ( !(is_accelerating()) ){
          Serial.println("state switched to ON");
          return ON;
        }
      case RFIDPin:
        if ( !(RFID_check()) ){
          Serial.println("state switched to ON");
          return ON;
        }
    }
  }

  Serial.println("state switched to OFF");
  return OFF;
  
}



//////////// LED light state switcher //////////

void led_state(int ledPin){
  if( ledPin==ON ){
    digitalWrite(checkPin,LOW);      
    digitalWrite(ONPin,HIGH);
    digitalWrite(OFFPin,LOW);
    Serial.println("state ON");
  }else if( ledPin == OFF ){
    digitalWrite(checkPin,LOW);      
    digitalWrite(ONPin,LOW);
    digitalWrite(OFFPin,HIGH);
    Serial.println("state OFF");
  }else if( ledPin == CHECK ){
    digitalWrite(checkPin,HIGH);      
    digitalWrite(ONPin,LOW);
    digitalWrite(OFFPin,LOW);
    Serial.println("state CHECKING");
  }else{
    Serial.println("Error in state decision");
  }
}








