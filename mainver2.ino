#include <Arduino.h>
#include <Wire.h>


/// SYSTEM STATE DEFINITIONS ///

#define OFF 0
#define CHECK 3
#define ON 1
#define ALERT 2
// state system variable
int state = OFF;



/// STARTING DEFINITIONS ///

/// Alimentation
#define VCC 0

/// PIR
// digital Pin 
#define START_PIR_PIN 4
//data array
#define N_PIR 100    // (estimated to be a good data for the readouts)
int start_PIR_val[N_PIR];
// variable to specify if its the first array of datas
bool firstPIRVal = 1;

/// Accelerometer
const int MPU = 0x68; // I2C address of the MPU-6050
int16_t AcX;
// datas collected by the accelerometer
#define N_ACC 10
int start_acc_val[N_ACC];
// variable to specify if its the first array of datas
bool firstAccVal = 1;
// 3 sigma of the standard deviation of the accelerometer
#define START_ACC_3_SIGMA 50



/// CHECKING DEFINITIONS ///

/// Time delays before starting [s]
#define START_PIR_DELAY 5
#define START_ACC_DELAY 5
int check_delay = 0;
// sensor that has activated the check state
#define ACC 102
#define PIR 101
#define GENERAL 100
int checkSens = 0;


/// CPD DEFINITIONS ///

/// CONTROL DEFINITIONS ///

/// ALERT DEFINITIONS ///

/// GENERAL ///

/// Sensors array and number of sensor used
#define N_SENS 1
int sens[N_SENS] = {START_PIR_PIN};
int i = 0;
int j = 0;

int con = 0;






void setup(){

  // Opening Serial monitoring
  Serial.begin(115200);
  // sensors initialization
  for(i=0; i<N_SENS; i++){
    pinMode(sens[i],INPUT);
    Serial.print("INPUT: "); Serial.println(i);
  }
  // set the alimentation pin to output
  pinMode(VCC,OUTPUT);
  // accelerometer initialization
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

}



void loop(){
  if(con!=1){
    Serial.print("--- STATE: "); Serial.print(state); Serial.println("---");
    if(state == ON){
      con = 1;
    }
  }
  

  switch(state){
    
    // OFF STATE
    case OFF:
      checkSens = start_check_change_2(CHECK,GENERAL);
    break;

    // CHECKING STATE
    case CHECK:
      //selects the delay based on the sensor deactivated
      if(check_delay == 0){
        Serial.println("--WAITING--");
        check_delay = 5;
      }
      //countdown of the delay
      check_delay = check_delay - 1;
      Serial.print("Delay"); Serial.println(check_delay);
      delay(1000);
      //checking for the start of the system
      if(check_delay == 0){
        checkSens = start_check_change_2(ON,GENERAL);
      }
    break;

    // CPD STATE (ON STATE)
    case ON:

    break;

    // ALERT STATE 
    case ALERT:

    break;
  
    default:
      Serial.println("Error: state not defined or defined incorrectly");
    break;

  }

}







/// FUNCTIONS ///



// STARTING & CHECKING//

// returns 0 if nothing has changed, 1 if PIR has changed and 2 if accelerometer has changed (LOGIC)
int start_check_change(int retState,int mode){
  if((mode == GENERAL || mode == ACC) && !(start_acc_change())){   //if the mode is selected and if the car is not moving 
    Serial.println("STARTING: Accelerometer variation");
    state = retState;
    return ACC;
  }else if((mode == GENERAL || mode == PIR) && !(start_PIR_change())){  //if it don't detect movement inside the cabinet
    Serial.println("STARTING: PIR variation");
    state = retState;
    return PIR;
  }else{
    Serial.println("STARTING: No variation");
    state = OFF;
    return GENERAL;
  }
}


// variation of start_check_change
int start_check_change_2(int retState,int mode){
  if(!(start_PIR_change()) && !(start_acc_change())){   //if the mode is selected and if the car is not moving 
    Serial.println("--> Nothing moving and Car still");
    state = retState;
    return GENERAL;
  }else{
    Serial.println("STARTING: No variation");
    state = OFF;
    return GENERAL;
  }
}



// returns 0 if PIR changes from movement to not movement (TESTED)
int start_PIR_change(void){

  int sum = 0;
  
  if(firstPIRVal){

    for(i = 0; i<N_PIR; i++){
      start_PIR_val[i] = digitalRead(START_PIR_PIN);
      sum = sum + start_PIR_val[i];
    }

    firstPIRVal = 0;

  }else{

    for(i=1; i<N_PIR; i++){
      start_PIR_val[i-1] = start_PIR_val[i];
      sum = sum + start_PIR_val[i-1];
    }
    start_PIR_val[N_PIR-1] = digitalRead(START_PIR_PIN);
    sum = sum + start_PIR_val[N_PIR-1];

  }

  Serial.print("Sum of the PIR readouts: "); Serial.println(sum);

  if(sum==0){   // when nothing is moving for some time it returns 0
    return 0;
  }else{
    return 1;
  }



}



// returns 1 if standard deviation of accelerometer changes TESTED
int start_acc_change(void){

  float sum = 0;
  Serial.print("Sum: "); Serial.println(sum); 
  // mean of the N_ACC datas
  long int mean = 0;
  // variance of the N_ACC datas
  long int dev = 0;

  if(firstAccVal){

    for (i = 0; i < N_ACC ; i++){
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 14, true); // request a total of 14 registers
    AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    start_acc_val[i] = AcX/10;
    sum = sum + start_acc_val[i];
    }

    firstAccVal = 0;

  }else{

    for(i=1; i<N_ACC; i++){
      start_acc_val[i-1] = start_acc_val[i];
      sum = sum + start_acc_val[i-1];
    }
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 14, true); // request a total of 14 registers
    start_acc_val[N_ACC-1] = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    sum = sum + start_acc_val[N_ACC-1];

  }

  mean = sum/N_ACC;
  Serial.print("Mean: "); Serial.println(mean); 

  sum = 0;
  for(i=0; i<N_ACC; i++){
    sum =  sum + pow(start_acc_val[i] - mean, 2);
  }

  Serial.print("Variance: "); Serial.println(sum);
  dev = sqrt(sum/(N_ACC - 1));
  Serial.print("Accelerometer standard deviation: "); Serial.println(dev);
  
  if(dev >= START_ACC_3_SIGMA){
    return 1;
  }else{
    return 0;
  }
}



// delay selection function 
int delay_select(int sens){
  if(sens == ACC){
    return START_ACC_DELAY;
  }else if(sens == PIR){
    return START_PIR_DELAY;
  }else{
    Serial.println("Error: selection of the delay not specified correctly");
    return 0;
  }
}