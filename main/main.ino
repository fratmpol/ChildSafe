#include <Arduino.h>

// PIN DEFINITIONS
#define VCCSensPin 1
#define vibPin 3
#define accPin 4
#define RFIDPin 5
#define PIRPin 10
#define radPin 11
#define CO2Pin 12
#define tempPin 13
#define ultraPin 14
#define N 8
////////////////////////////////// led pins for logic configuration
#define OFFPin 7
#define checkPin 8
#define ONPin 9

int pins[N] = {vibPin,accPin,RFIDPin,ultraPin,tempPin,PIRPin,radPin,CO2Pin}; /// think about using the arrays

// WEUGHTS DEFINITION
#define LOWERED_WINDOW 0
#define CLOSED_WINDOW 1

// GENERAL VARIABLES DEFINITION //
// delay variables for state sensors
#define DELAY_VIB 5000 // slower if the car has start and stop
#define DELAY_ACC 5000 // accelerometer delay must be slower than the others two
#define DELAY_RFID 5000
// delay variables for the time treshold
#define SHUTDOWN_DELAY 100 // (TO BE TUNED TO BE 40 MINUTES)
#define SHUTDOWN_TIME 4400/SHUTDOWN_DELAY // NB: 0.1millisecond scale (TO BE TUNED TO BE 40 MINUTES)

#define TRESHOLD 1



// STATE DEFINITIONS
#define OFF 0
#define ON 1
#define CHILD_DETECTED 2
#define CHECK 3  /////[REMOVE IN FINAL CODE]///// variable for the check state of the system before the powering on
// system state variable
int state = OFF;
// sensor alimentation state
int VCCSens = 0;

// CPD VARIABLES
// shutdown timer variable
int shut_t = 0;
// measurments results of the detection sensors
int det_res[3] = {0,0,0};
// weights of the sensors
int det_weights[3] = {0,0,0};
// control sensors timer
int control_t = 0;
// sum of the measurement
int sum = 0;



void setup() {
  // opening the serial monitoring
  Serial.begin(115200);
  Serial.println("-------STARTING-------");
  // sensors initialization
  for(int i=0; i<8; i++){
    pinMode(pins[i],INPUT);
    Serial.println(i);
  }
  pinMode(VCCSensPin,OUTPUT);
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

    if(control_t==0){
      det_weights[0] = PIR_weight();
      det_weights[1] = radar_weight();
      det_weights[2] = CO2_weight();
      control_t = 10;
    }

    det_res[0] = PIR_check();
    det_res[1] = radar_check();
    det_res[2]= CO2_check();

    for(int i=0; i<3; i++){
      sum = sum + det_weights[i]*det_res[i];
    }
    if(sum >= TRESHOLD){
      // ALERT SYSTEM
      state = CHILD_DETECTED;
    } else {
      sum = 0;
    }

    // time shutdown of the system -> if the system does not detect a child in the given time it shuts down automatically
    shut_t++;
    Serial.print("time: ");  /////[REMOVE IN FINAL CODE]/////
    Serial.println(shut_t);  /////[REMOVE IN FINAL CODE]/////
    // time counting for the redefinition of the weights of the mearurements
    if ( state != CHILD_DETECTED && shut_t >= SHUTDOWN_TIME) {
      state = OFF;
      shut_t = 0;
    }
    //control sensor time constant reduction at the end of the cicle
    control_t--;
  }

  delay(SHUTDOWN_DELAY); //main delay of the system needed to operate the time resolution of some cicles (may be removed in future)
  delay(SHUTDOWN_DELAY*10); /////[REMOVE IN FINAL CODE]///// delay introduced for logic monitoring (debugging) [REMOVE IN FINAL CODE]
  led_state(state);  /////[REMOVE IN FINAL CODE]///// led state function to illuminate the leds
 
}



///// power on/ power off functions /////

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

///// Detection of the CHILD functions /////

int PIR_check(void){
  int PIR_state = 0;
  if(PIR_state){
    return 1;
  }else if(!(PIR_state)){
    return 0;
  } else {
    Serial.println("Error: PIR not working");
  }
}

int radar_check(void){
  int radar_state = 0;
  if(radar_state){
    return 1;
  }else if(!(radar_state)){
    return 0;
  } else {
    Serial.println("Error: radar not working");
  }
}

int CO2_check(void){
  int CO2_state = 0;
  if(CO2_state){
    return 1;
  }else if(!(CO2_state)){
    return 0;
  } else {
    Serial.println("Error: CO2 sensor not working");
  }
}

///// Control of detection sensors /////

//return 1 if windows down and 0 if windows up
int ultrasound_check(void){
  int ultra_state = 0;
  if(ultra_state){
    return 1;
  }else if(!(ultra_state)){
    return 0;
  } else {
    Serial.println("Error: ultrasound not working");
  }
}

int temperature_check(void){
  int temp_state = 0;
  if(temp_state){
    return 1;
  }else if(!(temp_state)){
    return 0;
  } else {
    Serial.println("Error: temperature sensor not working");
  }
}

int PIR_weight(void){
  int temp_value = temperature_check();
  return temp_value; // NEED TO ASSIGN A FUNCTION FOR THE TEMPERATURE WEIGHT VALUE
}

int radar_weight(void){
  if(ultrasound_check()){
    return CLOSED_WINDOW;
  }else{
    return LOWERED_WINDOW;
  }
}

int CO2_weight(void){
  if(ultrasound_check()){
    return CLOSED_WINDOW;
  }else{
    return LOWERED_WINDOW;
  }
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

