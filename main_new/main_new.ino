#include <Arduino.h>
#include <Wire.h>



// When i wrote this code only God and I knew how it worked
// Now only God knows
//



///// SYSTEM STATE DEFINITIONS /////

// system states
#define OFF 0
#define ON 1
#define ALERT 2
#define SLEEP 3

// state system variable
int state = SLEEP;





///// STARTING DEFINITIONS /////


/// Alimentation ///
#define VCC 0


/// Ultrasound ///
// digital Pin 
#define START_ULTRA_TRIG_PIN 4
#define START_ULTRA_ECHO_PIN 5

// data array
#define N_ULTRA 10   // (estimated to be a good data for the readouts)
float startUltraVal[N_ULTRA];

// variable to specify if its the first array of datas
bool firstUltraVal = 1;

// standard deviation treshold
#define START_ULTRA_3_SIGMA 25   // (estimated after some experiments in which the car was shaked)


/// Accelerometer ///
// I2C address of the MPU-6050
const int MPU = 0x68; 
int16_t AcX;

// datas collected by the accelerometer
#define N_ACC 10
int start_acc_val[N_ACC];   // (estimated to be a good data for the readouts)

// variable to specify if its the first array of datas
bool firstAccVal = 1;

// 3 sigma of the standard deviation of the accelerometer
#define START_ACC_3_SIGMA 150   // (estimated after some experiments in which the car travelled on road)

/// Timer variables 
#define START_TIMER 900 //[ms] (~10 ms)
int startTimerSwitch = 0;
int startTimer = 0;





///// CPD DEFINITIONS /////


/// Shutdown timer [ms] ///
#define SHUTDOWN_TIMER 1000
int shutdownTimer = 0;


/// Control Timer [ms] ///
#define CONTROL_TIMER 10 //integer fraction of the shutdown timer
int controlTimer = 0;


/// Control sensors Pins ///
#define CONT_VIB_PIN 6
#define CONT_TEMP_PIN A0
#define CONT_ULTRA_TRIG_PIN 6
#define CONT_ULTRA_ECHO_PIN 7


/// Detection sensors Pins ///
#define DET_PIR_1_PIN 10
#define DET_PIR_2_PIN 11
#define DET_RAD_1_PIN 12
#define DET_RAD_2_PIN 13
#define DET_CO2_PIN A1

// sensor types
#define N_DET 3


/// Detection results ///
int det[N_DET];   // {PIR,RAD,CO2}


/// Temperature sensor ///
int Vo;
const float R1 = 100000;  //fixed resistance of the tension partition TO BE TUNED 
float logR2, R2, T;
const float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
//temperature treshold
#define TRESH_TEMP 37   // (centigrade degrees)


/// Ultrasound ///

// standard distance from the window
#define WINDOW_DISTANCE 10 // [cm]

// window weights
#define OPENED_WINDOW 0
#define CLOSED_WINDOW 1


/// CO2 ///

// check weight
int checkWindows = 1;
int windows = 1;

// treshold
#define CO2_TRESHOLD 9   // determined experimentally
int CO2Start = 0;


/// Accelerometer///

// treshold for vibrations 
#define DET_ACC_TRESHOLD 15

/// Radar and PIR
// delay between the two radars in order to not have interfeherence
#define INTERFEHERENCE_DELAY 10





///// ALERT DEFINITIONS /////

/// Pin definitions ///
#define BUZZER_PIN 9
#define BUTTON_PIN 2

/// Button variables ///
#define N_BUTTON 10

//data array of the button
int buttonVal[N_BUTTON];

//buzzer frequency
#define FREQUENCY 200   // can range from 0 to 255

// timer after detection
int time = 0;





/// GENERAL ///


/// Sensors array and number of sensor used ///
#define N_SENS 9
int sens[N_SENS] = {START_ULTRA_ECHO_PIN,CONT_ULTRA_ECHO_PIN,CONT_VIB_PIN,DET_CO2_PIN,DET_PIR_1_PIN,DET_PIR_2_PIN,DET_RAD_1_PIN,DET_RAD_2_PIN,BUTTON_PIN};

// counter variable
int i = 0;










void setup(){

  // Opening Serial monitoring
  Serial.begin(115200);
  Serial.println("----STARTING----");

  // sensors initialization for input pins
  for(i=0; i<N_SENS; i++){
      pinMode(sens[i],INPUT);
      // Serial.print("INPUT: "); Serial.println(i);
  }

  // set the alimentation pin to output
  pinMode(VCC,OUTPUT);

  //set the trigger pin to output
  pinMode(CONT_ULTRA_TRIG_PIN,OUTPUT);
  pinMode(START_ULTRA_TRIG_PIN,OUTPUT);

  //set the buzzer pin to output
  pinMode(BUZZER_PIN,OUTPUT);

  // accelerometer initialization
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  // button values initialization
  for(i=0;i<N_BUTTON;i++){
    buttonVal[i] = 0;
  }

}



