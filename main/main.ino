#include <Arduino.h>

// PIN DEFINITIONS
#define VCCSensPin 1
#define vibPin 2
#define accPin 3
#define RFIDPin 4

// GENERAL VARIABLES DEFINITION
#define DELAY_VIB 300000 // slower if the car has start and stop
#define DELAY_ACC 300000 // accelerometer delay must be slower than the others two
#define DELAY_RFID 300000
#define SHUTDOWN_DELAY 100 // (TO BE TUNED TO BE 40 MINUTES)
#define SHUTDOWN_TIME 2400/SHUTDOWN_DELAY // NB: 0.1millisecond scale (TO BE TUNED TO BE 40 MINUTES)



// STATE DEFINITIONS
#define OFF 0
#define ON 1
#define CHILD_DETECTED 2
// system state variable
int state = 0;
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
  Serial.begin(9600);
  Serial.println("-------STARTING-------");
  shut_t = 0;
}



void loop() {
  
  if(state = OFF){

    // checking if the driver is inside the car or the cra is moving
    
    if( !(is_vibrating) ){
      state = delay_and_check(DELAY_VIB);
    } else if( !(is_accelerating) ){
      state = delay_and_check(DELAY_ACC);
    } else if( !(RFID_check) ){
      state = delay_and_check(DELAY_RFID);
    }
    
  } else if(state = ON){



    // time shutdown of the system -> if the system does not detect a child in the given time it shuts down automatically
    delay(SHUTDOWN_DELAY); 
    shut_t = shut_t + 1;
    if ( state != CHILD_DETECTED && shut_t == SHUTDOWN_TIME) {
      state = OFF;
      shut_t = 0;
    }
  }
 
}


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


int is_vibrating(void){
  int out1 = 0;
  int out2 = 0;
  out1 = digitalRead(vibPin);
  out2 = digitalRead(vibPin);
  if(out1 != out2){
    return 1;
  }else if(out1 == out2){
    return 0;
  }else{
    Serial.println("Error: vibration not detected");
  }
}


int RFID_check(void){
  
}


int is_accelerating(void){
  
}


int delay_and_check(int T){
  delay(T);
  if ( !(is_vibrating) && !(RFID_check) && !(is_accelerating) ) {
    return ON;
  }else{
    return OFF;
  }
}






