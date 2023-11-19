#include <Arduino.h>
#include <Wire.h>


/// SYSTEM STATE DEFINITIONS ///

#define OFF 0
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
#define START_ACC_3_SIGMA 100

/// Timer variables 
#define START_TIMER 50 //[ms]
int StarttimerSwitch = 0;
int startTimer = 0;



/// CPD DEFINITIONS ///

/// Shutdown timer [ms]
#define SHUTDOWN_TIMER 10000
int shutdownTimer = 0;

/// Control Timer [ms]
#define CONTROL_TIMER 10000
int controlTimer = 0;

/// Control sensors Pins
#define CONT_VIB_PIN 5
#define CONT_TEMP_PIN A0
#define CONT_ULTRA_PIN 6
// number of control sensors
#define N_CON 3
// output of control sensors
int controlVal[N_CON];

/// Detection sensors Pins
#define DET_PIR_1_PIN 7
#define DET_PIR_2_PIN 8
#define DET_RAD_1_PIN 9
#define DET_RAD_2_PIN 10
#define DET_CO2_PIN A0
// number of detection sensors (type)
#define N_TYPE_DET 3
// number of detection sensors (total)
#define N_DET 5

/// output array of detection sensors
int detVal[N_DET];

// weights
int weights[N_DET];

/// Temperature sensor
int Vo;
const float R1 = 100000;  //fixed resistance of the tension partition 
float logR2, R2, T;
const float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;



/// ALERT DEFINITIONS ///

/// GENERAL ///

/// Sensors array and number of sensor used
#define N_SENS 8
int sens[N_SENS] = {START_PIR_PIN,CONT_ULTRA_PIN,CONT_VIB_PIN,DET_CO2_PIN,DET_PIR_1_PIN,DET_PIR_2_PIN,DET_RAD_1_PIN,DET_RAD_2_PIN};
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
  if(con != 1){
    Serial.print("--STATE: "); Serial.print(state); Serial.println(" --");
    if(state == ON){
      con = 1;
    }
  }
  
  switch(state){
    

    // OFF STATE
    case OFF:
      if( !(start_acc_change()) && !(start_PIR_change()) ){   //if the mode is selected and if the car is not moving 
        Serial.println("No accelerometer or PIR variation detected");
        if( !(StarttimerSwitch) ){
          Serial.println("TIMER STARTED");
          StarttimerSwitch = 1; // initializating the timer
          startTimer = START_TIMER;
        }else if(StarttimerSwitch && startTimer == 0){
          state = ON;
          // variables reset
          StarttimerSwitch = 0;
          startTimer = 0;
          //start detection sensor reset
          firstAccVal = 0;
          firstPIRVal = 0;
          // shutdown timer inizialization
          shutdownTimer = SHUTDOWN_TIMER;
        }
      }else{
        Serial.println("STARTING: No variation");
        state = OFF;
        //Reset timer variables
        StarttimerSwitch = 0;
        startTimer = 0;
      }
      //timer countdown
      if(StarttimerSwitch){
        startTimer = startTimer - 1;
        delay(1);
      }
    break;


    // CPD STATE (ON STATE)
    case ON:
      //control timer countdown
      if(controlTimer == 0){
        
        controlTimer = CONTROL_TIMER;
      }
      if(shutdownTimer == 0){
        state = OFF;
      }
      


      //shutdown timer countdown
      shutdownTimer = shutdownTimer - - 1;
      // control timer countdown
      controlTimer = controlTimer - 1;
      delay(1);
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

// STARTING & CHECKING //

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

  sum = 0;
  for(i=0; i<N_ACC; i++){
    sum =  sum + pow(start_acc_val[i] - mean, 2);
  }

  dev = sqrt(sum/(N_ACC - 1));
  Serial.print("Accelerometer standard deviation: "); Serial.println(dev);
  
  if(dev >= START_ACC_3_SIGMA){
    return 1;
  }else{
    return 0;
  }
}


// CPD //

void weights_computation(void){

}

// gives out the temperature
float control_temp_read(void){
  Vo = analogRead(CONT_TEMP_PIN);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  return T = T - 273.15;
}

int control_ultrasound_read(void){
  
}