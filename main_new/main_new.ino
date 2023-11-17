#include <Arduino.h>

// PIN DEFINITIONS //
#define VCCSensPin 1
// I/O sensors pins
#define motorVibPin 2
#define accPin 3
#define ultraSeatPin 4
// Detection sensor pins
#define PIRPin 5
#define radPin 6
#define CO2Pin 7
// control sensors pins
#define tempPin 8
#define ultraWindPin 9
#define radarVibPin 10
// number of sensors for each task
#define NIO 3
#define NDET 3
#define NCON 3
#define NSENS NIO+NDET+NCON
////////////////////////////////// led pins for logic configuration
#define OFFPin 7
#define checkPin 8
#define ONPin 9

// PIN ARRAYS //
int acc[NIO] = {motorVibPin,accPin,ultraSeatPin};
int det[NDET] = {PIRPin,radPin,CO2Pin};
int con[NCON] = {tempPin,ultraWindPin,radarVibPin};
int sens[NSENS] = {motorVibPin,accPin,ultraSeatPin,PIRPin,radPin,CO2Pin,tempPin,ultraWindPin,radarVibPin};

// STATE DEFINITION //
#define OFF 0
#define ON 1
#define CHECK 2
#define CHILD_DETECTED 3

// STATE ARRAY //
int state_array[4] = {OFF,ON,CHECK,CHILD_DETECTED}; //may be not used

// SYSTEM MAIN VARIABLE //
int state = OFF;

// WEiGHTS DEFINITION //
#define LOWERED_WINDOW 0
#define CLOSED_WINDOW 1 
#define T_CONST 1 //to be tuned

// DELAYS //
// delay for CHECK state
#define DELAY_VIB 5000 // slower if the car has start and stop
#define DELAY_ACC 5000 // accelerometer delay must be slower than the others two
#define DELAY_RFID 5000
// delay for the time treshold
#define SHUTDOWN_DELAY 100 // (TO BE TUNED TO BE 40 MINUTES)
#define SHUTDOWN_TIME 4400/SHUTDOWN_DELAY // NB: 0.1millisecond scale (TO BE TUNED TO BE 40 MINUTES)
//delay for before and after measurement
#define OLD_NEW_DELAY 10

// SHUTDOWN TIMER VARIABLE
int shut_t = 0;

// CONTROL SENSOR TIMER
int control_t = 0;

// TRESHOLD FOR DETECTION //
#define TRESHOLD 1


// SENSOR ALIMENTATION STATE
int VCCSens = 0;

// CPD VARIABLES
// measurments results of the detection sensors
int det_res[3] = {0,0,0};
// weights of the sensors
int det_weights[3] = {0,0,0};
// sum of the measurement
int sum = 0;

//check variable for the activation of the system (if possible replace with a pointer)
int check[NIO]; 








void setup() {

  // opening the serial monitoring
  Serial.begin(115200);
  Serial.println("-------STARTING-------");

  // SENSORS INITIALIZATION
  for(int i=0; i<NSENS; i++){
    pinMode(sens[i],INPUT);
    Serial.print("INPUT: "); Serial.println(i);
  }

  //INITIALIZATION OF ARRAYS
  for(int i = 0; i<NIO; i++){
    check[i] = 0;
    Serial.print("check("); Serial.print(i); Serial.print(") : "); Serial.println(check[i]);
  }

  // ALIMENTATION INITIALAZATION
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

  if(state == OFF){

    // controls if something has turned off
    check_function();
    // if something has turned off 


  }else if(state == CHECK){



  }else if(state == ON){



  }else if(state == CHILD_DETECTED){



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

///////////////////////////////////////////////////////////////////
void check_function(void){

    int old[NIO];
    int current[NIO];

    for(int i=0; i<NIO; i++){
        old[i] = 0;
        current[i] = 0;
    }

    old[0] = is_vibrating();
    old[1] = is_accelerating();
    old[2] = is_moving();

    delay(OLD_NEW_DELAY);

    current[0] = is_vibrating();
    current[1] = is_accelerating();
    current[2] = is_moving();

    //for each sensor if the new measured value it's different from the previous measured return 1 else return 0
    for(int i=0; i<NIO; i++){
        if(old != current){
            check[i] = 1;
        }else{
            check[i] = 0;
        }
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
int is_moving(void){
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