void loop(){
  
  Serial.print("--STATE: "); Serial.print(state); Serial.println(" --");
  
  switch(state){
    


    //////// OFF STATE ////////

    case OFF:


      if(!(start_acc_change(START_ACC_3_SIGMA)) && !(start_ultra_change()) ){   //if no seat variation and if the car is not moving 
        
        Serial.println("No accelerometer or Ultra variation detected");
        

        //if the timer is not started start the timer
        if( !(startTimerSwitch) ){

          Serial.println("TIMER STARTED");

          startTimerSwitch = 1;
          //timer initialization
          startTimer = START_TIMER;

        }else if(startTimerSwitch && startTimer == 0){

          Serial.println("TIMER ENDED");

          //go to the ON state
          state = ON;

          // variables reset for OFF state
          startTimerSwitch = 0;
          startTimer = 0;

          //start detection sensor reset
          firstAccVal = 1;
          firstUltraVal = 1;

          // shutdown timer inizialization
          shutdownTimer = SHUTDOWN_TIMER;

          //window check initialization
          checkWindows = 1;

        }


      }else{

        Serial.println("----> RESTARTING & STOP: VARIATION DETECTED");

        //state reset
        state = OFF;

        //Reset timer variables
        startTimerSwitch = 0;
        startTimer = 0;

      }


      //timer countdown
      if(startTimerSwitch){

        Serial.print("start timer: "); Serial.println(startTimer);

        startTimer = startTimer - 1;
        delay(1);
      }


    break;



    //////// CPD STATE (ON STATE) ////////

    case ON:

      // do the detection until the car is not moving or the driver has not re-entered
      if( !(start_acc_change(START_ACC_3_SIGMA)) && !(start_ultra_change()) ){
        

        /// CO2 DETECTION CONTROL AND SETUP ///

        if(checkWindows){

          // check if the windows are closed or opened
          windows = control_ultrasound_read();
          checkWindows = 0;
          CO2Start = analogRead(DET_CO2_PIN);

        }


        /// CO2 DETECTION ///

        if(windows==CLOSED_WINDOW){

          det[2] = CO2_detection();

        }


        /// RADAR DETECTION ///

        if( !(start_acc_change(DET_ACC_TRESHOLD)) ){   //do not use the radar if the car is vibrating
          
          int rad_val1 = digitalRead(DET_RAD_1_PIN);
          Serial.print("radar val1: "); Serial.println(rad_val1);
          
          delay(INTERFEHERENCE_DELAY);
          
          int rad_val2 = digitalRead(DET_RAD_2_PIN);
          Serial.print("radar val2: "); Serial.println(rad_val2);
          
          if(rad_val1 || rad_val2){   //if one of the two radar detect something a child is present inside the car

            Serial.println("------RAD DETECTION------");
            delay(2000);
            det[1] = 0; 

          }else{

            det[1] = 0;

          }

        }


        /// PIR DETECTION ///

        if(control_temp_read() <= TRESH_TEMP){   // control if the temperature is too high for the PIR to operate

          int PIR_val1 = digitalRead(DET_PIR_1_PIN);
          Serial.print("PIR val1: "); Serial.println(PIR_val1);

          delay(INTERFEHERENCE_DELAY);

          int PIR_val2 = digitalRead(DET_PIR_2_PIN);
          Serial.print("PIR val2: "); Serial.println(PIR_val2);

          if(PIR_val1 || PIR_val2){

            Serial.println("------PIR DETECTION------");
            delay(2000);
            det[0] = 0; 
          
          }else{

            det[0] = 0;

          }

        }


        //// DETECTION OF THE CHILDREN ////

        for(i=0; i<N_DET; i++){

          if(det[i]){
            state = ALERT;
          }

        }


      }else{

        state = OFF;
        
        //reset of CPD switches
        Serial.println("---EXIT_CPD: SOMEONE ENTERED---");
        checkWindows = 1;

      }

      

      //shutdown timer countdown
      shutdownTimer = shutdownTimer-1;

      Serial.print("shutdown timer: "); Serial.println(shutdownTimer);
      delay(1);

      //shutdown 
      if(shutdownTimer == 0){

        state = SLEEP;

        Serial.println("---WENT TO SLEEP---");
        firstAccVal = 1;

        //reset of CPD switches
        checkWindows = 1;

      }


    break;



    //////// ALERT STATE ////////

    case ALERT:


      delay(100);
      time ++ ;
      Serial.print("alert timer: "); Serial.println(time);

      analogWrite(BUZZER_PIN,FREQUENCY);

      if(button_pressed()){
        analogWrite(BUZZER_PIN,0);
        state = SLEEP;
      }

      /////// MAY ADD NEW FEATURES


    break;



    //////// SLEEP STATE //////

    case SLEEP:


    if(start_acc_change(START_ACC_3_SIGMA) || start_ultra_change()){

      state = OFF;

    }


    break;
  


    default:
      Serial.println("Error: state not defined or defined incorrectly");
    break;

  }



}










