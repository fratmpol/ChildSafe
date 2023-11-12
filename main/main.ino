#include <Arduino.h>

// PIN DEFINITIONS
#define VCCSensPin 1
#define vibPin 2
#define accPin 3

// GENERAL VARIABLES DEFINITION
#define DELAY_VIB 300000
#define DELAY_ACC 300000
#define DELAY_RFID 300000


// STATE DEFINITIONS
#define OFF 0
#define ON 1
// system state variable
int state = 0;
// sensor alimentation state
int VCCSens = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(vibPin,INPUT);
  pinMode(VCCSensPin,OUTPUT);
  Serial.begin(9600);
  Serial.println("-------STARTING-------");
}

void loop() {
  
  if(state = OFF){
    
    if( !(is_vibrating) ){
      state = delay_and_check(DELAY_VIB);
    }

    if( !(is_accelerating) ){
      state = delay_and_check(DELAY_ACC);
    }

    if( !(RFID_check) ){
      state = delay_and_check(DELAY_RFID);
    }
    
  }

  
  if(state = ON){
    
  }
 
}


void system_switch(int sswitch){
  if(sswitch = ON){
    state = ON;
    digitalWrite(VCCSensPin,HIGH);
  }else if(sswitch = OFF){
    state = OFF;
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

