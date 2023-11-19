#include <Arduino.h>
#include<Wire.h>

// PIN DEFINITIONS
#define VCCSensPin 1
#define vibPin 7
#define accPin 3
#define RFIDPin 4

// CONTROL ONLY ONE SENSOR -> !!!! change to 0 if want to use all the code !!!!
#define SINGLE_CHECK 1

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
int state = OFF;
// sensor alimentation state
int VCCSens = 0;
// shutdown timer variable
int shut_t = 0;
// control sensors cicle waiting time
int cont_t = 0;


/// accelerometer variables
const int MPU = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
#define N_READ 10
#define TRESH 70
int read[N_READ];
int prev_read[N_READ];


void setup() {
  /// accelerometer setup
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  // put your setup code here, to run once:
  pinMode(vibPin,INPUT);
  pinMode(VCCSensPin,OUTPUT);
  Serial.begin(9600);
  Serial.println("-------STARTING-------");
  shut_t = 0;
  for(int i = 0; i <N_READ; i++){
    read[i] = 0;
    prev_read[i] = 0;
  }
}



void loop() {
  state = OFF;
  if (SINGLE_CHECK) {

    Serial.println("---SINGLE CHECK---");
    check_only(accPin);

  }else{
      if(state == OFF){

      // checking if the driver is inside the car or the cra is moving
      
      //if( !(is_vibrating) ){
      //  state = delay_and_check(DELAY_VIB);
      //} else if( !(is_accelerating) ){
      //  state = delay_and_check(DELAY_ACC);
      //} else if( !(RFID_check) ){
      //  state = delay_and_check(DELAY_RFID);
      //}
      
    } else if(state == ON){
      Serial.print("ON");


      // time shutdown of the system -> if the system does not detect a child in the given time it shuts down automatically
      delay(SHUTDOWN_DELAY); 
      shut_t = shut_t + 1;
      if ( state != CHILD_DETECTED && shut_t == SHUTDOWN_TIME) {
        state = OFF;
        shut_t = 0;
      }
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
  Serial.println(" (1: )"); Serial.print(out1);
  out2 = digitalRead(vibPin);
  Serial.println(" (2: )"); Serial.print(out2);
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

  long int sum = 0;
  long int sum_var = 0;
  
  for(int i = 0; i <N_READ; i++){
    prev_read[i] = read[i];
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 14, true); // request a total of 14 registers
    AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Serial.print("Accelerometer: ");
    Serial.print("X = "); Serial.print(AcX);
    Serial.print(" | Y = "); Serial.print(AcY);
    Serial.print(" | Z = "); Serial.println(AcZ);
    read[i] = AcX;
    sum = sum + read[i];
  }

  int mean = sum/N_READ;

  for(int i = 0; i<N_READ; i++){

    sum_var = sum_var + ((read[i]-mean)*(read[i]-mean));

  }

  int var = sqrt(sum_var/(N_READ-1));
  Serial.print("variance: "); Serial.println(var);

}


int delay_and_check(int T){
  delay(T);
  if ( !(is_vibrating) && !(RFID_check) && !(is_accelerating) ) {
    return ON;
  }else{
    return OFF;
  } 
}

int check_only(int type){
  switch (type){
    case vibPin:
      return is_vibrating();
      break;
    case accPin:
      return is_accelerating();
      break;
    case RFIDPin:
      return RFID_check();
      break;
  }
}