///// FUNCTIONS /////




/// STARTING & CHECKING ///

// returns 0 if PIR changes from movement to not movement (TESTED)
int start_ultra_change(void){

  float sum = 0;
  float mean = 0;
  float dev = 0;
  
  if(firstUltraVal){

    for(i = 0; i<N_ULTRA; i++){

      digitalWrite(START_ULTRA_TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(START_ULTRA_TRIG_PIN, LOW);

      startUltraVal[i] = pulseIn(START_ULTRA_ECHO_PIN,HIGH)*0.017;
      sum = sum + startUltraVal[i];
    }

    firstUltraVal = 0;

  }else{

    for(i=1; i<N_ULTRA; i++){
      startUltraVal[i-1] = startUltraVal[i];
      sum = sum + startUltraVal[i-1];
    }

    digitalWrite(START_ULTRA_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(START_ULTRA_TRIG_PIN, LOW);

    startUltraVal[N_ULTRA-1] = pulseIn(START_ULTRA_ECHO_PIN,HIGH)*0.017;
    sum = sum + startUltraVal[N_ULTRA-1];

  }

  mean = sum/N_ULTRA;

  sum = 0;
  for(i=0;i<N_ULTRA;i++){
    sum = pow(startUltraVal[i]-mean,2);
  }

  dev = sqrt(sum)/(N_ULTRA-1);
  Serial.print("ULTRA st dev: "); Serial.println(dev);

  if(dev>START_ULTRA_3_SIGMA){   // when nothing is moving for some time it returns 0
    return 1;
  }else{
    return 0;
  }



}

// returns 1 if standard deviation of accelerometer changes TESTED
int start_acc_change(int tresh){

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
    AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    start_acc_val[N_ACC-1] = AcX/10;
    sum = sum + start_acc_val[N_ACC-1];
  }

  mean = sum/N_ACC; 

  sum = 0;
  for(i=0; i<N_ACC; i++){
    sum =  sum + pow(start_acc_val[i] - mean, 2);
  }

  dev = sqrt(sum/(N_ACC - 1));
  Serial.print("ACCEL st dev: "); Serial.println(dev);
  
  if(dev >= tresh){
    return 1;
  }else{
    return 0;
  }
}




/// CPD ///


/// CONTROL SENSORS

// gives out the temperature
float control_temp_read(void){
  Vo = analogRead(CONT_TEMP_PIN);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  T = T - 273.15;
  // Serial.print("CONTROL -> Temperature: "); Serial.println(T);
  return T;
}

// gives out the weights of the window closed or opened
int control_ultrasound_read(void){
  float duration_us, distance_cm;
  
  digitalWrite(CONT_ULTRA_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(CONT_ULTRA_TRIG_PIN, LOW);

  duration_us = pulseIn(CONT_ULTRA_ECHO_PIN, HIGH);

  distance_cm = duration_us * 0.017;
  // Serial.print("DEBUG: ---- DISTANCE: "); Serial.print(distance_cm); Serial.println(" ---- :DEBUG");

  if(distance_cm>=WINDOW_DISTANCE || distance_cm<1){
    Serial.println("CONTROL -> Ultrasound: window open");
    return OPENED_WINDOW;
  }else{
    Serial.println("CONTROL -> Ultrasound: window closed");
    return CLOSED_WINDOW;
  }
}


/// DETECTION

// returns 1 if the readout goes lower than a given treshold
int CO2_detection(void){
    float read = 0;
    read = analogRead(DET_CO2_PIN);
    Serial.println("<<<<---- CO2 VALUE ---->>>>");
    Serial.print("CO2: "); Serial.println(read);
    if(abs(read-CO2Start) >= CO2_TRESHOLD){
      return 1;
    }else{
      return 0;
    }
}




/// ALERT ///

// return 1 if the button is pressed
int button_pressed(void){

  for(i=1;i<N_BUTTON;i++){
    buttonVal[i-1] = buttonVal[i];
  }  
  buttonVal[N_BUTTON-1] = digitalRead(BUTTON_PIN);
  for(i=0;i<N_BUTTON;i++){
    if(buttonVal[i]){
      return 1;
    }else{
      return 0;
    }
  }

}
